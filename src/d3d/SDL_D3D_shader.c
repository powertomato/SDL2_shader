#ifdef SDL_SHADER_D3D

#include <stdio.h>
#include <SDL2/SDL.h>

#define D3D_DEBUG_INFO
#include <d3d9.h>
#include <d3dx9effect.h>

#include "../SDL_shader.h"
#include "SDL_D3D_shader.h"
#include "SDL_D3D_RenderStructs.h"

#define SAVE_RELEASE(X) if( X != NULL ){ ID3DXBuffer_Release( X ); X=NULL; }
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

extern char* textFileRead(const char* path);
static int SDL_D3D_setUniform_matrix( SDL_Uniform* uniform, D3DXMATRIX* matix );
SDL_Shader* SDL_D3D_createShader( SDL_Renderer* renderer, const char *name ) {

	SDL_Shader *shader;

	shader = (SDL_Shader*) malloc( sizeof(SDL_Shader) );
	shader->driver_data = malloc( sizeof(SDL_D3D_ShaderData ) );
	SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;
	D3D_RenderData *render_data = (D3D_RenderData*) renderer->driverdata;

	shader->renderer = renderer;
	shader->create_shader = SDL_D3D_createShader;
	shader->destroyShader =  SDL_D3D_destroyShader;
	shader->bindShader =     SDL_D3D_bindShader;
	shader->unbindShader =   SDL_D3D_unbindShader;
	shader->renderCopyShd =  SDL_D3D_renderCopyShd;

	shader->createUniform  = SDL_D3D_createUniform;
	shader->destroyUniform = SDL_D3D_destroyUniform;

	HRESULT result;
	int name_len = strlen( name );
	int ext_len =  9; //strlen(".d3d.hlsl");

	char *file_name = malloc(name_len + ext_len + 1 );
	strncpy( file_name, name, name_len );
	file_name[ name_len + ext_len ] = '\0';

	LPD3DXBUFFER code,error;
	char shader_version[8] = "vs_x_x";
	strncpy( file_name + name_len, ".d3d.hlsl", ext_len );

	shader_version[3] = '0'+vs_version_major;
	shader_version[5] = '0'+vs_version_minor;
	result = D3DXCompileShaderFromFile(file_name, NULL, NULL, "VertexShaderMain", 
		shader_version, 0, &code, &error, &(shader_data->vert_symtable));
	if( FAILED(result)){
		SDL_SetError("SDL_Shader: D3D CompilelShader() failed: \n--\n%s--\n", ID3DXBuffer_GetBufferPointer( error ));
		SAVE_RELEASE( code );
		SAVE_RELEASE( error );
		return NULL;
	}
	result = IDirect3DDevice9_CreateVertexShader(render_data->device, 
		(DWORD*) ID3DXBuffer_GetBufferPointer( code ), &shader_data->vert_shader);
	SAVE_RELEASE( code );
	SAVE_RELEASE( error );
	if (FAILED(result)) {
		shader->destroyShader(shader);
		SDL_SetError("SDL_Shader: D3D CreatePixelShader() failed, errno %d\n", result);
		return NULL;
	}

	shader_version[0] = 'p';
	shader_version[3] = '0'+ps_version_major;
	shader_version[5] = '0'+ps_version_minor;
	result = D3DXCompileShaderFromFile(file_name, NULL, NULL, "PixelShaderMain",
		shader_version, 0, &code, NULL, &(shader_data->pixl_symtable));
	if( FAILED(result)){
		SDL_SetError("SDL_Shader: D3D CompilelShader() failed: \n--\n%s--\n", ID3DXBuffer_GetBufferPointer( error ));
		SAVE_RELEASE( code );
		SAVE_RELEASE( error );
		return NULL;
	}
	result = IDirect3DDevice9_CreatePixelShader(render_data->device, 
		(DWORD*) ID3DXBuffer_GetBufferPointer( code ), &shader_data->pixl_shader);
	SAVE_RELEASE( code );
	SAVE_RELEASE( error );
	if (FAILED(result)) {
		shader->destroyShader(shader);
		SDL_SetError("SDL_Shader: D3D CreatePixelShader() failed, errno %d\n", result);
		return NULL;
	}

	free(file_name);

	shader_data->vport = SDL_createUniform(shader, "world_view_projection");
	if( shader_data->vport ){
	    D3DXMATRIX matrix;
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

		SDL_D3D_setUniform_matrix( shader_data->vport, &matrix );
	}
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
	
	shader->unbindShader( shader );
	free( shader->driver_data );
	free( shader );
	return 0;
}

static D3DFORMAT PixelFormatToD3DFMT(Uint32 format) {
	switch (format) {
		case SDL_PIXELFORMAT_RGB565:
			return D3DFMT_R5G6B5;
		case SDL_PIXELFORMAT_RGB888:
			return D3DFMT_X8R8G8B8;
		case SDL_PIXELFORMAT_ARGB8888:
			return D3DFMT_A8R8G8B8;
		case SDL_PIXELFORMAT_YV12:
		case SDL_PIXELFORMAT_IYUV:
			return D3DFMT_L8;
		default:
			return D3DFMT_UNKNOWN;
	}
}

static int D3D_ActivateRenderer(SDL_Renderer * renderer)
{
	D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
	HRESULT result;

	if (data->updateSize) {
		SDL_Window *window = renderer->window;
		int w, h;

		SDL_GetWindowSize(window, &w, &h);
		data->pparams.BackBufferWidth = w;
		data->pparams.BackBufferHeight = h;
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
			data->pparams.BackBufferFormat =
				PixelFormatToD3DFMT(SDL_GetWindowPixelFormat(window));
		} else {
			data->pparams.BackBufferFormat = D3DFMT_UNKNOWN;
		}
		/* XXX if (D3D_Reset(renderer) < 0) {
		   return -1;
		   }*/

		data->updateSize = SDL_FALSE;
	}
	if (data->beginScene) {
		result = IDirect3DDevice9_BeginScene(data->device);
		if (result == D3DERR_DEVICELOST) {
			/* XXX if (D3D_Reset(renderer) < 0) {
			   return -1./include/GL/glext.h;
			   }*/
			result = IDirect3DDevice9_BeginScene(data->device);
		}
		if (FAILED(result)) {
			return SDL_SetError("SDL_Shader: D3D BeginScene() failed, errno %d\n", result);
		}
		data->beginScene = SDL_FALSE;
	}
	return 0;
}


static void D3D_UpdateTextureScaleMode(D3D_RenderData *data, D3D_TextureData *texturedata, unsigned index)
{
	if (texturedata->scaleMode != data->scaleMode[index]) {
		IDirect3DDevice9_SetSamplerState(data->device, index, D3DSAMP_MINFILTER,
				texturedata->scaleMode);
		IDirect3DDevice9_SetSamplerState(data->device, index, D3DSAMP_MAGFILTER,
				texturedata->scaleMode);
		data->scaleMode[index] = texturedata->scaleMode;
	}
}

	static void
D3D_SetBlendMode(D3D_RenderData * data, int blendMode)
{
	switch (blendMode) {
		case SDL_BLENDMODE_NONE:
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_ALPHABLENDENABLE,
					FALSE);
			break;
		case SDL_BLENDMODE_BLEND:
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_ALPHABLENDENABLE,
					TRUE);
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLEND,
					D3DBLEND_SRCALPHA);
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLEND,
					D3DBLEND_INVSRCALPHA);
			if (data->enableSeparateAlphaBlend) {
				IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLENDALPHA,
						D3DBLEND_ONE);
				IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLENDALPHA,
						D3DBLEND_INVSRCALPHA);
			}
			break;
		case SDL_BLENDMODE_ADD:
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_ALPHABLENDENABLE,
					TRUE);
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLEND,
					D3DBLEND_SRCALPHA);
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLEND,
					D3DBLEND_ONE);
			if (data->enableSeparateAlphaBlend) {
				IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLENDALPHA,
						D3DBLEND_ZERO);
				IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLENDALPHA,
						D3DBLEND_ONE);
			}
			break;
		case SDL_BLENDMODE_MOD:
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_ALPHABLENDENABLE,
					TRUE);
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLEND,
					D3DBLEND_ZERO);
			IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLEND,
					D3DBLEND_SRCCOLOR);
			if (data->enableSeparateAlphaBlend) {
				IDirect3DDevice9_SetRenderState(data->device, D3DRS_SRCBLENDALPHA,
						D3DBLEND_ZERO);
				IDirect3DDevice9_SetRenderState(data->device, D3DRS_DESTBLENDALPHA,
						D3DBLEND_ONE);
			}
			break;
	}
}

int SDL_D3D_renderCopyShd(SDL_Shader* shader, SDL_Texture* texture_sdl,
		const SDL_Rect * srcrect, const SDL_FRect * dstrect) {

	SDL_Renderer* renderer = shader->renderer;
	D3D_RenderData *data = (D3D_RenderData *) renderer->driverdata;
	D3D_TextureData *texturedata;
	//SDL_D3D_ShaderData *shader_data = (SDL_D3D_ShaderData*) shader->driver_data;

	float minx, miny, maxx, maxy;
	float minu, maxu, minv, maxv;
	DWORD color;
	Vertex vertices[4];
	HRESULT result;

	if (D3D_ActivateRenderer(renderer) < 0) {
		return -1;
	}

	texturedata = (D3D_TextureData *)texture_sdl->driverdata;
	if (!texturedata) {
		SDL_SetError("SDL_Shader: D3D Texture is not currently available");
		return -1;
	}

	minx = dstrect->x - 0.5f;
	miny = dstrect->y - 0.5f;
	maxx = dstrect->x + dstrect->w - 0.5f;
	maxy = dstrect->y + dstrect->h - 0.5f;

	minu = (float) srcrect->x / texture_sdl->w;
	maxu = (float) (srcrect->x + srcrect->w) / texture_sdl->w;
	minv = (float) srcrect->y / texture_sdl->h;
	maxv = (float) (srcrect->y + srcrect->h) / texture_sdl->h;

	color = D3DCOLOR_ARGB(texture_sdl->a, texture_sdl->r, texture_sdl->g, texture_sdl->b);

	vertices[0].x = minx;
	vertices[0].y = miny;
	vertices[0].z = 0.0f;
	vertices[0].color = color;
	vertices[0].u = minu;
	vertices[0].v = minv;

	vertices[1].x = maxx;
	vertices[1].y = miny;
	vertices[1].z = 0.0f;
	vertices[1].color = color;
	vertices[1].u = maxu;
	vertices[1].v = minv;

	vertices[2].x = maxx;
	vertices[2].y = maxy;
	vertices[2].z = 0.0f;
	vertices[2].color = color;
	vertices[2].u = maxu;
	vertices[2].v = maxv;

	vertices[3].x = minx;
	vertices[3].y = maxy;
	vertices[3].z = 0.0f;
	vertices[3].color = color;
	vertices[3].u = minu;
	vertices[3].v = maxv;

	D3D_SetBlendMode(data, texture_sdl->blendMode);

	D3D_UpdateTextureScaleMode(data, texturedata, 0);

	result =
		IDirect3DDevice9_SetTexture(data->device, 0, (IDirect3DBaseTexture9 *)
				texturedata->texture);
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D SetTexture() failed, errno %d\n", result);
	}

	if (texturedata->yuv) {
		return SDL_SetError("SDL_Shader: D3D YUV-textures not supported\n");
	}
	
	int r = shader->bindShader(shader);
	if( r ) return r;
	result = IDirect3DDevice9_DrawPrimitiveUP(data->device, D3DPT_TRIANGLEFAN, 2,
			vertices, sizeof(*vertices));
	if (FAILED(result)) {
		return SDL_SetError("SDL_Shader: D3D DrawPrimitiveUP() failed, errno %d\n", result);
	}
	r = shader->unbindShader(shader);
	if( r ) return r;

	return 0;
}

SDL_Uniform* SDL_D3D_createUniformFromHandle( SDL_Shader* shader, D3DXHANDLE handle, LPD3DXCONSTANTTABLE table ) {
	// No validation checks, handle must be valid
	SDL_Uniform* uniform = (SDL_Uniform*) malloc( sizeof(SDL_Uniform) );
	
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
	if( handle == NULL ){
		handle = ID3DXConstantTable_GetConstantByName( shader_data->pixl_symtable, NULL, name );
		if( handle == NULL ){
			SDL_SetError("SDL_Shader: D3D GetParameterByName() failed\n" );
			return NULL;
		}else{
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
