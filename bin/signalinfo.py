#-*- coding:utf-8 -*-

import os
import sys
import json

from PIL import Image
from xml.etree.ElementTree import Element
from xml.etree.ElementTree import ElementTree

def Convert(src, dst):
    im0 = Image.open(src)
    im1 = Image.new("RGBA", (256, 256))
    
    l = max(0, int((im1.size[0] - im0.size[0]) / 2))
    t = max(0, int((im1.size[1] - im0.size[1]) / 2))
    
    r = l + min(im1.size[0], im0.size[0])
    b = t + min(im1.size[1], im0.size[1])

    im1.paste(im0, (l, t, r, b));
    
    im1.save(dst)
    
    l = im1.size[0]
    t = im1.size[1]
    r = -1
    b = -1
    
    for x in range(0, im1.size[0]):
        for y in range(0, im1.size[1]):
            if(im1.getpixel((x, y))[3] > 0):
                l = min(l, x)
                r = max(r, x)
                t = min(t, y)
                b = max(b, y)
    
    jf = open(dst + ".json", "w")
    print(json.dump({"name" : os.path.basename(dst), "left" : l, "top" : t, "right" : r, "bottom" : b}, fp=jf, sort_keys=True, indent=4))



if(3 != len(sys.argv)):
	print("Usage:" + sys.argv[0] + " src_path dst_path")
	sys.exit(1)

src = sys.argv[1]
dst = sys.argv[2]

if(not os.path.isabs(src)):
	src = os.path.abspath(src)

if(not os.path.isdir(src)):
	print(sys.argv[1] + " is not a path")
	sys.exit(2)

if(not os.path.isabs(dst)):
	dst = os.path.abspath(dst)

if(not os.path.isdir(dst)):
	print(sys.argv[2] + " is not a path")
	sys.exit(3)

for parent, dirs, files in os.walk(src):
	for file in files:
		sf = os.path.join(parent, file)
		df = os.path.join(dst, file)
		#print(sf, df)
		Convert(sf, df)
