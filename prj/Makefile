APPNAME		:= labelgen
DESTDIR		?= ../bin
SRCDIR		?= ../src
OBJDIR		?= ../obj
GLPDIR		?= ../vendor/glp

vpath %.c $(SRCDIR) $(GLPDIR)/src
vpath %.cpp $(SRCDIR) $(GLPDIR)/src
vpath %.o $(OBJDIR)

# Make command to use for dependencies
#CC = $(CROSS_COMPILE)gcc
#AR = $(CROSS_COMPILE)ar

TARGET_PATH_LIB = $(SDKTARGETSYSROOT)/usr/lib
TARGET_PATH_INCLUDE = $(SDKTARGETSYSROOT)/usr/include
#COMMON_DIR=../common

BIN = $(DESTDIR)/$(APPNAME)

CFLAGS = -D_GNU_SOURCE -fPIC -fno-strict-aliasing -fno-optimize-sibling-calls -Wall -g
CFLAGS += $(CFLAGS_EGL)
CFLAGS += -DLINUX -D_FREEIMAGE

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG -D_DEBUG 
else
	CFLAGS += -O3 
endif

CFLAGS += \
	-I$(TARGET_PATH_INCLUDE) \
	-I../vendor/include \
	-I$(GLPDIR)/src
OBJECTS += \
	main.o \
	tga.o \
	texture.o \
	buffer.o \
	vertexarray.o \
	shader.o \
	program.o \
	renderbuffer.o \
	framebuffer.o

DEPS += -lstdc++ -lrt -lm -ldl -lpthread -Wl,--library-path=$(TARGET_PATH_LIB),-rpath-link=$(TARGET_PATH_LIB)

CFLAGS += -DUSE_OGL
OBJECTS += glxc.o
DEPS += -lGLEW -lGL -lX11 -lfreeimageplus -lboost_system -lboost_filesystem

CPPFLAGS = -std=c++11
CPPFLAGS += $(CFLAGS)

$(BIN) : $(OBJECTS)
	@echo " LD " $@
	$(QUIET)$(CC) -o $(BIN) $(addprefix $(OBJDIR)/,$(OBJECTS)) $(DEPS)

%.o : %.c
	@echo " CC " $@
	$(QUIET)$(CC) $(CFLAGS) -MMD -c $< -o $(OBJDIR)/$@

%.o : %.cpp
	@echo " CXX " $@
	$(QUIET)$(CC) $(CPPFLAGS) -MMD -c $< -o $(OBJDIR)/$@

clean:
	rm -f $(addprefix $(OBJDIR)/,$(OBJECTS)) *~ $(addprefix $(OBJDIR)/,$(OBJECTS:.o=.d)) $(BIN)
	
install:
	cp -f $(APPNAME) $(DESTDIR)/.
	
	
-include $(OBJECTS_TUT:.o=.d)
