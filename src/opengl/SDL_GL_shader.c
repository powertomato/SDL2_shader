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
SDL_PROC(void, glActiveTextureARB, (GLenum) )
#undef SDL_PROC

#undef GL_GLEXT_PROTOTYPES
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "../SDL_shader.h"
#include "SDL_GL_shader.h"
#include "SDL_GL_RenderStructs.h"

static const float inv255f = 1.0f / 255.0f;
static char texture_names[][32] = {"tex0","tex1","tex2","tex3","tex4","tex5","tex6","tex7"};

typedef struct {
	GLuint p;

	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
	Uint32 is_target;
} SDL_GL_ShaderData;

typedef struct {
	float x, y, z;
	float r, g, b, a;
	float tex_s, tex_t;
} SDL_GL_Vertex_t;

extern char* SDL_Shader_readRW( SDL_RWops* rwop );

typedef struct {
	GLuint p;
	GLint loc;
} SDL_GL_UniformData;

static char color_conv[1024];
static int is_init = 0;

void SDL_GL_getVertex( SDL_Vertex* vertices, unsigned num,
	uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a,
	float *x, float *y, float *z,
	float *tex_s, float *tex_t )
{
	SDL_GL_Vertex_t* vbuff = (SDL_GL_Vertex_t*) vertices->vertexBuffer;
	if( r ) (*r) = (uint8_t) (vbuff[num].r * 255);
	if( g ) (*g) = (uint8_t) (vbuff[num].g * 255);
	if( b ) (*b) = (uint8_t) (vbuff[num].b * 255);
	if( a ) (*a) = (uint8_t) (vbuff[num].a * 255);

	if( x ) (*x) = (vbuff[num].x );
	if( y ) (*y) = (vbuff[num].y );
	if( z ) (*z) = (vbuff[num].z );

	if( tex_s ) (*tex_s) = (vbuff[num].tex_s );
	if( tex_t ) (*tex_t) = (vbuff[num].tex_t );
}

void SDL_GL_setVertexColor(SDL_Vertex* vertices, unsigned from, unsigned num,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	SDL_GL_Vertex_t* vbuff = (SDL_GL_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].r = r * inv255f;
		vbuff[i].g = g * inv255f;
		vbuff[i].b = b * inv255f;
		vbuff[i].a = a * inv255f;
	}
}
void SDL_GL_setVertexPosition(SDL_Vertex* vertices, unsigned from,
	unsigned num, float x, float y, float z )
{
	SDL_GL_Vertex_t* vbuff = (SDL_GL_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].x = x;
		vbuff[i].y = y;
		vbuff[i].z = z;
	}
}
void SDL_GL_setVertexTexCoord(SDL_Vertex* vertices, unsigned from,
	unsigned num, float s, float t )
{
	SDL_GL_Vertex_t* vbuff = (SDL_GL_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].tex_s = s;
		vbuff[i].tex_t = t;
	}
}
SDL_Vertex* SDL_GL_createVertexBuffer( unsigned size ) {
	SDL_Vertex* ret = (SDL_Vertex*) malloc( sizeof(SDL_Vertex) );
	if ( !ret ) {
		SDL_OutOfMemory();
		return NULL;
	}
	ret->vertexBuffer = malloc( sizeof(SDL_GL_Vertex_t)*size );
	if ( !ret->vertexBuffer ) {
		free( ret );
		SDL_OutOfMemory();
		return NULL;
	}
	ret->size = size;
	ret->getVertex = SDL_GL_getVertex;
	ret->setVertexColor = SDL_GL_setVertexColor;
	ret->setVertexPosition = SDL_GL_setVertexPosition;
	ret->setVertexTexCoord = SDL_GL_setVertexTexCoord;
	return ret;
}
int SDL_GL_destroyVertexBuffer( SDL_Vertex* buff ) {
	free( buff->vertexBuffer );
	free( buff );
	return 0;
}


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
		case SDL_GL_TEX0_NAME: strncpy( texture_names[0], (char*) value, 32 ); break;
		case SDL_GL_TEX1_NAME: strncpy( texture_names[1], (char*) value, 32 ); break;
		case SDL_GL_TEX2_NAME: strncpy( texture_names[2], (char*) value, 32 ); break;
		case SDL_GL_TEX3_NAME: strncpy( texture_names[3], (char*) value, 32 ); break;
		case SDL_GL_TEX4_NAME: strncpy( texture_names[4], (char*) value, 32 ); break;
		case SDL_GL_TEX5_NAME: strncpy( texture_names[5], (char*) value, 32 ); break;
		case SDL_GL_TEX6_NAME: strncpy( texture_names[6], (char*) value, 32 ); break;
		case SDL_GL_TEX7_NAME: strncpy( texture_names[7], (char*) value, 32 ); break;
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
static void SDL_GL_updateViewport( SDL_Shader* shader ) {
	SDL_Renderer* renderer = shader->renderer;
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	int w,h;
	SDL_RenderGetLogicalSize( renderer, &w, &h );
	if ( w && h && shader_data->vport ) {
		GLfloat projection[4][4];
		/* Prepare an orthographic projection */
		projection[0][0] = 2.0f / w;
		projection[0][1] = 0.0f;
		projection[0][2] = 0.0f;
		projection[0][3] = 0.0f;

		projection[1][0] = 0.0f;
		if (renderer->target) {
			projection[1][1] = 2.0f / h;
		} else {
			projection[1][1] = -2.0f / h;
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
	shader->updateViewport = SDL_GL_updateViewport;

	shader->createUniform =  SDL_GL_createUniform;
	shader->destroyUniform = SDL_GL_destroyUniform;

	shader->createVertexBuffer = SDL_GL_createVertexBuffer;
	shader->destroyVertexBuffer = SDL_GL_destroyVertexBuffer;

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
	glActiveTextureARB = (glActiveTextureARB_type) SDL_GL_GetProcAddress("glActiveTextureARB");
#endif

	v = glCreateShader( GL_VERTEX_SHADER );
	f = glCreateShader( GL_FRAGMENT_SHADER );
	shader_data->is_target = renderer->target != NULL;
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

		SDL_SetError("SDL_Shader[GL]: Could not compile vertex shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}
	glCompileShader( f );
	if( !SDL_GL_CompileSuccessful(f) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(f, 512, &len, buff);

		SDL_SetError("SDL_Shader[GL]: Could not compile fragment shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader_data->p = glCreateProgram();

	glAttachShader( shader_data->p,v );
	glAttachShader( shader_data->p,f );

	glBindAttribLocation( shader_data->p, GL_ATTRIBUTE_POSITION, "position");
	glBindAttribLocation( shader_data->p, GL_ATTRIBUTE_TEXCOORD, "texCoords");
	glBindAttribLocation( shader_data->p, GL_ATTRIBUTE_COLOR, "color");

	glLinkProgram( shader_data->p );
	if( !SDL_GL_LinkSuccessful(shader_data->p) ){

		GLchar buff[512];
		GLsizei len;
		glGetProgramInfoLog(shader_data->p, 512, &len, buff);

		SDL_SetError("SDL_Shader[GL]: Could not link shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader->bindShader(shader);

	glDeleteShader( v );
	glDeleteShader( f );

	shader_data->vport = SDL_createUniform(shader,"world_view_projection");
	SDL_GL_updateViewport(shader);

	shader_data->color_mode = SDL_createUniform(shader,"color_mode");
	glEnableVertexAttribArray(GL_ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(GL_ATTRIBUTE_TEXCOORD);
	glEnableVertexAttribArray(GL_ATTRIBUTE_COLOR);

	for( int i=0; i<8; i++ ) {
		SDL_Uniform* texN;
		texN = SDL_createUniform( shader,texture_names[i] );
		if( texN ) {
			SDL_GL_setUniform_i( texN,i );
			SDL_destroyUniform( shader, texN );
		}
	}

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
	if( shader_data->color_mode ) shader->destroyUniform(shader, shader_data->color_mode );
	if( shader_data->vport ) shader->destroyUniform(shader, shader_data->vport );
	if( shader_data->p ) glDeleteShader( shader_data->p );
	shader->unbindShader( shader );
	free( shader->driver_data );
	free( shader );
	return 0;
}


int SDL_GL_renderCopyShd(SDL_Shader* shader, SDL_Texture** textures,
	unsigned num_of_tex, SDL_Vertex* vertices, unsigned num_of_vert)
{
	int i;
	GL_TextureData *texturedata;
	SDL_GL_ShaderData *shader_data;
	GL_RenderData *data;

	for ( i=0; i<num_of_tex; i++ ) {
		texturedata = (GL_TextureData *) textures[i]->driverdata;
		if ( (  texturedata->texw != texturedata->texw ||
				texturedata->texh != texturedata->texh) && i!=0 )
		{
			return SDL_SetError("SDL_Shader[GL]: Textures must have the same size, "
					"internal representation missmatch");
		}
		if (texturedata->yuv) {
			return SDL_SetError("SDL_Shader[GL]: YUV-textures not supported");
		}
	}
	/* FIXME sampler2DRect needs texture coordinates in range (0..w)x(0..h) not (0..1)² */
	/* the size is scaled here and then scaled back at the end of the function */
	for ( i=0; i<num_of_vert; i++ ) {
		float tex_s, tex_t;
		vertices->getVertex( vertices, i,
				NULL, NULL, NULL, NULL, /*color*/
				NULL, NULL, NULL, /*position*/
				&tex_s, &tex_t);
		vertices->setVertexTexCoord( vertices, i, 1,
			tex_s * texturedata->texw, tex_t * texturedata->texh );
	}

	data = (GL_RenderData *) shader->renderer->driverdata;
	shader_data = (SDL_GL_ShaderData *) shader->driver_data;

	SDL_GL_MakeCurrent(shader->renderer->window, data->context);

	shader->bindShader(shader);
	if ( (shader->renderer->target == NULL && shader_data->is_target) ||
			(shader->renderer->target != NULL && !shader_data->is_target) ) {
		SDL_GL_updateViewport(shader);
		shader_data->is_target = shader->renderer->target != NULL;
	}

	for ( i=num_of_tex; i<8; i++ ) {
		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glDisable(GL_TEXTURE_2D);
	}
	for ( i=0; i<num_of_tex; i++ ) {
		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		SDL_GL_BindTexture(textures[i], NULL, NULL );
		glEnable(GL_TEXTURE_2D);
	}
	glActiveTextureARB( GL_TEXTURE0_ARB );

	/* invalidate SDLs intern shader */
	data->current.shader = SHADER_NONE;

	if ( shader_data->color_mode ) {
		SDL_GL_setUniform_i( shader_data->color_mode, textures[0]->format);
	}

	data->current.shader = SHADER_NONE;
	if (textures[0]->blendMode != data->current.blendMode) {
		switch (textures[0]->blendMode) {
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
		data->current.blendMode = textures[0]->blendMode;
	}

	SDL_GL_Vertex_t *vbuff = (SDL_GL_Vertex_t*) vertices->vertexBuffer;
    glVertexAttribPointer(GL_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE,
		sizeof(SDL_GL_Vertex_t), vbuff );
    glVertexAttribPointer(GL_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
		sizeof(SDL_GL_Vertex_t), &(vbuff[0].tex_s) );
	glVertexAttribPointer(GL_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE,
		sizeof(SDL_GL_Vertex_t), &(vbuff[0].r) );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_of_vert);

	shader->unbindShader(shader);
	/* FIXME */
	for ( i=0; i<num_of_vert; i++ ) {
		float tex_s, tex_t;
		vertices->getVertex( vertices, i,
				NULL, NULL, NULL, NULL, /*color*/
				NULL, NULL, NULL, /*position*/
				&tex_s, &tex_t);
		vertices->setVertexTexCoord( vertices, i, 1,
			tex_s / texturedata->texw, tex_t / texturedata->texh );
	}

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
