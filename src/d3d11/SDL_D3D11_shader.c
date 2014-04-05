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

#include <stdio.h>
#include <SDL2/SDL.h>

#define COBJMACROS
#define INITGUID
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#undef INITGUID
#undef COBJMACROS

#include "../SDL_shader.h"
#include "SDL_D3D11_shader.h"

#include "SDL_D3D11_RenderStructs.h"
#include "SDL_D3D11_SDL_internals.c"

#define SAFE_RELEASE(X) if ((X)) { IUnknown_Release(SDL_static_cast(IUnknown*, X)); X = NULL; }

#ifndef ID3D11Device_CreatePixelShader
#error
#endif

typedef struct {
	ID3D11VertexShader *vert_shader;
	ID3D11PixelShader *pixl_shader;

	ID3D11ShaderReflection* pReflector

	SDL_Uniform* color;
	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
} SDL_D3D11_ShaderData;

/*typedef struct {
	LPD3D11XCONSTANTTABLE table;
	D3D11XHANDLE handle;
} SDL_D3D11_UniformData;*/

static int vs_version_major = 1;
static int vs_version_minor = 1;
static int ps_version_major = 2;
static int ps_version_minor = 0;

void SDL_D3D11_hint(sdl_shader_hint flag, void* value) {
	int tmp;
	switch(flag) {
		case SDL_D3D11_VS_MAJOR_VERSION:
			tmp = *((int*)value);
			vs_version_major = tmp >= 0 && tmp <= 9 ? tmp : vs_version_major;
			break;
		case SDL_D3D11_VS_MINOR_VERSION:
			tmp = *((int*)value);
			vs_version_minor = tmp >= 0 && tmp <= 9 ? tmp : vs_version_minor;
			break;
		case SDL_D3D11_PS_MAJOR_VERSION:
			tmp = *((int*)value);
			ps_version_major = tmp >= 0 && tmp <= 9 ? tmp : ps_version_major;
			break;
		case SDL_D3D11_PS_MINOR_VERSION:
			tmp = *((int*)value);
			ps_version_minor = tmp >= 0 && tmp <= 9 ? tmp : ps_version_minor;
			break;
		default: /* nothing */ break;
	}

}

extern char* textFileRead(const char* path);
static int SDL_D3D11_setUniform_matrix( SDL_Uniform* uniform, Float4X4* matix );
SDL_Shader* SDL_D3D11_createShader( SDL_Renderer* renderer, const char *name ) {

	SDL_Shader *shader;

	shader = (SDL_Shader*) malloc( sizeof(SDL_Shader) );
	shader->driver_data = malloc( sizeof(SDL_D3D11_ShaderData ) );
	SDL_D3D11_ShaderData *shader_data = (SDL_D3D11_ShaderData*) shader->driver_data;
	D3D11_RenderData *render_data = (D3D11_RenderData*) renderer->driverdata;

	shader->renderer = renderer;
	shader->create_shader = SDL_D3D11_createShader;
	shader->destroyShader =  SDL_D3D11_destroyShader;
	shader->bindShader =     SDL_D3D11_bindShader;
	shader->unbindShader =   SDL_D3D11_unbindShader;
	shader->renderCopyShd =  SDL_D3D11_renderCopyShd;

	shader->createUniform  = SDL_D3D11_createUniform;
	shader->destroyUniform = SDL_D3D11_destroyUniform;

	HRESULT result;
	int name_len = strlen( name );
	int ext_len =  11; //strlen(".d3d11x.cso");

	char *file_name = malloc(name_len + ext_len + 1 );
	strncpy( file_name, name, name_len );
	file_name[ name_len + ext_len ] = '\0';


	strncpy( file_name + name_len, ".d3d11v.cso", ext_len );
	printf("d11 vertex shader: %s\n",file_name);
	char* vbuff = textFileRead( file_name );
	if( vbuff ){
		result = ID3D11Device_CreateVertexShader(render_data->d3dDevice,
				(DWORD*) vbuff,
				strlen(vbuff),
				NULL,
				&shader_data->vert_shader);
		if (FAILED(result)) {
			shader->destroyShader(shader);
			SDL_SetError("SDL_Shader: D3D11 CreateVertexShader() failed, errno %d\n", result);
			return NULL;
		}
		free( vbuff );
	}else{
		return NULL;
	}


	file_name[name_len+6]='p';
	printf("d11 pixel shader: %s\n",file_name);
	char* pbuff = textFileRead( file_name );
	if( pbuff ){
		result = ID3D11Device_CreatePixelShader(render_data->d3dDevice,
				(DWORD*) pbuff,
				strlen(pbuff),
				NULL,
				&shader_data->pixl_shader);
		if (FAILED(result)) {
			shader->destroyShader(shader);
			SDL_SetError("SDL_Shader: D3D11 CreatePixelShader() failed, errno %d\n", result);
			return NULL;
		}

		shader_data->pReflector = NULL;
		result = D3DReflect( 
				(ID3D11ShaderReflection*) pbuff,
				strlen(pbuff), 
				(REFIID) &IID_ID3D11ShaderReflection,
				(void**) &(shader_data->pReflector) );
		if (FAILED(result)) {
			shader->destroyShader(shader);
			SDL_SetError("SDL_Shader: D3D11 D3DReflect() failed, errno %d\n", result);
			return NULL;
		}

		free( pbuff );
	}else{
		return NULL;
	}
	free(file_name);

	shader_data->vport = SDL_createUniform(shader, "world_view_projection");
	if( shader_data->vport ){
	    Float4X4 matrix;
		matrix.m[0][0] = 2.0f / renderer->viewport.w;
		matrix.m[1][0] = 0.0f;
		matrix.m[2][0] = 0.0f;
		matrix.m[3][0] = 0.0f;

		matrix.m[0][1] = 0.0f;
		matrix.m[1][1] =-2.0f / renderer->viewport.h;
		matrix.m[2][1] = 0.0f;
		matrix.m[3][1] = 0.0f;

		matrix.m[0][2] = 0.0f;
		matrix.m[1][2] = 0.0f;
		matrix.m[2][2] = 0.0f;
		matrix.m[3][2] = 0.0f;

		matrix.m[0][3] =-1.0f;
		matrix.m[1][3] = 1.0f;
		matrix.m[2][3] = 0.0f;
		matrix.m[3][3] = 1.0f;

		SDL_D3D11_setUniform_matrix( shader_data->vport, &matrix );
	}
	shader_data->color = SDL_createUniform(shader,"color");
	shader_data->color_mode = SDL_createUniform(shader,"color_mode");
	return shader;
}

int SDL_D3D11_bindShader( SDL_Shader* shader ) {
	/*SDL_D3D11_ShaderData *shader_data = (SDL_D3D11_ShaderData*) shader->driver_data;
	D3D11_RenderData *render_data = (D3D11_RenderData*) shader->renderer->driverdata;

	HRESULT result = ID3D11Device_SetPixelShader(render_data->d3dDevice, shader_data->pixl_shader);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D11 SetPixelShader() failed, errno %d\n", result);
	}

	result = ID3D11Device_SetVertexShader(render_data->d3dDevice, shader_data->vert_shader);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D11 SetVertexShader() failed, errno %d\n", result);
	}*/

	return 0;
}

int SDL_D3D11_unbindShader( SDL_Shader* shader ) {
	/*D3D11_RenderData *render_data = (D3D11_RenderData*) shader->renderer->driverdata;

	HRESULT result = ID3D11Device_SetPixelShader(render_data->d3dDevice, NULL);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D11 SetPixelShader() failed, errno %d\n", result);
	}

	result = ID3D11Device_SetVertexShader(render_data->d3dDevice, NULL);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D11 SetVertexShader() failed, errno %d\n", result);
	}*/

	return 0;
}

int SDL_D3D11_destroyShader( SDL_Shader* shader ) {
	
	shader->unbindShader( shader );
	free( shader->driver_data );
	free( shader );
	return 0;
}

int SDL_D3D11_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture_sdl,
		const SDL_Rect * srcrect, const SDL_Rect * dstrect_i) {

	SDL_Renderer* renderer = shader->renderer;
	//D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
	D3D11_TextureData *texturedata;
	//SDL_D3D11_ShaderData *shader_data = (SDL_D3D11_ShaderData*) shader->driver_data;

	float minx, miny, maxx, maxy;
	float minu, maxu, minv, maxv;
	Float4 color;
	VertexPositionColor vertices[4];

    D3D11_RenderStartDrawOp(renderer);
    D3D11_RenderSetBlendMode(renderer, texture_sdl->blendMode);

	/*if (D3D11_ActivateRenderer(renderer) < 0) {
		return -1;
	}*/

	texturedata = (D3D11_TextureData *)texture_sdl->driverdata;
	if (!texturedata) {
		SDL_SetError("SDL_Shader: D3D11 Texture is not currently available");
		return -1;
	}

	minx = dstrect.x - 0.5f;
	miny = dstrect.y - 0.5f;
	maxx = dstrect.x + dstrect.w - 0.5f;
	maxy = dstrect.y + dstrect.h - 0.5f;

	minu = (float) srcrect->x / texture_sdl->w;
	maxu = (float) (srcrect->x + srcrect->w) / texture_sdl->w;
	minv = (float) srcrect->y / texture_sdl->h;
	maxv = (float) (srcrect->y + srcrect->h) / texture_sdl->h;

    color.x = 1.0f;     /* red */
    color.y = 1.0f;     /* green */
    color.z = 1.0f;     /* blue */
    color.w = 1.0f;     /* alpha */
    if (texture_sdl->modMode & SDL_TEXTUREMODULATE_COLOR) {
        color.x = (float)(texture_sdl->r / 255.0f);     /* red */
        color.y = (float)(texture_sdl->g / 255.0f);     /* green */
        color.z = (float)(texture_sdl->b / 255.0f);     /* blue */
    }
    if (texture_sdl->modMode & SDL_TEXTUREMODULATE_ALPHA) {
        color.w = (float)(texture_sdl->a / 255.0f);     /* alpha */
    }

	vertices[0].pos.x = minx;
	vertices[0].pos.y = miny;
	vertices[0].pos.z = 0.0f;
	vertices[0].color = color;
	vertices[0].tex.x = minu;
	vertices[0].tex.y = minv;

	vertices[1].pos.x = maxx;
	vertices[1].pos.y = miny;
	vertices[1].pos.z = 0.0f;
	vertices[1].color = color;
	vertices[1].tex.x = maxu;
	vertices[1].tex.y = minv;

	vertices[2].pos.x = maxx;
	vertices[2].pos.y = maxy;
	vertices[2].pos.z = 0.0f;
	vertices[2].color = color;
	vertices[2].tex.x = maxu;
	vertices[2].tex.y = maxv;

	vertices[3].pos.x = minx;
	vertices[3].pos.y = maxy;
	vertices[3].pos.z = 0.0f;
	vertices[3].color = color;
	vertices[3].tex.x = minu;
	vertices[3].tex.y = maxv;

    if (D3D11_UpdateVertexBuffer(renderer, vertices, sizeof(vertices)) != 0) {
        return -1;
    }
	
	/*int r = shader->bindShader(shader);
	if( r ) return r;*/

    D3D11_RenderFinishDrawOp(renderer, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, sizeof(vertices) / sizeof(VertexPositionColor));

	/*r = shader->unbindShader(shader);
	if( r ) return r;*/

	return 0;
}

SDL_Uniform* SDL_D3D11_createUniformFromHandle( SDL_Shader* shader, int XXX, int YYY) {
	// No validation checks, handle must be valid
	/*SDL_Uniform* uniform = (SDL_Uniform*) malloc( sizeof(SDL_Uniform) );
	
	uniform->setUniform_iv  = SDL_D3D11_setUniform_iv;
	uniform->setUniform_i   = SDL_D3D11_setUniform_i;
	uniform->setUniform_i2  = SDL_D3D11_setUniform_i2;
	uniform->setUniform_i3  = SDL_D3D11_setUniform_i3;
	uniform->setUniform_i4  = SDL_D3D11_setUniform_i4;

	uniform->setUniform_fv  = SDL_D3D11_setUniform_fv;
	uniform->setUniform_f   = SDL_D3D11_setUniform_f;
	uniform->setUniform_f2  = SDL_D3D11_setUniform_f2;
	uniform->setUniform_f3  = SDL_D3D11_setUniform_f3;
	uniform->setUniform_f4  = SDL_D3D11_setUniform_f4;

	SDL_D3D11_UniformData* udata = malloc( sizeof(SDL_D3D11_UniformData) );*/
	return NULL;
}

SDL_Uniform* SDL_D3D11_createUniform( SDL_Shader* shader, const char* name ) {
	/*D3D11XHANDLE handle;
	SDL_D3D11_ShaderData *shader_data = (SDL_D3D11_ShaderData*) shader->driver_data;

	shader->bindShader( shader );
	handle = ID3D11XConstantTable_GetConstantByName( shader_data->vert_symtable, NULL, name );
	if( handle == NULL ){
		handle = ID3D11XConstantTable_GetConstantByName( shader_data->pixl_symtable, NULL, name );
		if( handle == NULL ){
			SDL_SetError("SDL_Shader: D3D11 GetParameterByName() failed\n" );
			return NULL;
		}else{
			return SDL_D3D11_createUniformFromHandle( shader, handle, shader_data->pixl_symtable );
		}
	}
	return SDL_D3D11_createUniformFromHandle( shader, handle, shader_data->vert_symtable );*/
	return NULL;
}

int SDL_D3D11_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform ) {
	/*(void) shader;
	free(uniform->driver_data);
	free(uniform);*/
	return 0;
}

int SDL_D3D11_setUniform_fv( SDL_Uniform* uniform, float* vector, int num ) {
	/*HRESULT result;
	SDL_Renderer* renderer = uniform->shader->renderer;
	D3D11_RenderData *render_data = (D3D11_RenderData*) renderer->driverdata;
	uniform->shader->bindShader(uniform->shader);
	SDL_D3D11_UniformData* udata = (SDL_D3D11_UniformData*) uniform->driver_data;
	
	result = ID3D11XConstantTable_SetFloatArray( udata->table, render_data->d3dDevice, udata->handle, vector, num );

	if (FAILED(result)){
		return SDL_SetError("SDL_Shader: D3D11 SetFloatArray() failed, errno %d\n", result);
	}*/
	return 0;
}
int SDL_D3D11_setUniform_f ( SDL_Uniform* uniform, float a ){
	/*float tmp[1] = {a};
	return SDL_D3D11_setUniform_fv( uniform, tmp, 1);*/
	return 0;
}
int SDL_D3D11_setUniform_f2( SDL_Uniform* uniform, float a, float b ){
	/*float tmp[2] = {a,b};
	return SDL_D3D11_setUniform_fv( uniform, tmp, 2);*/
	return 0;
}
int SDL_D3D11_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c ){
	/*float tmp[3] = {a,b,c};
	return SDL_D3D11_setUniform_fv( uniform, tmp, 3);*/
	return 0;
}
int SDL_D3D11_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d ){
	/*float tmp[4] = {a,b,c,d};
	return SDL_D3D11_setUniform_fv( uniform, tmp, 4);*/
	return 0;
}

int SDL_D3D11_setUniform_iv( SDL_Uniform* uniform, int*   vector, int num ) {
	/*HRESULT result;
	SDL_Renderer* renderer = uniform->shader->renderer;
	D3D11_RenderData *render_data = (D3D11_RenderData*) renderer->driverdata;
	uniform->shader->bindShader(uniform->shader);
	SDL_D3D11_UniformData* udata = (SDL_D3D11_UniformData*) uniform->driver_data;
	
	result = ID3D11XConstantTable_SetIntArray( udata->table, render_data->d3dDevice, udata->handle, vector, num );

	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D11 SetIntArray() failed, errno %d\n", result);
	}*/
	return 0;
}
int SDL_D3D11_setUniform_i ( SDL_Uniform* uniform, int a ){
	/*int tmp[1] = {a};
	return SDL_D3D11_setUniform_iv( uniform, tmp, 1);*/
	return 0;
}
int SDL_D3D11_setUniform_i2( SDL_Uniform* uniform, int a, int b ){
	/*int tmp[2] = {a,b};
	return SDL_D3D11_setUniform_iv( uniform, tmp, 2);*/
	return 0;
}
int SDL_D3D11_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c ){
	/*int tmp[3] = {a,b,c};
	return SDL_D3D11_setUniform_iv( uniform, tmp, 3);*/
	return 0;
}
int SDL_D3D11_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d ){
	/*int tmp[4] = {a,b,c,d};
	return SDL_D3D11_setUniform_iv( uniform, tmp, 4);*/
	return 0;
}

static int SDL_D3D11_setUniform_matrix( SDL_Uniform* uniform, Float4X4* matrix ){
	/*HRESULT result;
	SDL_Renderer* renderer = uniform->shader->renderer;
	D3D11_RenderData *render_data = (D3D11_RenderData*) renderer->driverdata;
	uniform->shader->bindShader(uniform->shader);
	SDL_D3D11_UniformData* udata = (SDL_D3D11_UniformData*) uniform->driver_data;
	
	result = ID3D11XConstantTable_SetMatrix( udata->table, render_data->d3dDevice, udata->handle, matrix );
	if( FAILED(result) ){
		return SDL_SetError("SDL_Shader: D3D11 SetMatrix() failed, errno %d\n", result);
	}*/
	return 0;
}

#else
extern void __suppress_a_warning__();
#endif /* SDL_SHADER_D3D11 */
