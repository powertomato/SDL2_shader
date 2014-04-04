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

#ifndef _SDL_D3D_RenderStructs_h
#define _SDL_D3D_RenderStructs_h

#include "../SDL_SYS_RenderStructs.h"

typedef struct
{
    void* d3dDLL;
    IDirect3D9 *d3d;
    IDirect3DDevice9 *device;
    UINT adapter;
    D3DPRESENT_PARAMETERS pparams;
    SDL_bool updateSize;
    SDL_bool beginScene;
    SDL_bool enableSeparateAlphaBlend;
    D3DTEXTUREFILTERTYPE scaleMode[8];
    IDirect3DSurface9 *defaultRenderTarget;
    IDirect3DSurface9 *currentRenderTarget;
    void* d3dxDLL;
    LPDIRECT3DPIXELSHADER9 ps_yuv;
} D3D_RenderData;

typedef struct
{
    IDirect3DTexture9 *texture;
    D3DTEXTUREFILTERTYPE scaleMode;

    /* YV12 texture support */
    SDL_bool yuv;
    IDirect3DTexture9 *utexture;
    IDirect3DTexture9 *vtexture;
    Uint8 *pixels;
    int pitch;
    SDL_Rect locked_rect;
} D3D_TextureData;

typedef struct
{
    float x, y, z;
    DWORD color;
    float u, v;
} Vertex;

#endif /* _SDL_D3D_RenderStructs_h */

