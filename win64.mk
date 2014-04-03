DEFINES+=-DSDL_SHADER_D3D #-DSDL_SHADER_OPENGL -DGL_GLEXT_PROTOTYPES
TEST_CASES=color_mode
GCC=x86_64-w64-mingw32-gcc
GPP=x86_64-w64-mingw32-g++
AR=x86_64-w64-mingw32-ar
RM=rm -rf
MKDIR_LIST=mkdir -p
CPPFLAGS=-g3 --pedantic -Wall -std=c++11 $(DEFINES) -fno-rtti
CFLAGS=-g3 --pedantic -Wall -std=c99 $(DEFINES)
LDFLAGS= -lSDL2 -lSDL2_image \
	-lws2_32 -lstdc++ -ld3dx9_43 \
	-lopengl32 -lglew32 
EXE_EXT=.exe
RUN=wine

include common.mk
