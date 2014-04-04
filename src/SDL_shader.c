#include <SDL2/SDL.h>
#include "SDL_SYS_RenderStructs.h"
#include "SDL_shader.h"
#include "opengles2/SDL_GLES2_shader.h"
#include "opengl/SDL_GL_shader.h"
#include "d3d/SDL_D3D_shader.h"
#include "d3d11/SDL_D3D11_shader.h"


#include <stdio.h>
SDL_Shader* SDL_createShader( SDL_Renderer *renderer, const char* name ) {

	SDL_Shader* shader = NULL;
	SDL_SetError("SDL_Shader: No shading available for renderer '%s'\n", renderer->info.name);


#ifdef SDL_SHADER_OPENGL
	if( strcmp( renderer->info.name, "opengl" )==0 ) {
		shader = SDL_GL_createShader( renderer, name );
	}
#endif
	
/* opengles1.x didn't support shaders */

#ifdef SDL_SHADER_OPENGLES2
	if( strcmp( renderer->info.name, "opengles2" )==0 ) {
		shader = SDL_GLES2_createShader( renderer, name );
	}
#endif

#ifdef SDL_SHADER_D3D
	if( strcmp( renderer->info.name, "direct3d" )==0 ) {
		shader = SDL_D3D_createShader( renderer, name );
	}
#endif

#ifdef SDL_SHADER_D3D11
	if( strcmp( renderer->info.name, "direct3d11" )==0 ) {
		shader = SDL_D3D11_createShader( renderer, name );
	}
#endif

	return shader;

}


void SDL_hint( sdl_shader_hint flag, void* value ){
#ifdef SDL_SHADER_OPENGL
	SDL_GL_hint( flag, value );
#endif

#ifdef SDL_SHADER_OPENGLES2
	SDL_GLES2_hint( flag, value );
#endif
#ifdef SDL_SHADER_D3D
	SDL_D3D_hint( flag, value );
#endif
#ifdef SDL_SHADER_D3D11
	SDL_D3D11_hint( flag, value );
#endif
}


int SDL_destroyShader( SDL_Shader* shader ){
	return shader->destroyShader( shader );
}


int SDL_bindShader( SDL_Shader* shader ){
	return shader->bindShader( shader );
}

int SDL_renderCopyShd( SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_Rect * dstrect){

	SDL_Renderer *renderer = shader->renderer;
    SDL_Rect real_srcrect = { 0, 0, 0, 0 };
    SDL_Rect real_dstrect = { 0, 0, 0, 0 };
    SDL_FRect frect;

    if (renderer != texture->renderer) {
        return SDL_SetError("Texture was not created with this renderer");
    }

    real_srcrect.x = 0;
    real_srcrect.y = 0;
    real_srcrect.w = texture->w;
    real_srcrect.h = texture->h;
    if (srcrect) {
        if (!SDL_IntersectRect(srcrect, &real_srcrect, &real_srcrect)) {
            return 0;
        }
    }

    SDL_RenderGetViewport(renderer, &real_dstrect);
    real_dstrect.x = 0;
    real_dstrect.y = 0;
    if (dstrect) {
        if (!SDL_HasIntersection(dstrect, &real_dstrect)) {
            return 0;
        }
        real_dstrect = *dstrect;
    }

    if (texture->native) {
        texture = texture->native;
    }

    if (renderer->hidden) {
        return 0;
    }

    frect.x = real_dstrect.x * renderer->scale.x;
    frect.y = real_dstrect.y * renderer->scale.y;
    frect.w = real_dstrect.w * renderer->scale.x;
    frect.h = real_dstrect.h * renderer->scale.y;

	return shader->renderCopyShd( shader, texture, &real_srcrect, &frect );
}


char* textFileRead(const char* path) {
	FILE* f = fopen(path,"rb");
	if( f==NULL ) {
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;
	return string;
}

SDL_Uniform* SDL_createUniform( SDL_Shader* shader, const char* name ) {
	return shader->createUniform( shader, name );
}

int SDL_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform ) {
	return shader->destroyUniform( shader, uniform );
}

int SDL_setUniform_fv( SDL_Uniform* uniform, float* vector, int num ) {
	return uniform->setUniform_fv( uniform, vector, num );
}

int SDL_setUniform_iv( SDL_Uniform* uniform, int*   vector, int num ) {
	return uniform->setUniform_iv( uniform, vector, num );
}

int SDL_setUniform_f( SDL_Uniform* uniform, float a ){
	return uniform->setUniform_f( uniform, a );
}
int SDL_setUniform_f2( SDL_Uniform* uniform, float a, float b ){
	return uniform->setUniform_f2( uniform, a, b );
}
int SDL_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c ){
	return uniform->setUniform_f3( uniform, a, b, c );
}
int SDL_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d ){
	return uniform->setUniform_f4( uniform, a, b, c, d );
}

int SDL_setUniform_i ( SDL_Uniform* uniform, int a ){
	return uniform->setUniform_i( uniform, a );
}
int SDL_setUniform_i2( SDL_Uniform* uniform, int a, int b ){
	return uniform->setUniform_i2( uniform, a, b );
}
int SDL_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c ){
	return uniform->setUniform_i3( uniform, a, b, c );
}
int SDL_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d ){
	return uniform->setUniform_i4( uniform, a, b, c, d );
}

