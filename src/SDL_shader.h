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

#ifndef _SDL_shader_h
#define _SDL_shader_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SDL_GLSL_VERSION,
	SDL_D3D_VS_MAJOR_VERSION,
	SDL_D3D_VS_MINOR_VERSION,
	SDL_D3D_PS_MAJOR_VERSION,
	SDL_D3D_PS_MINOR_VERSION,

	SDL_GL_TEX0_NAME,
	SDL_GL_TEX1_NAME,
	SDL_GL_TEX2_NAME,
	SDL_GL_TEX3_NAME,
	SDL_GL_TEX4_NAME,
	SDL_GL_TEX5_NAME,
	SDL_GL_TEX6_NAME,
	SDL_GL_TEX7_NAME,
} sdl_shader_hint;

typedef struct {
	char* vshader;
	char* pshader;
} SDL_ShaderFileNames;

typedef struct {
	SDL_RWops* vshader;
	SDL_RWops* pshader;
} SDL_ShaderStream;

typedef struct SDL_Shader_t SDL_Shader;
typedef struct SDL_Uniform_t SDL_Uniform;
struct SDL_Uniform_t{
	SDL_Shader* shader;

	int (*setUniform_fv)( SDL_Uniform*, float* vector, int num );
	int (*setUniform_iv)( SDL_Uniform*, int* vector, int num );

	int (*setUniform_f )( SDL_Uniform* uniform, float a );
	int (*setUniform_f2)( SDL_Uniform* uniform, float a, float b );
	int (*setUniform_f3)( SDL_Uniform* uniform, float a, float b, float c );
	int (*setUniform_f4)( SDL_Uniform* uniform, float a, float b, float c, float d );

	int (*setUniform_i )( SDL_Uniform* uniform, int a );
	int (*setUniform_i2)( SDL_Uniform* uniform, int a, int b );
	int (*setUniform_i3)( SDL_Uniform* uniform, int a, int b, int c );
	int (*setUniform_i4)( SDL_Uniform* uniform, int a, int b, int c, int d );

	void* driver_data;
};

typedef struct SDL_Vertex_t SDL_Vertex;
struct SDL_Vertex_t {
	unsigned size;
	void* vertexBuffer;
	void (*setVertexColor)(SDL_Vertex* vertices, unsigned from,
			unsigned num, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void (*setVertexPosition)(SDL_Vertex* vertices, unsigned from,
			unsigned num, float x, float y, float z );
	void (*setVertexTexCoord)(SDL_Vertex* vertices, unsigned from,
			unsigned num, float s, float t );
	void (*getVertex)( SDL_Vertex* vertices, unsigned num,
			uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a,
			float *x, float *y, float *z,
			float *tex_s, float *tex_t );
};

struct SDL_Shader_t {
	SDL_Renderer* renderer;

	SDL_Shader* (*create_shader)( SDL_Renderer *renderer, SDL_ShaderStream* str );

	int (*bindShader)( SDL_Shader* shader );
	int (*unbindShader)( SDL_Shader* shader );
	int (*destroyShader)( SDL_Shader* shader );

	int (*renderCopyShd)(SDL_Shader* shader, SDL_Texture** textures,
			unsigned num_of_tex, SDL_Vertex* vertices, unsigned num_of_vert);
	void (*updateViewport)( SDL_Shader *shader );
	
	SDL_Uniform* (*createUniform)( SDL_Shader* shader, const char* name );
	int (*destroyUniform)( SDL_Shader* shader, SDL_Uniform* uniform );

	SDL_Vertex* (*createVertexBuffer)( unsigned size );
	int (*destroyVertexBuffer)( SDL_Vertex* buff );

	void* driver_data;
};



void SDL_shader_hint(sdl_shader_hint flag, void* value);

SDL_ShaderFileNames SDL_getShaderFileNames( SDL_Renderer* renderer, 
	const char* name );
SDL_ShaderStream SDL_getShaderStream( SDL_ShaderFileNames* names, int delete_ );

SDL_Shader* SDL_createShader( SDL_Renderer *renderer, const char* name );
SDL_Shader* SDL_createShader_RW( SDL_Renderer *renderer, 
	SDL_ShaderStream* shdstream, int close );
int SDL_destroyShader( SDL_Shader* shader );
int SDL_bindShader( SDL_Shader* shader );
int SDL_renderCopyShd( SDL_Shader* shader, SDL_Texture* texture,
		const SDL_Rect * srcrect, const SDL_Rect * dstrect);
int SDL_renderCopyShdArray( SDL_Shader* shader, SDL_Texture** textures,
		const SDL_Rect * srcrect, const SDL_Rect * dstrect, unsigned num);
int SDL_renderVertexBuffer( SDL_Shader* shader, SDL_Vertex *vertices,
		SDL_Texture** textures, unsigned num_of_tex);
void SDL_updateViewport( SDL_Shader *shader );

SDL_Uniform* SDL_createUniform( SDL_Shader* shader, const char* name );
int SDL_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform );

int SDL_setUniform_fv( SDL_Uniform* uniform, float* vector, int num );
int SDL_setUniform_iv( SDL_Uniform* uniform, int*   vector, int num );

int SDL_setUniform_f ( SDL_Uniform* uniform, float a );
int SDL_setUniform_f2( SDL_Uniform* uniform, float a, float b );
int SDL_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c );
int SDL_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d );

int SDL_setUniform_i ( SDL_Uniform* uniform, int a );
int SDL_setUniform_i2( SDL_Uniform* uniform, int a, int b );
int SDL_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c );
int SDL_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d );

#ifdef __cplusplus
}
#endif

#endif /* _SDL_shader_h */

