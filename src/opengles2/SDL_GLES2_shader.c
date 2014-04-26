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

	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
	Uint32 is_target;
} SDL_GLES2_ShaderData;
extern char* SDL_Shader_readRW( SDL_RWops* rwop );

typedef struct {
	GLuint p;
	GLint loc;
} SDL_GLES2_UniformData;

typedef struct {
	float x, y, z;
	float r, g, b, a;
	float tex_s, tex_t;
} SDL_GLES2_Vertex_t;

static const float inv255f = 1.0f / 255.0f;
static char texture_names[][32] = {"tex0","tex1","tex2","tex3","tex4","tex5","tex6","tex7"};
static char color_conv[1024];
static int is_init = 0;

void SDL_GLES2_getVertex( SDL_Vertex* vertices, unsigned num,
	uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a,
	float *x, float *y, float *z,
	float *tex_s, float *tex_t )
{
	SDL_GLES2_Vertex_t* vbuff = (SDL_GLES2_Vertex_t*) vertices->vertexBuffer;
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

void SDL_GLES2_setVertexColor(SDL_Vertex* vertices, unsigned from, unsigned num,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	SDL_GLES2_Vertex_t* vbuff = (SDL_GLES2_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].r = r * inv255f;
		vbuff[i].g = g * inv255f;
		vbuff[i].b = b * inv255f;
		vbuff[i].a = a * inv255f;
	}
}
void SDL_GLES2_setVertexPosition(SDL_Vertex* vertices, unsigned from,
	unsigned num, float x, float y, float z )
{
	SDL_GLES2_Vertex_t* vbuff = (SDL_GLES2_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].x = x;
		vbuff[i].y = y;
		vbuff[i].z = z;
	}
}
void SDL_GLES2_setVertexTexCoord(SDL_Vertex* vertices, unsigned from,
	unsigned num, float s, float t )
{
	SDL_GLES2_Vertex_t* vbuff = (SDL_GLES2_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].tex_s = s;
		vbuff[i].tex_t = t;
	}
}
SDL_Vertex* SDL_GLES2_createVertexBuffer( unsigned size ) {
	SDL_Vertex* ret = (SDL_Vertex*) malloc( sizeof(SDL_Vertex) );
	if ( !ret ) {
		SDL_OutOfMemory();
		return NULL;
	}
	ret->vertexBuffer = malloc( sizeof(SDL_GLES2_Vertex_t)*size );
	if ( !ret->vertexBuffer ) {
		free( ret );
		SDL_OutOfMemory();
		return NULL;
	}
	ret->size = size;
	ret->getVertex = SDL_GLES2_getVertex;
	ret->setVertexColor = SDL_GLES2_setVertexColor;
	ret->setVertexPosition = SDL_GLES2_setVertexPosition;
	ret->setVertexTexCoord = SDL_GLES2_setVertexTexCoord;
	return ret;
}
int SDL_GLES2_destroyVertexBuffer( SDL_Vertex* buff ) {
	free( buff->vertexBuffer );
	free( buff );
	return 0;
}


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
		SDL_GLES2_setUniform_matrix( shader_data->vport, (GLfloat*) projection );
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
	shader->updateViewport = SDL_GLES2_updateViewport;

	shader->createUniform =  SDL_GLES2_createUniform;
	shader->destroyUniform = SDL_GLES2_destroyUniform;

	shader->createVertexBuffer = SDL_GLES2_createVertexBuffer;
	shader->destroyVertexBuffer = SDL_GLES2_destroyVertexBuffer;

	shader_data->p = 0;
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
	if( !SDL_GLES2_CompileSuccessful(v) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(v, 512, &len, buff);

		SDL_SetError("SDL_Shader[GLES2]: Could not compile vertex shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}
	glCompileShader( f );
	if( !SDL_GLES2_CompileSuccessful(f) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(f, 512, &len, buff);

		SDL_SetError("SDL_Shader[GLES2]: Could not compile fragment shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader_data->p = glCreateProgram();

	glAttachShader( shader_data->p,v );
	glAttachShader( shader_data->p,f );

	glBindAttribLocation( shader_data->p, GLES2_ATTRIBUTE_POSITION, "position");
	glBindAttribLocation( shader_data->p, GLES2_ATTRIBUTE_TEXCOORD, "texCoords");
	glBindAttribLocation( shader_data->p, GLES2_ATTRIBUTE_COLOR, "color");

	glLinkProgram( shader_data->p );
	if( !SDL_GLES2_LinkSuccessful(shader_data->p) ){

		GLchar buff[512];
		GLsizei len;
		glGetProgramInfoLog(shader_data->p, 512, &len, buff);

		SDL_SetError("SDL_Shader[GLES2]: OpenGL Could not link shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader->bindShader(shader);

	glDeleteShader( v );
	glDeleteShader( f );

	shader_data->vport = SDL_createUniform(shader,"world_view_projection");
	SDL_GLES2_updateViewport( shader );

	shader_data->color_mode = SDL_createUniform(shader,"color_mode");
	glEnableVertexAttribArray(GLES2_ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
	glEnableVertexAttribArray(GLES2_ATTRIBUTE_COLOR);

	for( int i=0; i<8; i++ ) {
		SDL_Uniform* texN;
		texN = SDL_createUniform( shader,texture_names[i] );
		if( texN ) {
			SDL_GLES2_setUniform_i( texN,i );
			SDL_destroyUniform( shader, texN );
		}
	}

	return shader;
}

int SDL_GLES2_bindShader( SDL_Shader* shader ) {
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
	glUseProgram( shader_data->p );
	return glGetError();
}

int SDL_GLES2_unbindShader( SDL_Shader* shader ) {
	GLES2_DriverContext *data = (GLES2_DriverContext *) shader->renderer->driverdata;
	if ( data->current_program ) 
		glUseProgram( data->current_program->id );
	else
		glUseProgram( 0 );

	return glGetError();
}

int SDL_GLES2_destroyShader( SDL_Shader* shader ) {
	SDL_GLES2_ShaderData *shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;
	if (shader_data->color_mode )
		shader->destroyUniform( shader, shader_data->color_mode );
	if( shader_data->vport )
		shader->destroyUniform( shader, shader_data->vport );
	if ( shader_data->p )
		glDeleteShader( shader_data->p );
	free( shader->driver_data );
	free( shader );
	return 0;
}


int SDL_GLES2_renderCopyShd(SDL_Shader* shader, SDL_Texture** textures,
	unsigned num_of_tex, SDL_Vertex* vertices, unsigned num_of_vert)
{
	GLES2_DriverContext *data;
	SDL_GLES2_ShaderData *shader_data;
	int i;

	data = (GLES2_DriverContext *) shader->renderer->driverdata;
	shader_data = (SDL_GLES2_ShaderData*) shader->driver_data;

	SDL_GL_MakeCurrent(shader->renderer->window, data->context);

	shader->bindShader(shader);
	if ( (shader->renderer->target == NULL && shader_data->is_target) ||
			(shader->renderer->target != NULL && !shader_data->is_target) ) {
		SDL_GLES2_updateViewport(shader);
		shader_data->is_target = shader->renderer->target != NULL;
	}

	for ( i=num_of_tex; i<8; i++ ) {
		data->glActiveTexture( GL_TEXTURE0 + i );
		glDisable(GL_TEXTURE_2D);
	}

	for ( i=0; i<num_of_tex; i++ ) {
		data->glActiveTexture( GL_TEXTURE0 + i );
		SDL_GL_BindTexture(textures[i], NULL, NULL);
		glEnable(GL_TEXTURE_2D);
	}

	if ( shader_data->is_target ) {
		if ( shader_data->color_mode ) {
			SDL_GLES2_setUniform_i( shader_data->color_mode, shader->renderer->target->format);
		}
	} else {
		if ( shader_data->color_mode ) {
			SDL_GLES2_setUniform_i( shader_data->color_mode, textures[0]->format);
		}
	}

    if (textures[0]->blendMode != data->current.blendMode) {
		switch (textures[0]->blendMode) {
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
		data->current.blendMode = textures[0]->blendMode;
	}
	if( !data->current.tex_coords ){
		data->glEnableVertexAttribArray(GLES2_ATTRIBUTE_TEXCOORD);
		data->current.tex_coords = SDL_TRUE;
	}

	SDL_GLES2_Vertex_t *vbuff = (SDL_GLES2_Vertex_t*) vertices->vertexBuffer;
    data->glVertexAttribPointer(GLES2_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE,
		sizeof(SDL_GLES2_Vertex_t), vbuff);
    data->glVertexAttribPointer(GLES2_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
		sizeof(SDL_GLES2_Vertex_t), &(vbuff[0].tex_s) );
    data->glVertexAttribPointer(GLES2_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE,
		sizeof(SDL_GLES2_Vertex_t), &(vbuff[0].r) );

    data->glDrawArrays(GL_TRIANGLE_STRIP, 0, num_of_vert);

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
