/*
  SDL2 shader
  Copyright (C) 2014 Stefan Krulj (powertomato) <powertomato (-at-) gmail.com>

  Thes code is internally defined by SDL2 and copied from the SDL2-2.0.3
  source release. Minor changes or additions (if any) are provided by the
  license below.
*/

/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#ifndef _SDL_GL_RenderStructs_h
#define _SDL_GL_RenderStructs_h

#include "../SDL_SYS_RenderStructs.h"
#include <SDL2/SDL_opengl.h>

typedef enum {
    SHADER_NONE,
    SHADER_SOLID,
    SHADER_RGB,
    SHADER_YV12,
    NUM_SHADERS
} GL_Shader;


typedef enum
{
    GL_ATTRIBUTE_POSITION = 0,
    GL_ATTRIBUTE_TEXCOORD = 1,
    GL_ATTRIBUTE_ANGLE = 2,
    GL_ATTRIBUTE_CENTER = 3,
} GLES2_Attribute;

typedef struct
{
    GLhandleARB program;
    GLhandleARB vert_shader;
    GLhandleARB frag_shader;
} GL_ShaderData;

typedef struct GL_ShaderContext GL_ShaderContext;
struct GL_ShaderContext
{
    GLenum (*glGetError)(void);

    PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
    PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
    PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
    PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
    PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
    PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
    PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
    PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
    PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
    PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
    PFNGLUNIFORM1IARBPROC glUniform1iARB;
    PFNGLUNIFORM1FARBPROC glUniform1fARB;
    PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;

    SDL_bool GL_ARB_texture_rectangle_supported;

    GL_ShaderData shaders[NUM_SHADERS];
};

typedef struct GL_FBOList GL_FBOList;
struct GL_FBOList
{
    Uint32 w, h;
    GLuint FBO;
    GL_FBOList *next;
};

typedef struct
{
    SDL_GLContext context;

    SDL_bool debug_enabled;
    SDL_bool GL_ARB_debug_output_supported;
    int errors;
    char **error_messages;
    GLDEBUGPROCARB next_error_callback;
    GLvoid *next_error_userparam;

    SDL_bool GL_ARB_texture_rectangle_supported;
    struct {
        GL_Shader shader;
        Uint32 color;
        int blendMode;
    } current;

    SDL_bool GL_EXT_framebuffer_object_supported;
    GL_FBOList *framebuffers;

    /* OpenGL functions */
#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "SDL_glfuncs.h"
#undef SDL_PROC

    /* Multitexture support */
    SDL_bool GL_ARB_multitexture_supported;
    PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
    GLint num_texture_units;

    PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
    PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
    PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;

    /* Shader support */
    GL_ShaderContext *shaders;

} GL_RenderData;

typedef struct
{
    GLuint texture;
    GLenum type;
    GLfloat texw;
    GLfloat texh;
    GLenum format;
    GLenum formattype;
    void *pixels;
    int pitch;
    SDL_Rect locked_rect;

    /* YV12 texture support */
    SDL_bool yuv;
    GLuint utexture;
    GLuint vtexture;

    GL_FBOList *fbo;
} GL_TextureData;

#endif /*_SDL_GL_RenderStructs_h */

