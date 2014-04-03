
#ifdef SDL_SHADER_D3D
#ifndef _SDL_D3D_shader_h
#define _SDL_D3D_shader_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

void SDL_D3D_hint(sdl_shader_hint flag, void* value);
SDL_Shader* SDL_D3D_createShader( SDL_Renderer* renderer, const char *name );
int SDL_D3D_destroyShader( SDL_Shader* shader );
int SDL_D3D_bindShader( SDL_Shader* shader );
int SDL_D3D_unbindShader( SDL_Shader* shader );
int SDL_D3D_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect);


SDL_Uniform* SDL_D3D_createUniform( SDL_Shader* shader, const char* name );
int SDL_D3D_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform );

int SDL_D3D_setUniform_fv( SDL_Uniform* uniform, float* vector, int num );
int SDL_D3D_setUniform_f( SDL_Uniform* uniform, float a );
int SDL_D3D_setUniform_f2( SDL_Uniform* uniform, float a, float b );
int SDL_D3D_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c );
int SDL_D3D_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d );


int SDL_D3D_setUniform_iv( SDL_Uniform* uniform, int*   vector, int num );
int SDL_D3D_setUniform_i ( SDL_Uniform* uniform, int a );
int SDL_D3D_setUniform_i2( SDL_Uniform* uniform, int a, int b );
int SDL_D3D_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c );
int SDL_D3D_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d );

#ifdef __cplusplus
}
#endif

#endif /* _SDL_D3D_shader_h */

#endif /* SDL_SHADER_D3D */
