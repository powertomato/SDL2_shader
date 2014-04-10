/*
  SDL2 shader
  Copyright (C) 2014 Stefan Krulj (powertomato) <powertomato (-at-) gmail.com>

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

#ifdef SDL_SHADER_OPENGL

#include <stdio.h>
#ifdef _WIN32
#include <SDL2/SDL_opengl.h>
#include <GL/glext.h>

#define SDL_PROC(ret,func,params) typedef ret (APIENTRY * func##_type) params; func##_type func;
SDL_PROC(void, glUniform1f, (GLint, GLfloat) )
SDL_PROC(void, glUniform2f, (GLint, GLfloat, GLfloat) )
SDL_PROC(void, glUniform3f, (GLint, GLfloat, GLfloat, GLfloat) )
SDL_PROC(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat) )
SDL_PROC(void, glUniform1fv, (GLint, GLsizei, const GLfloat*) )

SDL_PROC(void, glUniform1i, (GLint, GLint) )
SDL_PROC(void, glUniform2i, (GLint, GLint, GLint) )
SDL_PROC(void, glUniform3i, (GLint, GLint, GLint, GLint) )
SDL_PROC(void, glUniform4i, (GLint, GLint, GLint, GLint, GLint) )
SDL_PROC(void, glUniform1iv, (GLint, GLsizei, const GLint*) )
SDL_PROC(void, glGetShaderiv, (GLuint, GLenum,  GLint*) )
SDL_PROC(void, glGetProgramiv, (GLuint,  GLenum, GLint*) )
SDL_PROC(GLuint, glCreateShader, (GLenum) )
SDL_PROC(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*,  const GLint*) )
SDL_PROC(void, glCompileShader, (GLuint) )
SDL_PROC(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*) )
SDL_PROC(void, glBindAttribLocation, (GLuint, GLuint, const GLchar*) )
SDL_PROC(GLuint, glCreateProgram, (void) )
SDL_PROC(void, glAttachShader, (GLuint, GLuint) )
SDL_PROC(void, glLinkProgram, (GLuint) )
SDL_PROC(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*) )
SDL_PROC(void, glDeleteShader, (GLuint) )
SDL_PROC(void, glEnableVertexAttribArray, (GLuint) )
SDL_PROC(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid* ) )
SDL_PROC(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*) )
SDL_PROC(GLint, glGetUniformLocation, (GLuint, const GLchar*) )
SDL_PROC(void, glUseProgram, (GLuint) )
#undef SDL_PROC

#undef GL_GLEXT_PROTOTYPES
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "../SDL_shader.h"
#include "SDL_GL_shader.h"
#include "SDL_GL_RenderStructs.h"

typedef struct {
	GLuint p;

	SDL_Uniform* color;
	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
} SDL_GL_ShaderData;
extern char* SDL_Shader_readRW( SDL_RWops* rwop );

typedef struct {
	GLuint p;
	GLint loc;
} SDL_GL_UniformData;

static const float inv255f = 1.0f / 255.0f;
static char color_conv[1024];
static int is_init = 0;

void SDL_GL_init() {
	char color_conv_fmt[] = 
	"vec4 convertColor(int type, vec4 color) {\n"
	"	vec4 ret = color;\n"
	"	if( type==%d || /*ABGR4444*/\n" /*BGR formats*/
	"		type==%d || /*BGR555*/\n"
	"		type==%d || /*BGRA4444*/\n"
	"		type==%d || /*ABGR1555*/\n"
	"		type==%d || /*BGRA5551*/\n"
	"		type==%d || /*BGR565*/\n"
	"		type==%d || /*BGR24*/\n"
	"		type==%d || /*BGR888*/\n"
	"		type==%d || /*BGRX8888*/\n"
	"		type==%d || /*ABGR8888*/\n"
	"		type==%d    /*BGRA8888*/\n"
	"	){\n"
	"		ret.r = color.b;\n"
	"		ret.b = color.r;\n"
	"	}\n"
	"	if( type==%d || /*BGR555*/\n" /*Formats without alpha*/
	"		type==%d || /*BGR565*/\n"
	"		type==%d || /*BGR24*/\n"
	"		type==%d || /*BGR888*/\n"
	"		type==%d || /*RGB332*/\n"
	"		type==%d || /*RGB444*/\n"
	"		type==%d || /*RGB555*/\n"
	"		type==%d || /*RGB565*/\n"
	"		type==%d || /*RGB24*/\n"
	"		type==%d    /*RGB888*/\n"
	"	){\n"
	"		ret.a = 1.0;\n"
	"	}\n"
	"	return ret;\n"
	"}\n";

	snprintf(color_conv, 1024, color_conv_fmt,
		SDL_PIXELFORMAT_ABGR4444,
		SDL_PIXELFORMAT_BGR555,
		SDL_PIXELFORMAT_BGRA4444,
		SDL_PIXELFORMAT_ABGR1555,
		SDL_PIXELFORMAT_BGRA5551,
		SDL_PIXELFORMAT_BGR565,
		SDL_PIXELFORMAT_BGR24,
		SDL_PIXELFORMAT_BGR888,
		SDL_PIXELFORMAT_BGRX8888,
		SDL_PIXELFORMAT_ABGR8888,
		SDL_PIXELFORMAT_BGRA8888,

		SDL_PIXELFORMAT_BGR555,
		SDL_PIXELFORMAT_BGR565,
		SDL_PIXELFORMAT_BGR24,
		SDL_PIXELFORMAT_BGR888,
		SDL_PIXELFORMAT_RGB332,
		SDL_PIXELFORMAT_RGB444,
		SDL_PIXELFORMAT_RGB555,
		SDL_PIXELFORMAT_RGB565,
		SDL_PIXELFORMAT_RGB24,
		SDL_PIXELFORMAT_RGB888
	);
	is_init = 1;

}

void SDL_GL_hint(sdl_shader_hint flag, void* value) {
	switch(flag) {
		//case SDL_GL_VERSION: GLSL_version = *((int*)value); break;
		default: /* nothing */ break;
	}
}

static int SDL_GL_CompileSuccessful(int obj) {
  int status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  return status == GL_TRUE;
}

static int SDL_GL_LinkSuccessful(int obj) {
  int status;
  glGetProgramiv(obj, GL_LINK_STATUS, &status);
  return status == GL_TRUE;
}

int SDL_GL_setUniform_matrix( SDL_Uniform* uniform, GLfloat* mat );
SDL_Shader* SDL_GL_createShader( SDL_Renderer* renderer,
	SDL_ShaderStream* shdstream )
{

	SDL_Shader *shader;
	GLuint v;
	GLuint f;

	char *vs,*fs;
	if( !is_init ){
		SDL_GL_init();
	}

	shader = (SDL_Shader*) malloc( sizeof(SDL_Shader) );
	if( !shader ){
		SDL_OutOfMemory();
		return NULL;
	}
	shader->driver_data = malloc( sizeof(SDL_GL_ShaderData ) );
	if ( !shader->driver_data ) {
		free( shader->driver_data );
		SDL_OutOfMemory();
		return NULL;
	}
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;

	shader->renderer = renderer;
	shader->create_shader = SDL_GL_createShader;
	shader->destroyShader =  SDL_GL_destroyShader;
	shader->bindShader =     SDL_GL_bindShader;
	shader->unbindShader =   SDL_GL_unbindShader;
	shader->renderCopyShd =  SDL_GL_renderCopyShd;

	shader->createUniform =  SDL_GL_createUniform;
	shader->destroyUniform = SDL_GL_destroyUniform;

	shader_data->p = 0;

#ifdef _WIN32
	glUniform1f = (glUniform1f_type) SDL_GL_GetProcAddress("glUniform1f");
	glUniform2f = (glUniform2f_type) SDL_GL_GetProcAddress("glUniform2f");
	glUniform3f = (glUniform3f_type) SDL_GL_GetProcAddress("glUniform3f");
	glUniform4f = (glUniform4f_type) SDL_GL_GetProcAddress("glUniform4f");
	glUniform1fv = (glUniform1fv_type) SDL_GL_GetProcAddress("glUniform1fv");

	glUniform1i = (glUniform1i_type) SDL_GL_GetProcAddress("glUniform1i");
	glUniform2i = (glUniform2i_type) SDL_GL_GetProcAddress("glUniform2i");
	glUniform3i = (glUniform3i_type) SDL_GL_GetProcAddress("glUniform3i");
	glUniform4i = (glUniform4i_type) SDL_GL_GetProcAddress("glUniform4i");
	glUniform1iv = (glUniform1iv_type) SDL_GL_GetProcAddress("glUniform1iv");
	glGetShaderiv = (glGetShaderiv_type) SDL_GL_GetProcAddress("glGetShaderiv");
	glGetProgramiv = (glGetProgramiv_type) SDL_GL_GetProcAddress("glGetProgramiv");
	glCreateShader = (glCreateShader_type) SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource = (glShaderSource_type) SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (glCompileShader_type) SDL_GL_GetProcAddress("glCompileShader");
	glGetShaderInfoLog = (glGetShaderInfoLog_type) SDL_GL_GetProcAddress("glGetShaderInfoLog");
	glBindAttribLocation = (glBindAttribLocation_type) SDL_GL_GetProcAddress("glBindAttribLocation");
	glCreateProgram = (glCreateProgram_type) SDL_GL_GetProcAddress("glCreateProgram");
	glAttachShader = (glAttachShader_type) SDL_GL_GetProcAddress("glAttachShader");
	glLinkProgram = (glLinkProgram_type) SDL_GL_GetProcAddress("glLinkProgram");
	glGetProgramInfoLog = (glGetProgramInfoLog_type) SDL_GL_GetProcAddress("glGetProgramInfoLog");
	glDeleteShader = (glDeleteShader_type) SDL_GL_GetProcAddress("glDeleteShader");
	glEnableVertexAttribArray = (glEnableVertexAttribArray_type) SDL_GL_GetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (glVertexAttribPointer_type) SDL_GL_GetProcAddress("glVertexAttribPointer");
	glUniformMatrix4fv = (glUniformMatrix4fv_type) SDL_GL_GetProcAddress("glUniformMatrix4fv");
	glGetUniformLocation = (glGetUniformLocation_type) SDL_GL_GetProcAddress("glGetUniformLocation");
	glUseProgram = (glUseProgram_type) SDL_GL_GetProcAddress("glUseProgram");
#endif

	v = glCreateShader( GL_VERTEX_SHADER );
	f = glCreateShader( GL_FRAGMENT_SHADER );
	shader_data->color = NULL;
	shader_data->vport = NULL;
	shader_data->color_mode = NULL;

	vs = SDL_Shader_readRW( shdstream->vshader );
	if( vs==NULL ){
		shader->destroyShader(shader);
		SDL_OutOfMemory();
		return NULL;
	}

	fs = SDL_Shader_readRW( shdstream->pshader );
	if( fs==NULL ){
		free(fs);
		shader->destroyShader(shader);
		SDL_OutOfMemory();
		return NULL;
	}

	const char* vss = (const char*) vs;
	const char *fss[] = {fs, color_conv};
	glShaderSource(v, 1, &vss,NULL);
	glShaderSource(f, 2, fss,NULL);

	free(vs);
	free(fs);

	glCompileShader( v );
	if( !SDL_GL_CompileSuccessful(v) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(v, 512, &len, buff);

		SDL_SetError("SDL_Shader: OpenGL Could not compile vertex shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}
	glCompileShader( f );
	if( !SDL_GL_CompileSuccessful(f) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(f, 512, &len, buff);

		SDL_SetError("SDL_Shader: OpenGL Could not compile fragment shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader_data->p = glCreateProgram();

	glAttachShader( shader_data->p,v );
	glAttachShader( shader_data->p,f );

	glBindAttribLocation( shader_data->p, GL_ATTRIBUTE_POSITION, "position");
	glBindAttribLocation( shader_data->p, GL_ATTRIBUTE_TEXCOORD, "texCoords");

	glLinkProgram( shader_data->p );
	if( !SDL_GL_LinkSuccessful(shader_data->p) ){

		GLchar buff[512];
		GLsizei len;
		glGetProgramInfoLog(shader_data->p, 512, &len, buff);

		SDL_SetError("SDL_Shader: OpenGL Could not link shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader->bindShader(shader);

	glDeleteShader( v );
	glDeleteShader( f );

    if ( renderer->viewport.w && renderer->viewport.h) {
		shader_data->vport = SDL_createUniform(shader,"world_view_projection");
		if( shader_data->vport ) {
			GLfloat projection[4][4];
			/* Prepare an orthographic projection */
			projection[0][0] = 2.0f / renderer->viewport.w;
			projection[0][1] = 0.0f;
			projection[0][2] = 0.0f;
			projection[0][3] = 0.0f;
			projection[1][0] = 0.0f;
			if (renderer->target) {
				projection[1][1] = 2.0f / renderer->viewport.h;
			} else {
				projection[1][1] = -2.0f / renderer->viewport.h;
			}
			projection[1][2] = 0.0f;
			projection[1][3] = 0.0f;
			projection[2][0] = 0.0f;
			projection[2][1] = 0.0f;
			projection[2][2] = 0.0f;
			projection[2][3] = 0.0f;
			projection[3][0] = -1.0f;
			if (renderer->target) {
				projection[3][1] = -1.0f;
			} else {
				projection[3][1] = 1.0f;
			}
			projection[3][2] = 0.0f;
			projection[3][3] = 1.0f;
			SDL_GL_setUniform_matrix( shader_data->vport, (GLfloat*) projection );
		}
    }
	shader_data->color = SDL_createUniform(shader,"color");
	shader_data->color_mode = SDL_createUniform(shader,"color_mode");
	glEnableVertexAttribArray(GL_ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(GL_ATTRIBUTE_TEXCOORD);

	return shader;
}

int SDL_GL_bindShader( SDL_Shader* shader ) {
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	glUseProgram( shader_data->p );
	return glGetError();
}

int SDL_GL_unbindShader( SDL_Shader* shader ) {
	glUseProgram( 0 );
	return glGetError();
}

int SDL_GL_destroyShader( SDL_Shader* shader ) {
	// SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	if( !shader )
		return 0;
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	shader->destroyUniform(shader, shader_data->color_mode );
	shader->destroyUniform(shader, shader_data->color );
	shader->destroyUniform(shader, shader_data->vport );
	if( shader_data->p ) glDeleteShader( shader_data->p );
	shader->unbindShader( shader );
	free( shader->driver_data );
	free( shader );
	return 0;
}


int SDL_GL_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_Rect * dstrect_i) {

	float width, height;
	GL_RenderData *data = (GL_RenderData *) shader->renderer->driverdata;
    GL_TextureData *texturedata = (GL_TextureData *) texture->driverdata;
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData *) shader->driver_data;
	SDL_FRect dstrect = {dstrect_i->x, dstrect_i->y, dstrect_i->w, dstrect_i->h };

    GLfloat minu, maxu, minv, maxv;

    if (texturedata->yuv) {
        return SDL_SetError("SDL_Shader: OpenGL YUV-textures not supported");
    }


	SDL_GL_MakeCurrent(shader->renderer->window, data->context);
	SDL_GL_BindTexture(texture, &width, &height );

	shader->bindShader(shader);
	data->current.shader = SHADER_NONE;
	if (texture->modMode ) {
		if( shader_data->color ) SDL_GL_setUniform_f4( shader_data->color,
				(GLfloat) texture->r * inv255f,
				(GLfloat) texture->g * inv255f,
				(GLfloat) texture->b * inv255f,
				(GLfloat) texture->a * inv255f);
	}else{
		if( shader_data->color ) 
			SDL_GL_setUniform_f4( shader_data->color, 1,1,1,1 );
	}

	if( shader_data->color_mode ) {
		SDL_GL_setUniform_i( shader_data->color_mode, texture->format);
	}

    data->current.shader = SHADER_NONE;
    if (texture->blendMode != data->current.blendMode) {

		switch (texture->blendMode) {
			case SDL_BLENDMODE_NONE:
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glDisable(GL_BLEND);
				break;
			case SDL_BLENDMODE_BLEND:
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
				data->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case SDL_BLENDMODE_ADD:
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
				data->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
				break;
			case SDL_BLENDMODE_MOD:
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
				data->glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
				break;
		}
		data->current.blendMode = texture->blendMode;
	}

    GLfloat vertices[8];
    GLfloat texCoords[8];

    vertices[0] = dstrect.x;
    vertices[1] = dstrect.y;
    vertices[2] = (dstrect.x + dstrect.w);
    vertices[3] = dstrect.y;
    vertices[4] = dstrect.x;
    vertices[5] = (dstrect.y + dstrect.h);
    vertices[6] = vertices[2];
    vertices[7] = vertices[5];
    glVertexAttribPointer(GL_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    minu = (GLfloat) srcrect->x / texture->w;
    minu *= texturedata->texw;
    maxu = (GLfloat) (srcrect->x + srcrect->w) / texture->w;
    maxu *= texturedata->texw;
    minv = (GLfloat) srcrect->y / texture->h;
    minv *= texturedata->texh;
    maxv = (GLfloat) (srcrect->y + srcrect->h) / texture->h;
    maxv *= texturedata->texh;

    texCoords[0] = minu;
    texCoords[1] = minv;
    texCoords[2] = maxu;
    texCoords[3] = minv;
    texCoords[4] = minu;
    texCoords[5] = maxv;
    texCoords[6] = maxu;
    texCoords[7] = maxv;

    glVertexAttribPointer(GL_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shader->unbindShader(shader);

	return glGetError();
}

SDL_Uniform* SDL_GL_createUniform( SDL_Shader* shader, const char* name ) {
	GLint loc;
	SDL_GL_ShaderData* shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	loc = glGetUniformLocation(shader_data->p, name);
	if( loc == -1 )
		return NULL;

	SDL_Uniform* uniform = (SDL_Uniform*) malloc( sizeof(SDL_Uniform) );
	if( !uniform ){
		SDL_OutOfMemory();
		return NULL;
	}
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*) malloc( sizeof(SDL_GL_UniformData) );
	if( !udata ){
		free(uniform);
		SDL_OutOfMemory();
		return NULL;
	}

	uniform->shader = shader;

	uniform->setUniform_iv  = SDL_GL_setUniform_iv;
	uniform->setUniform_i   = SDL_GL_setUniform_i;
	uniform->setUniform_i2  = SDL_GL_setUniform_i2;
	uniform->setUniform_i3  = SDL_GL_setUniform_i3;
	uniform->setUniform_i4  = SDL_GL_setUniform_i4;

	uniform->setUniform_fv  = SDL_GL_setUniform_fv;
	uniform->setUniform_f   = SDL_GL_setUniform_f;
	uniform->setUniform_f2  = SDL_GL_setUniform_f2;
	uniform->setUniform_f3  = SDL_GL_setUniform_f3;
	uniform->setUniform_f4  = SDL_GL_setUniform_f4;

	udata->p = shader_data->p;
	udata->loc = loc;

	uniform->driver_data = udata;
	return uniform;

}
int SDL_GL_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform ){
	free(uniform->driver_data);
	free(uniform);
	return 0;
}

int SDL_GL_setUniform_matrix( SDL_Uniform* uniform, GLfloat* mat ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniformMatrix4fv( udata->loc, 1, GL_FALSE, mat );
	return glGetError();
}

int SDL_GL_setUniform_fv( SDL_Uniform* uniform, float* vector, int num ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1fv( udata->loc, num, vector);
	return glGetError();
}
int SDL_GL_setUniform_f(  SDL_Uniform* uniform, float a ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1f(udata->loc, a );
	return glGetError();
}
int SDL_GL_setUniform_f2( SDL_Uniform* uniform, float a, float b ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform2f(udata->loc, a,b );
	return glGetError();
}
int SDL_GL_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform3f(udata->loc, a,b,c );
	return glGetError();
}
int SDL_GL_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform4f(udata->loc, a,b,c,d );
	return glGetError();
}


int SDL_GL_setUniform_iv( SDL_Uniform* uniform, int* vector, int num ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1iv( udata->loc, num, vector);
	return glGetError();
}
int SDL_GL_setUniform_i(  SDL_Uniform* uniform, int a ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1i(udata->loc, a );
	return glGetError();
}
int SDL_GL_setUniform_i2( SDL_Uniform* uniform, int a, int b ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform2i(udata->loc, a,b );
	return glGetError();
}
int SDL_GL_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform3i(udata->loc, a,b,c );
	return glGetError();
}
int SDL_GL_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform4i(udata->loc, a,b,c,d );
	return glGetError();
}

#else
extern void __suppress_a_warning__();
#endif /* SDL_SHADER_OPENGL */
