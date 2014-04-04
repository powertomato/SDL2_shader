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

	SDL_D3D11_VS_MAJOR_VERSION,
	SDL_D3D11_VS_MINOR_VERSION,
	SDL_D3D11_PS_MAJOR_VERSION,
	SDL_D3D11_PS_MINOR_VERSION,
} sdl_shader_hint;

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

struct SDL_Shader_t {
	SDL_Renderer* renderer;

	SDL_Shader* (*create_shader)( SDL_Renderer *renderer, const char* name );
	int (*bindShader)( SDL_Shader* shader );
	int (*unbindShader)( SDL_Shader* shader );
	int (*destroyShader)( SDL_Shader* shader );
	int (*renderCopyShd)( SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_Rect * dstrect);
	
	SDL_Uniform* (*createUniform)( SDL_Shader* shader, const char* name );
	int (*destroyUniform)( SDL_Shader* shader, SDL_Uniform* uniform );

	void* driver_data;
};



void SDL_hint(sdl_shader_hint flag, void* value);
SDL_Shader* SDL_createShader( SDL_Renderer *renderer, const char* name );
int SDL_destroyShader( SDL_Shader* shader );
int SDL_bindShader( SDL_Shader* shader );
int SDL_renderCopyShd( SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_Rect * dstrect);

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

