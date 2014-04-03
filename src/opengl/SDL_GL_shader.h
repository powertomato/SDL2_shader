
#ifdef SDL_SHADER_OPENGL

#ifndef _SDL_GL_shader_h
#define _SDL_GL_shader_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

void SDL_GL_hint(sdl_shader_hint flag, void* value);
SDL_Shader* SDL_GL_createShader( SDL_Renderer* renderer, const char *name );
int SDL_GL_destroyShader( SDL_Shader* shader );
int SDL_GL_bindShader( SDL_Shader* shader );
int SDL_GL_unbindShader( SDL_Shader* shader );
int SDL_GL_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect);


SDL_Uniform* SDL_GL_createUniform( SDL_Shader* shader, const char* name );
int SDL_GL_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform );

int SDL_GL_setUniform_fv( SDL_Uniform* uniform, float* vector, int num );
int SDL_GL_setUniform_f ( SDL_Uniform* uniform, float a );
int SDL_GL_setUniform_f2( SDL_Uniform* uniform, float a, float b );
int SDL_GL_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c );
int SDL_GL_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d );

int SDL_GL_setUniform_iv( SDL_Uniform* uniform, int* vector, int num );
int SDL_GL_setUniform_i ( SDL_Uniform* uniform, int a );
int SDL_GL_setUniform_i2( SDL_Uniform* uniform, int a,int b );
int SDL_GL_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c );
int SDL_GL_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d );

#ifdef __cplusplus
}
#endif

#endif /* _SDL_GL_shader_h */

#endif /* SDL_SHADER_OPENGL */
