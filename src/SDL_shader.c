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

#include <SDL2/SDL.h>
#include <string.h>
#include "SDL_SYS_RenderStructs.h"
#include "SDL_shader.h"
#include "opengles2/SDL_GLES2_shader.h"
#include "opengl/SDL_GL_shader.h"
#include "d3d/SDL_D3D_shader.h"
//#include "d3d11/SDL_D3D11_shader.h"


#include <stdio.h>
SDL_Shader* SDL_createShader( SDL_Renderer *renderer, const char* name ) {
	SDL_ShaderFileNames names = SDL_getShaderFileNames( renderer, name );
	SDL_ShaderStream stream = SDL_getShaderStream( &names, 1 );
	return SDL_createShader_RW( renderer, &stream, 1 );
}

SDL_Shader* SDL_createShader_RW( SDL_Renderer *renderer,
	SDL_ShaderStream* shdstream, int close )
{
	SDL_Shader* shader = NULL;
	SDL_SetError("SDL_Shader: No shading available for renderer '%s'\n", renderer->info.name);


#ifdef SDL_SHADER_OPENGL
	if( strcmp( renderer->info.name, "opengl" )==0 ) {
		shader = SDL_GL_createShader( renderer, shdstream );
	}
#endif
	
/* opengles1.x didn't support shaders */

#ifdef SDL_SHADER_OPENGLES2
	if( strcmp( renderer->info.name, "opengles2" )==0 ) {
		shader = SDL_GLES2_createShader( renderer, shdstream );
	}
#endif

#ifdef SDL_SHADER_D3D
	if( strcmp( renderer->info.name, "direct3d" )==0 ) {
		shader = SDL_D3D_createShader( renderer, shdstream );
	}
#endif

#ifdef SDL_SHADER_D3D11
	if( strcmp( renderer->info.name, "direct3d11" )==0 ) {
		shader = SDL_D3D11_createShader( renderer, shdstream );
	}
#endif

	return shader;

}

void SDL_shader_hint( sdl_shader_hint flag, void* value ){
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

SDL_ShaderFileNames SDL_getShaderFileNames( SDL_Renderer* renderer,
	const char* name )
{

	char* vshd_ext;
	char* pshd_ext;
	int vshd_ext_len;
	int pshd_ext_len;
#ifdef SDL_SHADER_OPENGL
	char vshd_ext_gl[] = ".gl.vert";
	char pshd_ext_gl[] = ".gl.frag";
	if( strcmp( renderer->info.name, "opengl" )==0 ) {
		vshd_ext_len = sizeof( vshd_ext_gl );
		vshd_ext = vshd_ext_gl;
		pshd_ext_len = sizeof( pshd_ext_gl );
		pshd_ext = pshd_ext_gl;
	}
#endif 
#ifdef SDL_SHADER_OPENGLES2
	char vshd_ext_gles2[] = ".gles2.vert";
	char pshd_ext_gles2[] = ".gles2.frag";
	if( strcmp( renderer->info.name, "opengles2" )==0 ) {
		vshd_ext_len = sizeof( vshd_ext_gles2 );
		vshd_ext = vshd_ext_gles2;
		pshd_ext_len = sizeof( pshd_ext_gles2 );
		pshd_ext = pshd_ext_gles2;
	}
#endif
#ifdef SDL_SHADER_D3D
	char both_shd_ext_d3d[] = ".d3d.hlsl"; /* HLSL uses a single file */
	if( strcmp( renderer->info.name, "direct3d" )==0 ) {
		vshd_ext_len = sizeof( both_shd_ext_d3d );
		vshd_ext = both_shd_ext_d3d;
		pshd_ext_len = 0;
		pshd_ext = NULL;
	}
#endif
#ifdef SDL_SHADER_D3D11
	
#endif

	SDL_ShaderFileNames ret = {NULL,NULL};
	int name_len = strlen( name );

	ret.vshader = malloc(name_len + vshd_ext_len + 1 );
	if( !ret.vshader ){
		SDL_OutOfMemory();
		return ret;
	}
	if( pshd_ext ) {
		ret.pshader = malloc(name_len + pshd_ext_len + 1 );
		if( !ret.pshader ){
			free( ret.vshader );
			ret.vshader = NULL;
			SDL_OutOfMemory();
			return ret;
		}
	}
	ret.vshader[ name_len + vshd_ext_len ] = '\0';
	strncpy( ret.vshader, name, name_len );
	strncpy( ret.vshader + name_len, vshd_ext, vshd_ext_len );

	if( pshd_ext ){
		ret.pshader[ name_len + pshd_ext_len ] = '\0';
		strncpy( ret.pshader, name, name_len );
		strncpy( ret.pshader + name_len, pshd_ext, pshd_ext_len );
	}

	return ret;
}

SDL_ShaderStream SDL_getShaderStream( SDL_ShaderFileNames* names, int delete ){
	SDL_ShaderStream ret;
	ret.vshader = SDL_RWFromFile( names->vshader, "rb" );
	ret.pshader = SDL_RWFromFile( names->pshader, "rb" );
	if( delete ){
		if( names->vshader ) {
			free( names->vshader);
		}
		if( names->pshader ) {
			free( names->pshader);
		}
	}
	return ret;
}

int SDL_destroyShader( SDL_Shader* shader ){
	return shader->destroyShader( shader );
}

int SDL_bindShader( SDL_Shader* shader ){
	return shader->bindShader( shader );
}
void SDL_updateViewport( SDL_Shader *shader ){
	shader->updateViewport( shader );
}
int SDL_renderVertexBuffer( SDL_Shader* shader, SDL_Vertex *vertices,
		SDL_Texture** textures, unsigned num_of_tex)
{
	unsigned i;
	SDL_Renderer* renderer = shader->renderer;

	if ( num_of_tex==0 )
		return 0;
	if ( num_of_tex >= 8 )
		return SDL_SetError("SDL_Shader: only 8 textures are supported");
	for ( i=0; i<num_of_tex; i++ ) {
		if (( textures[i]->w != textures[0]->w ||
			  textures[i]->h != textures[0]->h ) && i!=0 )
		{
			return SDL_SetError("SDL_Shader: Textures must have the same size");
		}
		if (( textures[i]->format != textures[0]->format) && i!=0 ) {
			return SDL_SetError("SDL_Shader: Textures must have the same format");
		}
		if (renderer != textures[i]->renderer) {
			return SDL_SetError("SDL_Shader: Texture was not created with this renderer");
		}
	}

	return shader->renderCopyShd( shader, textures, num_of_tex, 
		vertices, vertices->size );
}

int SDL_renderCopyShdArray( SDL_Shader* shader, SDL_Texture** textures,
		const SDL_Rect * srcrect, const SDL_Rect * dstrect, unsigned num_of_tex)
{

	SDL_Renderer *renderer = shader->renderer;

    if (renderer->hidden) {
		return 0;
	}

    SDL_Rect real_srcrect = { 0, 0, 0, 0 };
    SDL_Rect real_dstrect = { 0, 0, 0, 0 };
	unsigned i;

    real_srcrect.x = 0;
    real_srcrect.y = 0;
    real_srcrect.w = textures[0]->w;
    real_srcrect.h = textures[0]->h;
    if (srcrect) {
        if (!SDL_IntersectRect(srcrect, &real_srcrect, &real_srcrect)) {
            return 0;
        }
    }

	//XXX bug here
    //SDL_RenderGetViewport(renderer, &real_dstrect);
    if (dstrect) {
        /*if (!SDL_HasIntersection(dstrect, &real_dstrect)) {
            return 0;
        }*/
        real_dstrect = *dstrect;
    }

	for( i=0; i<num_of_tex; i++ ) {
		if (textures[i]->native) {
			textures[i] = textures[i]->native;
		}
	}

    /*real_dstrect.x *= renderer->scale.x;
    real_dstrect.y *= renderer->scale.y;
    real_dstrect.w *= renderer->scale.x;
    real_dstrect.h *= renderer->scale.y;*/

	SDL_Vertex* vertices = shader->createVertexBuffer(4);

	int r,g,b,a;
	r = textures[0]->r;
	g = textures[0]->g;
	b = textures[0]->b;
	a = textures[0]->a;
	vertices->setVertexColor( vertices, 0,4, r,g,b,a);

	vertices->setVertexPosition( vertices, 0,1,
		real_dstrect.x, real_dstrect.y, 0.0f);
	vertices->setVertexPosition( vertices, 1,1,
		(real_dstrect.x + real_dstrect.w), real_dstrect.y, 0.0f);
	vertices->setVertexPosition( vertices, 2,1,
		real_dstrect.x, (real_dstrect.y + real_dstrect.h), 0.0f);
	vertices->setVertexPosition( vertices, 3,1,
		(real_dstrect.x + real_dstrect.w),
		(real_dstrect.y + real_dstrect.h), 0.0f);

    float minu, maxu, minv, maxv;
    minu = real_srcrect.x / textures[0]->w;
    maxu = (real_srcrect.x + real_srcrect.w) / textures[0]->w;
    minv = real_srcrect.y / textures[0]->h;
    maxv = (real_srcrect.y + real_srcrect.h) / textures[0]->h;
	vertices->setVertexTexCoord( vertices, 0,1, minu, minv);
	vertices->setVertexTexCoord( vertices, 1,1, maxu, minv);
	vertices->setVertexTexCoord( vertices, 2,1, minu, maxv);
	vertices->setVertexTexCoord( vertices, 3,1, maxu, maxv);

	int res = SDL_renderVertexBuffer( shader, vertices, textures, num_of_tex );
	shader->destroyVertexBuffer( vertices );
	return res;
}


int SDL_renderCopyShd( SDL_Shader* shader, SDL_Texture* texture,
               const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{
	SDL_Texture *textures[] = {texture};
	return SDL_renderCopyShdArray( shader, textures, srcrect, dstrect, 1 );
}

char* SDL_Shader_readRW( SDL_RWops* rwop ) {
	Sint64 fsize = rwop->size(rwop);
	char *string = malloc(fsize + 1);
	if( !string ){
		return string;
	}

	rwop->read(rwop, string, fsize, 1);
	string[fsize] = '\0';

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

