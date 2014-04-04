
#ifndef _SDL_shader_h
#define _SDL_shader_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "SDL_SYS_RenderStructs.h" //FIXME renderCopyShd needs SDL_FRect def

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
               const SDL_Rect * srcrect, const SDL_FRect * dstrect);
	
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

