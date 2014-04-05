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

#ifdef SDL_SHADER_D3D11
#ifndef _SDL_D3D11_shader_h
#define _SDL_D3D11_shader_h

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

void SDL_D3D11_hint(sdl_shader_hint flag, void* value);
SDL_Shader* SDL_D3D11_createShader( SDL_Renderer* renderer, const char *name );
int SDL_D3D11_destroyShader( SDL_Shader* shader );
int SDL_D3D11_bindShader( SDL_Shader* shader );
int SDL_D3D11_unbindShader( SDL_Shader* shader );
int SDL_D3D11_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_FRect * dstrect);


SDL_Uniform* SDL_D3D11_createUniform( SDL_Shader* shader, const char* name );
int SDL_D3D11_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform );

int SDL_D3D11_setUniform_fv( SDL_Uniform* uniform, float* vector, int num );
int SDL_D3D11_setUniform_f( SDL_Uniform* uniform, float a );
int SDL_D3D11_setUniform_f2( SDL_Uniform* uniform, float a, float b );
int SDL_D3D11_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c );
int SDL_D3D11_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d );


int SDL_D3D11_setUniform_iv( SDL_Uniform* uniform, int*   vector, int num );
int SDL_D3D11_setUniform_i ( SDL_Uniform* uniform, int a );
int SDL_D3D11_setUniform_i2( SDL_Uniform* uniform, int a, int b );
int SDL_D3D11_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c );
int SDL_D3D11_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d );

#ifdef __cplusplus
}
#endif

#endif /* _SDL_D3D11_shader_h */

#endif /* SDL_SHADER_D3D11 */
