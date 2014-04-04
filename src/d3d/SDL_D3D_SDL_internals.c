/*
  SDL2 shader
  Copyright (C) 2014 Stefan Krulj (powertomato) <powertomato (-at-) gmail.com>

  Thes code is internally defined by SDL2 and copied from the SDL2-2.0.3
  source release. Minor changes or additions (if any) are provided by the
  license below.
*/

/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

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

