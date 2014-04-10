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


#ifdef SDL_SHADER_OPENGLES2

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>

#include "../SDL_shader.h"
#include "SDL_GLES2_shader.h"
#include "SDL_GLES2_RenderStructs.h"

typedef struct {
	GLuint p;

	SDL_Uniform* color;
	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
} SDL_GLES2_ShaderData;
extern char* SDL_Shader_readRW( SDL_RWops* rwop );

typedef struct {
	GLuint p;
	GLint loc;
} SDL_GLES2_UniformData;

static const float inv255f = 1.0f / 255.0f;
static char color_conv[1024];
static int is_init = 0;

void SDL_GLES2_init() {
	char color_conv_fmt[] = 
	"vec4 convertColor(int type, vec4 color) {\n"
	"	vec4 ret = vec4(color.b, color.g, color.r, color.a);\n"
	"	if( type==%d || /*ABGR4444*/\n" //BGR formats
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
	"		ret.r = color.r;\n"
	"		ret.b = color.b;\n"
	"	}\n"
	"	if( type==%d || /*BGR555*/\n" //Formats without alpha
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

void SDL_GLES2_hint(sdl_shader_hint flag, void* value) {
	switch(flag) {
		default: /* nothing */ break;
	}
}

int SDL_GLES2_CompileSuccessful(int obj) {
  int status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  return status == GL_TRUE;
}

int SDL_GLES2_LinkSuccessful(int obj) {
  int status;
  glGetProgramiv(obj, GL_LINK_STATUS, &status);
  return status == GL_TRUE;
}

int SDL_GLES2_setUniform_matrix( SDL_Uniform* uniform, GLfloat* mat );
static void SDL_GLES2_updateViewport( SDL_Shader* shader ) {
	SDL_Renderer* renderer = shader->renderer;
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
    if ( renderer->viewport.w && renderer->viewport.h) {
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
			SDL_GLES2_setUniform_matrix( shader_data->vport, (GLfloat*) projection );
		}
    }
}


SDL_Shader* SDL_GLES2_createShader( SDL_Renderer* renderer,
	SDL_ShaderStream *shdstream )
{

	SDL_Shader *shader;
	GLuint v;
	GLuint f;
	char *vs,*fs;

	if( !is_init ){
		SDL_GLES2_init();
	}

	shader = (SDL_Shader*) malloc( sizeof(SDL_Shader) );
	if ( !shader ) {
		SDL_OutOfMemory();
		return NULL;
	}
	shader->driver_data = malloc( sizeof(SDL_GLES2_ShaderData ) );
	if ( !shader->driver_data ) {
		free( shader );
		SDL_OutOfMemory();
		return NULL;
	}
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;

	shader->renderer = renderer;
	shader->create_shader = SDL_GLES2_createShader;
	shader->destroyShader =  SDL_GLES2_destroyShader;
	shader->bindShader =     SDL_GLES2_bindShader;
	shader->unbindShader =   SDL_GLES2_unbindShader;
	shader->renderCopyShd =  SDL_GLES2_renderCopyShd;

	shader->createUniform =  SDL_GLES2_createUniform;
	shader->destroyUniform = SDL_GLES2_destroyUniform;

	shader_data->p = 0;
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
	if( !SDL_GLES2_CompileSuccessful(v) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(v, 512, &len, buff);

		SDL_SetError("SDL_Shader: OpenGL Could not compile vertex shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}
	glCompileShader( f );
	if( !SDL_GLES2_CompileSuccessful(f) ){

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

	glBindAttribLocation( shader_data->p, GLES2_ATTRIBUTE_POSITION, "position");
	glBindAttribLocation( shader_data->p, GLES2_ATTRIBUTE_TEXCOORD, "texCoords");

	glLinkProgram( shader_data->p );
	if( !SDL_GLES2_LinkSuccessful(shader_data->p) ){

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

	shader_data->vport = SDL_createUniform(shader,"world_view_projection");
	SDL_GLES2_updateViewport( shader );

	shader_data->color = SDL_createUniform(shader,"color");
	shader_data->color_mode = SDL_createUniform(shader,"color_mode");
	glEnableVertexAttribArray(GLES2_ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);

	return shader;
}

int SDL_GLES2_bindShader( SDL_Shader* shader ) {
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
	glUseProgram( shader_data->p );
	return glGetError();
}

int SDL_GLES2_unbindShader( SDL_Shader* shader ) {
	GLES2_DriverContext *data = (GLES2_DriverContext *) shader->renderer->driverdata;
	glUseProgram( data->current_program->id );
	return glGetError();
}

int SDL_GLES2_destroyShader( SDL_Shader* shader ) {
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
	shader->destroyUniform( shader, shader_data->color );
	shader->destroyUniform( shader, shader_data->color_mode );
	shader->destroyUniform( shader, shader_data->vport );
	if( shader_data->p ) glDeleteShader( shader_data->p );
	free( shader->driver_data );
	free( shader );
	return 0;
}


int SDL_GLES2_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_Rect * dstrect_i) {
    GLfloat vertices[8];
    GLfloat texCoords[8];
	GLES2_DriverContext *data = (GLES2_DriverContext *) shader->renderer->driverdata;
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
    GLES2_TextureData *texturedata = (GLES2_TextureData *) texture->driverdata;
	SDL_FRect dstrect = {dstrect_i->x, dstrect_i->y, dstrect_i->w, dstrect_i->h };

	SDL_GL_MakeCurrent(shader->renderer->window, data->context);

    data->glBindTexture(texturedata->texture_type, texturedata->texture);

	shader->bindShader(shader);
	if (texture->modMode ) {
		if( shader_data->color ) SDL_GLES2_setUniform_f4( shader_data->color,
				(GLfloat) texture->r * inv255f,
				(GLfloat) texture->g * inv255f,
				(GLfloat) texture->b * inv255f,
				(GLfloat) texture->a * inv255f);
	}else{
		if( shader_data->color ) SDL_GLES2_setUniform_f4( shader_data->color, 1,1,1,1 );
	}

	if( shader_data->color_mode ) {
		SDL_GLES2_setUniform_i( shader_data->color_mode, texture->format);
	}

    if (texture->blendMode != data->current.blendMode) {
		switch (texture->blendMode) {
			case SDL_BLENDMODE_NONE:
				//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glDisable(GL_BLEND);
				break;
			case SDL_BLENDMODE_BLEND:
				//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case SDL_BLENDMODE_ADD:
				//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
				break;
			case SDL_BLENDMODE_MOD:
				//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
				glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
				break;
		}
		data->current.blendMode = texture->blendMode;
	}
	if( !data->current.tex_coords ){
		data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
		data->current.tex_coords = SDL_TRUE;
	}

    vertices[0] = dstrect.x;
    vertices[1] = (dstrect.y + dstrect.h);
    vertices[2] = (dstrect.x + dstrect.w);
    vertices[3] = (dstrect.y + dstrect.h);
    vertices[4] = dstrect.x;
    vertices[5] = dstrect.y;
    vertices[6] = (dstrect.x + dstrect.w);
    vertices[7] = dstrect.y;
    data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    texCoords[0] = srcrect->x / (GLfloat)texture->w;
    texCoords[1] = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
    texCoords[2] = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
    texCoords[3] = (srcrect->y + srcrect->h) / (GLfloat)texture->h;
    texCoords[4] = srcrect->x / (GLfloat)texture->w;
    texCoords[5] = srcrect->y / (GLfloat)texture->h;
    texCoords[6] = (srcrect->x + srcrect->w) / (GLfloat)texture->w;
    texCoords[7] = srcrect->y / (GLfloat)texture->h;

    data->glVertexAttribPointer(GLES2_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    data->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shader->unbindShader(shader);
	
	return glGetError();
}

SDL_Uniform* SDL_GLES2_createUniform( SDL_Shader* shader, const char* name ) {
	GLint loc;
	SDL_GLES2_ShaderData* shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
	loc = glGetUniformLocation(shader_data->p, name);
	if( loc == -1 )
		return NULL;

	SDL_Uniform* uniform = (SDL_Uniform*) malloc( sizeof(SDL_Uniform) );
	if ( !uniform ) {
		SDL_OutOfMemory();
		return NULL;
	}
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*) malloc( sizeof(SDL_GLES2_UniformData) );
	if ( !udata ) {
		free( uniform );
		SDL_OutOfMemory();
		return NULL;
	}

	uniform->shader = shader;

	uniform->setUniform_iv  = SDL_GLES2_setUniform_iv;
	uniform->setUniform_i   = SDL_GLES2_setUniform_i;
	uniform->setUniform_i2  = SDL_GLES2_setUniform_i2;
	uniform->setUniform_i3  = SDL_GLES2_setUniform_i3;
	uniform->setUniform_i4  = SDL_GLES2_setUniform_i4;

	uniform->setUniform_fv  = SDL_GLES2_setUniform_fv;
	uniform->setUniform_f   = SDL_GLES2_setUniform_f;
	uniform->setUniform_f2  = SDL_GLES2_setUniform_f2;
	uniform->setUniform_f3  = SDL_GLES2_setUniform_f3;
	uniform->setUniform_f4  = SDL_GLES2_setUniform_f4;

	udata->p = shader_data->p;
	udata->loc = loc;

	uniform->driver_data = udata;
	return uniform;

}
int SDL_GLES2_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform ){
	if( !uniform )
		return 0;
	free(uniform->driver_data);
	free(uniform);
	return 0;
}

int SDL_GLES2_setUniform_matrix( SDL_Uniform* uniform, GLfloat* mat ) {
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniformMatrix4fv( udata->loc, 1, GL_FALSE, mat );
	return glGetError();
}

int SDL_GLES2_setUniform_fv( SDL_Uniform* uniform, float* vector, int num ) {
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform1fv( udata->loc, num, vector);
	return glGetError();
}
int SDL_GLES2_setUniform_f(  SDL_Uniform* uniform, float a ) {
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform1f(udata->loc, a );
	return glGetError();
}
int SDL_GLES2_setUniform_f2( SDL_Uniform* uniform, float a, float b ){
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform2f(udata->loc, a,b );
	return glGetError();
}
int SDL_GLES2_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c ){
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform3f(udata->loc, a,b,c );
	return glGetError();
}
int SDL_GLES2_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d ){
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform4f(udata->loc, a,b,c,d );
	return glGetError();
}


int SDL_GLES2_setUniform_iv( SDL_Uniform* uniform, int* vector, int num ) {
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform1iv( udata->loc, num, vector);
	return glGetError();
}
int SDL_GLES2_setUniform_i(  SDL_Uniform* uniform, int a ) {
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform1i(udata->loc, a );
	return glGetError();
}
int SDL_GLES2_setUniform_i2( SDL_Uniform* uniform, int a, int b ){
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform2i(udata->loc, a,b );
	return glGetError();
}
int SDL_GLES2_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c ){
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform3i(udata->loc, a,b,c );
	return glGetError();
}
int SDL_GLES2_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d ){
	SDL_GLES2_UniformData* udata = (SDL_GLES2_UniformData*)uniform->driver_data;
	glUniform4i(udata->loc, a,b,c,d );
	return glGetError();
}

#else
extern void __suppress_a_warning__();
#endif /* SDL_SHADER_OPENGLES2 */
