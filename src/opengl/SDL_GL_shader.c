#ifdef SDL_SHADER_OPENGL

#include <stdio.h>
#ifdef _WIN32
#include <SDL2/SDL_opengl.h>
//#define GLEW_STATIC
//#include <GL/glew.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

#include "../SDL_shader.h"
#include "SDL_GL_shader.h"
#include "SDL_GL_RenderStructs.h"
#include "SDL_glfunc_ptr.h"

typedef struct {
	GLuint v;
	GLuint f;
	GLuint p;

	SDL_Uniform* color;
	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
} SDL_GL_ShaderData;
extern char* textFileRead(const char* path);

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

int SDL_GL_CompileSuccessful(int obj) {
  int status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  return status == GL_TRUE;
}

int SDL_GL_LinkSuccessful(int obj) {
  int status;
  glGetProgramiv(obj, GL_LINK_STATUS, &status);
  return status == GL_TRUE;
}

int SDL_GL_setUniform_matrix( SDL_Uniform* uniform, GLfloat* mat );
SDL_Shader* SDL_GL_createShader( SDL_Renderer* renderer, const char *name){
	//TODO error handling, on error return NULL and SDL_SetError(fmt,...)
	// Does geometry shaders and tessalation make any sense?

	SDL_Shader *shader;
	char *vs,*fs;
	if( !is_init ){
		SDL_GL_init();
	}

	shader = (SDL_Shader*) malloc( sizeof(SDL_Shader) );
	shader->driver_data = malloc( sizeof(SDL_GL_ShaderData ) );
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
	shader_data->v = glCreateShader( GL_VERTEX_SHADER );
	shader_data->f = glCreateShader( GL_FRAGMENT_SHADER );
	shader_data->color = NULL;
	shader_data->vport = NULL;
	shader_data->color_mode = NULL;

	int name_len = strlen( name );
	int ext_len =  8; //strlen(".gl.xxxx");

	char *file_name = malloc(name_len + ext_len + 1 );
	strncpy( file_name, name, name_len );
	file_name[ name_len + ext_len ] = '\0';

	strncpy( file_name + name_len, ".gl.vert", ext_len );
	vs = textFileRead( file_name );
	if( vs==NULL ){
		SDL_SetError("SDL_Shader: OpenGL Could not open file '%s'\n", file_name);
		free(file_name);
		shader->destroyShader(shader);
		return NULL;
	}

	strncpy( file_name + name_len, ".gl.frag", ext_len );
	fs = textFileRead( file_name );
	if( fs==NULL ){
		SDL_SetError("SDL_Shader: OpenGL Could not open file '%s'\n", file_name);
		free(fs);
		free(file_name);
		shader->destroyShader(shader);
		return NULL;
	}

	const char* vss = (const char*) vs;
	const char *fss[] = {fs, color_conv};
	glShaderSource(shader_data->v, 1, &vss,NULL);
	glShaderSource(shader_data->f, 2, fss,NULL);

	free(file_name);
	free(vs);
	free(fs);

	glCompileShader( shader_data->v );
	if( !SDL_GL_CompileSuccessful(shader_data->v) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(shader_data->v, 512, &len, buff);

		SDL_SetError("SDL_Shader: OpenGL Could not compile vertex shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}
	glCompileShader( shader_data->f );
	if( !SDL_GL_CompileSuccessful(shader_data->f) ){

		GLchar buff[512];
		GLsizei len;
		glGetShaderInfoLog(shader_data->f, 512, &len, buff);

		SDL_SetError("SDL_Shader: OpenGL Could not compile fragment shader: \n-------\n%s\n-------\n", buff );
		shader->destroyShader(shader);
		return NULL;
	}

	shader_data->p = glCreateProgram();

	glAttachShader( shader_data->p,shader_data->v );
	glAttachShader( shader_data->p,shader_data->f );

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

	glDeleteShader( shader_data->v );
	glDeleteShader( shader_data->f );

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
	// TODO check gl, sdl return code compatibility, error handling
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	glUseProgram( shader_data->p ); //XXX
	return 0;
}

int SDL_GL_unbindShader( SDL_Shader* shader ) {
	// TODO check gl, sdl return code compatibility, error handling
	glUseProgram( 0 ); //XXX
	return 0;
}

int SDL_GL_destroyShader( SDL_Shader* shader ) {
	// SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	// TODO GL destroy stuff
	// 
	if( !shader )
		return 0;
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	if( shader_data->f ) glDeleteShader( shader_data->f );
	if( shader_data->v ) glDeleteShader( shader_data->v );
	if( shader_data->p ) glDeleteShader( shader_data->p );
	shader->unbindShader( shader );
	shader->destroyUniform(shader, shader_data->color );
	shader->destroyUniform(shader, shader_data->vport );
	free( shader->driver_data );
	free( shader );
	return 0;
}


int SDL_GL_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect) {

	float width, height;
	GL_RenderData *data = (GL_RenderData *) shader->renderer->driverdata;
    GL_TextureData *texturedata = (GL_TextureData *) texture->driverdata;
	SDL_GL_ShaderData *shader_data = (SDL_GL_ShaderData *) shader->driver_data;
    GLfloat minu, maxu, minv, maxv;

    if (texturedata->yuv) {
        return SDL_SetError("SDL_Shader: OpenGL YUV-textures not supported");
    }

	SDL_GL_MakeCurrent(shader->renderer->window, data->context);
	SDL_GL_BindTexture(texture, &width, &height );

	shader->bindShader(shader);
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

/*  data->glBegin(GL_TRIANGLE_STRIP);
	data->glTexCoord2f(minu, minv);
    data->glVertex2f(dstrect->x, dstrect->y );
    data->glTexCoord2f(maxu, minv);
    data->glVertex2f(dstrect->x + dstrect->w, dstrect->y );
    data->glTexCoord2f(minu, maxv);
    data->glVertex2f(dstrect->x, dstrect->y + dstrect->h);
    data->glTexCoord2f(maxu, maxv);
    data->glVertex2f(dstrect->x + dstrect->w, dstrect->y + dstrect->h);
    data->glEnd(); */

    GLfloat vertices[8];
    GLfloat texCoords[8];

    vertices[0] = dstrect->x;
    vertices[1] = dstrect->y;
    vertices[2] = (dstrect->x + dstrect->w);
    vertices[3] = dstrect->y;
    vertices[4] = dstrect->x;
    vertices[5] = (dstrect->y + dstrect->h);
    vertices[6] = vertices[2]; //(dstrect->x + dstrect->w);
    vertices[7] = vertices[5]; //(dstrect->y + dstrect->h);
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
	return 0;
}

SDL_Uniform* SDL_GL_createUniform( SDL_Shader* shader, const char* name ) {
	GLint loc;
	SDL_GL_ShaderData* shader_data = (SDL_GL_ShaderData*) shader->driver_data;
	loc = glGetUniformLocation(shader_data->p, name);
	if( loc == -1 )
		return NULL;

	SDL_Uniform* uniform = (SDL_Uniform*) malloc( sizeof(SDL_Uniform) );
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*) malloc( sizeof(SDL_GL_UniformData) );

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
	if( !uniform )
		return 0;
	free(uniform->driver_data);
	free(uniform);
	return 0;
}

int SDL_GL_setUniform_matrix( SDL_Uniform* uniform, GLfloat* mat ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniformMatrix4fv( udata->loc, 1, GL_FALSE, mat );
	return 0;
}

int SDL_GL_setUniform_fv( SDL_Uniform* uniform, float* vector, int num ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1fv( udata->loc, num, vector);
	return 0;
}
int SDL_GL_setUniform_f(  SDL_Uniform* uniform, float a ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1f(udata->loc, a );
	return 0;
}
int SDL_GL_setUniform_f2( SDL_Uniform* uniform, float a, float b ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform2f(udata->loc, a,b );
	return 0;
}
int SDL_GL_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform3f(udata->loc, a,b,c );
	return 0;
}
int SDL_GL_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform4f(udata->loc, a,b,c,d );
	return 0;
}


int SDL_GL_setUniform_iv( SDL_Uniform* uniform, int* vector, int num ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1iv( udata->loc, num, vector);
	return 0;
}
int SDL_GL_setUniform_i(  SDL_Uniform* uniform, int a ) {
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform1i(udata->loc, a );
	return 0;
}
int SDL_GL_setUniform_i2( SDL_Uniform* uniform, int a, int b ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform2i(udata->loc, a,b );
	return 0;
}
int SDL_GL_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform3i(udata->loc, a,b,c );
	return 0;
}
int SDL_GL_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d ){
	SDL_GL_UniformData* udata = (SDL_GL_UniformData*)uniform->driver_data;
	glUniform4i(udata->loc, a,b,c,d );
	return 0;
}

#else
extern void __suppress_a_warning__();
#endif /* SDL_SHADER_OPENGL */
