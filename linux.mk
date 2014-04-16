DEFINES+=-DSDL_SHADER_OPENGL \
-DSDL_SHADER_OPENGLES2 \
-DGL_GLEXT_PROTOTYPES

TEST_CASES=color_mode \
context_switch \
all_the_shaders \
vertex_buffer

GCC=gcc
GPP=g++
AR=ar
RM=rm -rf
MKDIR_LIST=mkdir -p
CPPFLAGS=-g3 --pedantic -Wall -std=c++11 $(DEFINES) -fno-rtti
CFLAGS=-g3 --pedantic -Wall -std=c99 $(DEFINES)
LDFLAGS=-lSDL2 -lSDL2_image \
	-ldl -lGL -lm
EXE_EXT=
RUN=#optirun

include common.mk

.PHONY: install
install:
	cp libSDL2_shader.a /usr/lib
	cp src/SDL_shader.h /usr/include
