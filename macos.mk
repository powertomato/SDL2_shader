DEFINES+=-DSDL_SHADER_OPENGL \
-DGL_GLEXT_PROTOTYPES

TEST_CASES=color_mode \
context_switch \
all_the_shaders \
vertex_buffer \
normal_map \
scale2x

GCC=clang
GPP=clang++
AR=ar
RM=rm -rf
FRAMEWORKS= -F/Library/Frameworks 
#-F/Library/Frameworks/SDL2_Image.framework/ -framework SDL2
CPPFLAGS=-g3 -pedantic -Wall -std=c++11 $(DEFINES) $(FRAMEWORKS)
CFLAGS=-g3 -pedantic -Wall -std=c99 $(DEFINES) $(FRAMEWORKS)
LDFLAGS=-framework SDL2\
-framework SDL2_Image\
-framework OpenGL

EXE_EXT=
RUN=

include common.mk

.PHONY: install
install:
	echo "No install target yet"
	#TODO
