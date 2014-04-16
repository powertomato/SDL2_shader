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

#ifdef SDL_SHADER_D3D

#include <stdio.h>
#include <SDL2/SDL.h>

#define D3D_DEBUG_INFO
#define COBJMACROS
#include <d3d9.h>
#include <d3dx9effect.h>
#include <unknwn.h>
#undef D3D_DEBUG_INFO
#undef COBJMACROS

#include "../SDL_shader.h"
#include "SDL_D3D_shader.h"

#include "SDL_D3D_RenderStructs.h"
#include "SDL_D3D_SDL_internals.c"

#define SAFE_RELEASE(X) if ((X)) { IUnknown_Release(SDL_static_cast(IUnknown*, X)); X = NULL; }
typedef struct {
	LPDIRECT3DPIXELSHADER9 pixl_shader;
	LPDIRECT3DVERTEXSHADER9 vert_shader;

	LPD3DXCONSTANTTABLE pixl_symtable;
	LPD3DXCONSTANTTABLE vert_symtable;

	SDL_Uniform* color;
	SDL_Uniform* vport;
	SDL_Uniform* color_mode;
} SDL_D3D_ShaderData;

typedef struct {
	LPD3DXCONSTANTTABLE table;
	D3DXHANDLE handle;
} SDL_D3D_UniformData;

typedef Vertex SDL_D3D_Vertex_t;

void SDL_D3D_getVertex( SDL_Vertex* vertices, unsigned num,
	uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a,
	float *x, float *y, float *z,
	float *tex_s, float *tex_t )
{
	SDL_D3D_Vertex_t* vbuff = (SDL_D3D_Vertex_t*) vertices->vertexBuffer;
	DWORD color = vbuff[num].color;
	if( a ) (*a) = (uint8_t) ((color & 0xFF000000)>>24);
	if( r ) (*r) = (uint8_t) ((color & 0x00FF0000)>>16);
	if( g ) (*g) = (uint8_t) ((color & 0x0000FF00)>> 8);
	if( b ) (*b) = (uint8_t) (color & 0x000000FF);

	if( x ) (*x) = (vbuff[num].x );
	if( y ) (*y) = (vbuff[num].y );
	if( z ) (*z) = (vbuff[num].z );

	if( tex_s ) (*tex_s) = (vbuff[num].u );
	if( tex_t ) (*tex_t) = (vbuff[num].v );
}

void SDL_D3D_setVertexColor(SDL_Vertex* vertices, unsigned from, unsigned num,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	SDL_D3D_Vertex_t* vbuff = (SDL_D3D_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].color = D3DCOLOR_ARGB( a,r,g,b );
	}
}
void SDL_D3D_setVertexPosition(SDL_Vertex* vertices, unsigned from,
	unsigned num, float x, float y, float z )
{
	SDL_D3D_Vertex_t* vbuff = (SDL_D3D_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].x = x;
		vbuff[i].y = y;
		vbuff[i].z = z;
	}
}
void SDL_D3D_setVertexTexCoord(SDL_Vertex* vertices, unsigned from,
	unsigned num, float s, float t )
{
	SDL_D3D_Vertex_t* vbuff = (SDL_D3D_Vertex_t*) vertices->vertexBuffer;
	unsigned i;
	for ( i=from; i<from+num; i++ ) {
		vbuff[i].u = s;
		vbuff[i].v = t;
	}
}
SDL_Vertex* SDL_D3D_createVertexBuffer( unsigned size ) {
	SDL_Vertex* ret = (SDL_Vertex*) malloc( sizeof(SDL_Vertex) );
	if ( !ret ) {
		SDL_OutOfMemory();
		return NULL;
	}
	ret->vertexBuffer = malloc( sizeof(SDL_D3D_Vertex_t)*size );
	if ( !ret->vertexBuffer ) {
		free( ret );
		SDL_OutOfMemory();
		return NULL;
	}
	ret->size = size;
	ret->getVertex = SDL_D3D_getVertex;
	ret->setVertexColor = SDL_D3D_setVertexColor;
	ret->setVertexPosition = SDL_D3D_setVertexPosition;
	ret->setVertexTexCoord = SDL_D3D_setVertexTexCoord;
	return ret;
}
int SDL_D3D_destroyVertexBuffer( SDL_Vertex* buff ) {
	free( buff->vertexBuffer );
	free( buff );
	return 0;
}

static int vs_version_major = 1;
static int vs_version_minor = 1;
static int ps_version_major = 2;
static int ps_version_minor = 0;

void SDL_D3D_hint(sdl_shader_hint flag, void* value) {
	int tmp;
	switch(flag) {
		case SDL_D3D_VS_MAJOR_VERSION:
			tmp = *((int*)value);
			vs_version_major = tmp >= 0 && tmp <= 9 ? tmp : vs_version_major;
			break;
		case SDL_D3D_VS_MINOR_VERSION:
			tmp = *((int*)value);
			vs_version_minor = tmp >= 0 && tmp <= 9 ? tmp : vs_version_minor;
			break;
		case SDL_D3D_PS_MAJOR_VERSION:
			tmp = *((int*)value);
			ps_version_major = tmp >= 0 && tmp <= 9 ? tmp : ps_version_major;
			break;
		case SDL_D3D_PS_MINOR_VERSION:
			tmp = *((int*)value);
			ps_version_minor = tmp >= 0 && tmp <= 9 ? tmp : ps_version_minor;
			break;
		default: /* nothing */ break;
	}

}

static int SDL_D3D_setUniform_matrix( SDL_Uniform* uniform, D3DXMATRIX* matix );
static void SDL_D3D_updateViewport( SDL_Shader* shader ) {
	SDL_Renderer* renderer = shader->renderer;
	SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;
	int w,h;
	SDL_RenderGetLogicalSize( renderer, &w, &h );
    if ( w && h && shader_data->vport )
	{
	    D3DXMATRIX matrix;
		matrix.m[0][0] = 2.0f / w;
		matrix.m[1][0] = 0.0f;
		matrix.m[2][0] = 0.0f;
		matrix.m[3][0] = 0.0f;

		matrix.m[0][1] = 0.0f;
		matrix.m[1][1] =-2.0f / h;
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

		SDL_D3D_setUniform_matrix( shader_data->vport, &matrix );
	}
}

extern char* SDL_Shader_readRW( SDL_RWops* rwop );
SDL_Shader* SDL_D3D_createShader( SDL_Renderer* renderer,
	SDL_ShaderStream* shdstream )
{

	SDL_Shader *shader;

	shader = (SDL_Shader*) malloc( sizeof(SDL_Shader) );
	if ( !shader ) {
		SDL_OutOfMemory();
		return NULL;
	}
	shader->driver_data = malloc( sizeof(SDL_D3D_ShaderData ) );
	if ( !shader->driver_data ) {
		free( shader );
		SDL_OutOfMemory();
		return NULL;
	}
	SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;
	D3D_RenderData *render_data = (D3D_RenderData*) renderer->driverdata;

	shader->renderer = renderer;
	shader->create_shader = SDL_D3D_createShader;
	shader->destroyShader =  SDL_D3D_destroyShader;
	shader->bindShader =     SDL_D3D_bindShader;
	shader->unbindShader =   SDL_D3D_unbindShader;
	shader->renderCopyShd =  SDL_D3D_renderCopyShd;
	shader->updateViewport = SDL_D3D_updateViewport;

	shader->createUniform  = SDL_D3D_createUniform;
	shader->destroyUniform = SDL_D3D_destroyUniform;

	shader->createVertexBuffer = SDL_D3D_createVertexBuffer;
	shader->destroyVertexBuffer = SDL_D3D_destroyVertexBuffer;

	HRESULT result;

	LPD3DXBUFFER code,error;
	char shader_version[8] = "vs_x_x";

	char* buff = SDL_Shader_readRW( shdstream->vshader );
	if( !buff ){
		SDL_OutOfMemory();
		return NULL;
	}
	shader_version[3] = '0'+vs_version_major;
	shader_version[5] = '0'+vs_version_minor;
	result = D3DXCompileShader(buff, strlen(buff), NULL, NULL, "VertexShaderMain",
		shader_version, 0, &code, &error, &(shader_data->vert_symtable));
	if ( FAILED(result)){
		SDL_SetError("SDL_Shader: D3D CompilelShader() failed: \n--\n%s--\n", ID3DXBuffer_GetBufferPointer( error ));
		SAFE_RELEASE( code );
		SAFE_RELEASE( error );
		return NULL;
	}
	result = IDirect3DDevice9_CreateVertexShader(render_data->device,
		(DWORD*) ID3DXBuffer_GetBufferPointer( code ), &shader_data->vert_shader);
	SAFE_RELEASE( code );
	SAFE_RELEASE( error );
	if (FAILED(result)) {
		shader->destroyShader(shader);
		SDL_SetError("SDL_Shader: D3D CreateVertexShader() failed, errno %d\n", result);
		return NULL;
	}

	shader_version[0] = 'p';
	shader_version[3] = '0'+ps_version_major;
	shader_version[5] = '0'+ps_version_minor;
	result = D3DXCompileShader(buff, strlen(buff), NULL, NULL, "PixelShaderMain",
		shader_version, 0, &code, &error, &(shader_data->pixl_symtable));
	if ( FAILED(result)){
		shader->destroyShader(shader);
		SDL_SetError("SDL_Shader: D3D CompilelShader() failed: \n--\n%s--\n", ID3DXBuffer_GetBufferPointer( error ));
		SAFE_RELEASE( code );
		SAFE_RELEASE( error );
		return NULL;
	}
	result = IDirect3DDevice9_CreatePixelShader(render_data->device, 
		(DWORD*) ID3DXBuffer_GetBufferPointer( code ), &shader_data->pixl_shader);
	SAFE_RELEASE( code );
	SAFE_RELEASE( error );
	free( buff );
	if (FAILED(result)) {
		shader->destroyShader(shader);
		SDL_SetError("SDL_Shader: D3D CreatePixelShader() failed, errno %d\n", result);
		return NULL;
	}

	shader_data->vport = SDL_createUniform(shader, "world_view_projection");

	SDL_D3D_updateViewport(shader);

	shader_data->color = SDL_createUniform(shader,"color");
	shader_data->color_mode = SDL_createUniform(shader,"color_mode");
	return shader;
}

int SDL_D3D_bindShader( SDL_Shader* shader ) {
	SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;
	D3D_RenderData *render_data = (D3D_RenderData*) shader->renderer->driverdata;
	HRESULT result = IDirect3DDevice9_SetPixelShader(render_data->device, shader_data->pixl_shader);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D SetPixelShader() failed, errno %d\n", result);
	}

	result = IDirect3DDevice9_SetVertexShader(render_data->device, shader_data->vert_shader);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D SetVertexShader() failed, errno %d\n", result);
	}

	return 0;
}

int SDL_D3D_unbindShader( SDL_Shader* shader ) {
	D3D_RenderData *render_data = (D3D_RenderData*) shader->renderer->driverdata;

	HRESULT result = IDirect3DDevice9_SetPixelShader(render_data->device, NULL);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D SetPixelShader() failed, errno %d\n", result);
	}

	result = IDirect3DDevice9_SetVertexShader(render_data->device, NULL);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D SetVertexShader() failed, errno %d\n", result);
	}

	return 0;
}

int SDL_D3D_destroyShader( SDL_Shader* shader ) {
	SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;
	SAFE_RELEASE( shader_data->vert_shader );
	SAFE_RELEASE( shader_data->pixl_shader );
	SAFE_RELEASE( shader_data->vert_symtable );
	SAFE_RELEASE( shader_data->pixl_symtable );
	shader->unbindShader( shader );
	free( shader->driver_data );
	free( shader );
	return 0;
}

int SDL_D3D_renderCopyShd(SDL_Shader* shader, SDL_Texture** textures,
		unsigned num_of_tex, SDL_Vertex* vertices, unsigned num_of_vert)
{

	SDL_Renderer* renderer;
	D3D_RenderData *data;
	D3D_TextureData *texturedata;
	int i;
	HRESULT result;


	renderer = shader->renderer;
	data = (D3D_RenderData *) renderer->driverdata;

	if (D3D_ActivateRenderer(renderer) < 0) {
		return -1;
	}

	for ( i=0; i<num_of_tex; i++ ) {
		texturedata = textures[i]->driverdata;
		if (!texturedata) {
			return SDL_SetError("SDL_Shader: D3D Texture is not currently available");
		}
		if (texturedata->yuv) {
			return SDL_SetError("SDL_Shader: D3D YUV-textures not supported\n");
		}

		result = IDirect3DDevice9_SetTexture(data->device, i,
			(IDirect3DBaseTexture9 *) texturedata->texture);
		if (FAILED(result)) {
			return SDL_SetError("SDL_Shader: D3D SetTexture() failed, errno %d\n", result);
		}
	}

	D3D_SetBlendMode(data, textures[0]->blendMode);
	D3D_UpdateTextureScaleMode(data, texturedata, 0);

	int r = shader->bindShader(shader);
	if ( r ) return r;

	SDL_D3D_Vertex_t *vbuff = (SDL_D3D_Vertex_t*) vertices->vertexBuffer;
	result = IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_TRIANGLESTRIP,
		num_of_vert-2, vbuff, sizeof(Vertex));
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D DrawPrimitiveUP() "
			"failed, errno %d\n", result);
	}
	r = shader->unbindShader(shader);
	if ( r ) return r;

	return 0;
}

SDL_Uniform* SDL_D3D_createUniformFromHandle( SDL_Shader* shader, D3DXHANDLE handle, LPD3DXCONSTANTTABLE table ) {
	// No validation checks, handle must be valid
	SDL_Uniform* uniform = (SDL_Uniform*) malloc( sizeof(SDL_Uniform) );
	if ( !uniform ) {
		SDL_OutOfMemory();
		return NULL;
	}
	
	uniform->setUniform_iv  = SDL_D3D_setUniform_iv;
	uniform->setUniform_i   = SDL_D3D_setUniform_i;
	uniform->setUniform_i2  = SDL_D3D_setUniform_i2;
	uniform->setUniform_i3  = SDL_D3D_setUniform_i3;
	uniform->setUniform_i4  = SDL_D3D_setUniform_i4;

	uniform->setUniform_fv  = SDL_D3D_setUniform_fv;
	uniform->setUniform_f   = SDL_D3D_setUniform_f;
	uniform->setUniform_f2  = SDL_D3D_setUniform_f2;
	uniform->setUniform_f3  = SDL_D3D_setUniform_f3;
	uniform->setUniform_f4  = SDL_D3D_setUniform_f4;

	SDL_D3D_UniformData* udata = malloc( sizeof(SDL_D3D_UniformData) );
	if ( !udata ) {
		free( uniform );
		SDL_OutOfMemory();
		return NULL;
	}
	udata->handle = handle;
	udata->table = table;
	uniform->driver_data =  udata;
	uniform->shader = shader;
	return uniform;
}

SDL_Uniform* SDL_D3D_createUniform( SDL_Shader* shader, const char* name ) {
	D3DXHANDLE handle;
	SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;

	shader->bindShader( shader );
	handle = ID3DXConstantTable_GetConstantByName( shader_data->vert_symtable, NULL, name );
	if ( handle == NULL ){
		handle = ID3DXConstantTable_GetConstantByName( shader_data->pixl_symtable, NULL, name );
		if ( handle == NULL ){
			SDL_SetError("SDL_Shader: D3D GetParameterByName() failed\n" );
			return NULL;
		} else {
			return SDL_D3D_createUniformFromHandle( shader, handle, shader_data->pixl_symtable );
		}
	}
	return SDL_D3D_createUniformFromHandle( shader, handle, shader_data->vert_symtable );
}

int SDL_D3D_destroyUniform( SDL_Shader* shader, SDL_Uniform* uniform ) {
	(void) shader;
	free(uniform->driver_data);
	free(uniform);
	return 0;
}

int SDL_D3D_setUniform_fv( SDL_Uniform* uniform, float* vector, int num ) {
	HRESULT result;
	SDL_Renderer* renderer = uniform->shader->renderer;
	D3D_RenderData *render_data = (D3D_RenderData*) renderer->driverdata;
	uniform->shader->bindShader(uniform->shader);
	SDL_D3D_UniformData* udata = (SDL_D3D_UniformData*) uniform->driver_data;
	
	result = ID3DXConstantTable_SetFloatArray( udata->table, render_data->device, udata->handle, vector, num );

	if (FAILED(result)){
		return SDL_SetError("SDL_Shader: D3D SetFloatArray() failed, errno %d\n", result);
	}
	return 0;
}
int SDL_D3D_setUniform_f ( SDL_Uniform* uniform, float a ){
	float tmp[1] = {a};
	return SDL_D3D_setUniform_fv( uniform, tmp, 1);
}
int SDL_D3D_setUniform_f2( SDL_Uniform* uniform, float a, float b ){
	float tmp[2] = {a,b};
	return SDL_D3D_setUniform_fv( uniform, tmp, 2);

}
int SDL_D3D_setUniform_f3( SDL_Uniform* uniform, float a, float b, float c ){
	float tmp[3] = {a,b,c};
	return SDL_D3D_setUniform_fv( uniform, tmp, 3);

}
int SDL_D3D_setUniform_f4( SDL_Uniform* uniform, float a, float b, float c, float d ){
	float tmp[4] = {a,b,c,d};
	return SDL_D3D_setUniform_fv( uniform, tmp, 4);
}

int SDL_D3D_setUniform_iv( SDL_Uniform* uniform, int*   vector, int num ) {
	HRESULT result;
	SDL_Renderer* renderer = uniform->shader->renderer;
	D3D_RenderData *render_data = (D3D_RenderData*) renderer->driverdata;
	uniform->shader->bindShader(uniform->shader);
	SDL_D3D_UniformData* udata = (SDL_D3D_UniformData*) uniform->driver_data;
	
	result = ID3DXConstantTable_SetIntArray( udata->table, render_data->device, udata->handle, vector, num );

	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D SetIntArray() failed, errno %d\n", result);
	}
	return 0;
}
int SDL_D3D_setUniform_i ( SDL_Uniform* uniform, int a ){
	int tmp[1] = {a};
	return SDL_D3D_setUniform_iv( uniform, tmp, 1);
}
int SDL_D3D_setUniform_i2( SDL_Uniform* uniform, int a, int b ){
	int tmp[2] = {a,b};
	return SDL_D3D_setUniform_iv( uniform, tmp, 2);
}
int SDL_D3D_setUniform_i3( SDL_Uniform* uniform, int a, int b, int c ){
	int tmp[3] = {a,b,c};
	return SDL_D3D_setUniform_iv( uniform, tmp, 3);
}
int SDL_D3D_setUniform_i4( SDL_Uniform* uniform, int a, int b, int c, int d ){
	int tmp[4] = {a,b,c,d};
	return SDL_D3D_setUniform_iv( uniform, tmp, 4);
}

static int SDL_D3D_setUniform_matrix( SDL_Uniform* uniform, D3DXMATRIX* matrix ){
	HRESULT result;
	SDL_Renderer* renderer = uniform->shader->renderer;
	D3D_RenderData *render_data = (D3D_RenderData*) renderer->driverdata;
	uniform->shader->bindShader(uniform->shader);
	SDL_D3D_UniformData* udata = (SDL_D3D_UniformData*) uniform->driver_data;
	
	result = ID3DXConstantTable_SetMatrix( udata->table, render_data->device, udata->handle, matrix );
	if( FAILED(result) ){
		return SDL_SetError("SDL_Shader: D3D SetMatrix() failed, errno %d\n", result);
	}
	return 0;
}

#else
extern void __suppress_a_warning__();
#endif /* SDL_SHADER_D3D */
