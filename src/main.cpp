#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <signal.h>
#include <boost/filesystem.hpp>

#include <glcontext.h>
#include <program.h>
#include <texture.h>
#include <buffer.h>
#include <vertexarray.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <FreeImagePlus.h>

volatile sig_atomic_t quit = 0;

void sighandler(int signal)
{
	std::cout << "Caught signal " << signal << ", setting flaq to quit.\n";
	quit = true;
}

bool fetchFilesInDirectory(const char* dir, std::vector<std::string>& files)
{
	boost::filesystem::path path = boost::filesystem::system_complete(boost::filesystem::path(dir));
	if(!boost::filesystem::exists(path) || !boost::filesystem::is_directory(path))
	{
	    return false;
	}
	
	const boost::filesystem::directory_iterator endPath;
	for(boost::filesystem::directory_iterator p(path); p != endPath; ++p)
	{
	    std::string ext = boost::filesystem::extension((*p).path());
	    if(ext == ".png" || ext == ".jpg")
	    {
	        files.push_back((*p).path().string());
	    }
	}
	return files.size() > 0;
}

Program createProgram()
{
    Shader shaders[] =
    {
        Shader(SHADER_SOURCE(
attribute vec2 pos;\n
\n
uniform vec4 rect;\n
uniform mat4 trans;\n
\n
varying vec2 tc;\n
\n
void main()\n
{\n
    gl_Position = trans * vec4(pos * rect.zw + rect.xy, 0.0, 1.0);\n
    tc = pos;\n
}\n
        ), 0, GL_VERTEX_SHADER),
    
        Shader(SHADER_SOURCE(
\n
uniform sampler2D tex;\n
\n
varying vec2 tc;\n
\n
void main()\n
{\n
    gl_FragColor = texture2D(tex, tc);\n
}\n
        ), 0, GL_FRAGMENT_SHADER)
    };
    
    return Program(shaders, 2);
}

int main (int argc, char* argv[])
{
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	
	if(argc < 4)
	{
	    std::cout << "Usage:" << argv[0] << " object_path background_path dist_path" << std::endl;
	    return 1;
	}
	
	std::vector<std::string> objs;
    std::vector<std::string> bgs;

    if(!fetchFilesInDirectory(argv[1], objs))
    {
	    std::cout << "Can't open object path:" << argv[1] << std::endl;
	    return 2;
    }
    
    if(!fetchFilesInDirectory(argv[2], bgs))
    {
	    std::cout << "Can't open background path:" << argv[2] << std::endl;
	    return 3;
    }
    
	boost::filesystem::path dist = boost::filesystem::system_complete(boost::filesystem::path(argv[3]));
	if(!boost::filesystem::exists(dist) || !boost::filesystem::is_directory(dist))
	{
	    std::cout << "Can't open dist path:" << argv[3] << std::endl;
	    return 4;
	}

    for(auto i = objs.begin();i != objs.end();++i)
    {
        std::cout << *i << std::endl;
    }

    for(auto i = bgs.begin();i != bgs.end();++i)
    {
        std::cout << *i << std::endl;
    }

    glContext context(1024, 1024);
    std::cout << "Frame size:(" << context.getWidth() << ", " << context.getHeight() << ")" << std::endl;
    
    glClearColor(0.1f, 0.1f, 1.0f, 0.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
    Program program(createProgram());
    program.Use();
    program.setUniform("tex", 0);
        
    float vertices[] =
    {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };
    Buffer vb(vertices, sizeof(vertices), GL_ARRAY_BUFFER);
    
    uint16_t indices[] =
    {
        0, 1, 2, 3,
    };
    Buffer ib(indices, sizeof(indices), GL_ELEMENT_ARRAY_BUFFER);
    
    VertexArray vao;
    vao.Bind();
    vb.Bind();
    vao.VertexAttrib(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    ib.Bind();
    vao.Unbind();
    
    glm::mat4 bgTrans(1.0f);
    glm::vec4 bgRect(-1.0f, -1.0f, 2.0f, 2.0f);    
		
    Texture objTex;
    Texture bgTex;

    bool saved = false;
    std::vector<uint8_t> pixels(context.getWidth() * context.getHeight() * 4);
    
    auto obji = objs.begin();
    auto bgi = bgs.begin();
	while (!quit && bgi != bgs.end())
	{
        objTex.Load((*obji).c_str());
        bgTex.Load((*bgi).c_str());

        float width = objTex.getWidth();
        float height = objTex.getHeight();
        glm::vec4 objRect(-0.5f * width, -0.5f * height, width, height);    

        const float aov = 0.78f;
        float bgWidth = bgTex.getWidth();
        float bgHeight = bgTex.getHeight();
        float distance = bgHeight * 0.5f / tan(aov * 0.5f);
        glm::mat4 prj  = glm::perspective(0.78f, bgWidth / bgHeight, 0.5f * distance, 1.5f * distance); 
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, distance), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glViewport(0, 0, bgTex.getWidth(), bgTex.getHeight());
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    	    
	    vao.Bind();
	    program.Use();

        glDisable(GL_BLEND);
	    bgTex.Bind();
        program.setUniform("rect", bgRect);
        program.setUniform("trans", bgTrans);
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	    
	    glEnable(GL_BLEND);
	    objTex.Bind();
        program.setUniform("rect", objRect);
        program.setUniform("trans", prj * view);
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	    
	    vao.Unbind();
	    
// 	    glFlush();
// 
// 	    if(!saved)
// 	    {
//         fipImage image(FIT_BITMAP, bgTex.getWidth(), bgTex.getHeight(), 24);
// 	    glReadPixels(0, 0, image.getWidth(), image.getHeight(), GL_BGR_EXT, GL_UNSIGNED_BYTE, image.accessPixels());
// 	    image.flipVertical();
//         image.save((dist / boost::filesystem::path(*bgi).filename()).string().c_str());
//         
//         }

	    
	    context.SwapBuffers();
	    
// 	    ++obji;
// 	    if(obji == objs.end())
// 	    {
// 	        obji = objs.begin();
// 	    }
// 	    ++bgi;
	}
	return 0;
}
