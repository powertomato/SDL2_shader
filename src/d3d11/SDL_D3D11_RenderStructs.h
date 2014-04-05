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

#ifndef _SDL_D3D11_RenderStructs_h
#define _SDL_D3D11_RenderStructs_h

#include "../SDL_SYS_RenderStructs.h"

typedef struct
{
    float x;
    float y;
} Float2;

typedef struct
{
    float x;
    float y;
    float z;
} Float3;

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} Float4;

typedef struct
{
	float m[4][4];
} Float4X4;

/* Vertex shader, common values */
typedef struct
{
    Float4X4 model;
    Float4X4 projectionAndView;
} VertexShaderConstants;

/* Per-vertex data */
typedef struct
{
    Float3 pos;
    Float2 tex;
    Float4 color;
} VertexPositionColor;

/* Per-texture data */
typedef struct
{
    ID3D11Texture2D *mainTexture;
    ID3D11ShaderResourceView *mainTextureResourceView;
    ID3D11RenderTargetView *mainTextureRenderTargetView;
    ID3D11Texture2D *stagingTexture;
    int lockedTexturePositionX;
    int lockedTexturePositionY;
    D3D11_FILTER scaleMode;

    /* YV12 texture support */
    SDL_bool yuv;
    ID3D11Texture2D *mainTextureU;
    ID3D11ShaderResourceView *mainTextureResourceViewU;
    ID3D11Texture2D *mainTextureV;
    ID3D11ShaderResourceView *mainTextureResourceViewV;
    Uint8 *pixels;
    int pitch;
    SDL_Rect locked_rect;
} D3D11_TextureData;

#ifdef __WINRT__
typedef ID3D11Device1 ID3D11DeviceN;
typedef IDXGIFactory2 IDXGIFactoryN;
typedef ID3D11DeviceContext1 ID3D11DeviceContextN;
typedef IDXGISwapChain1 IDXGISwapChainN;
#else
typedef ID3D11Device ID3D11DeviceN;
typedef IDXGIFactory1 IDXGIFactoryN;
typedef ID3D11DeviceContext ID3D11DeviceContextN;
typedef IDXGISwapChain IDXGISwapChainN;
#endif
/* Private renderer data */
typedef struct
{
    void *hDXGIMod;
    void *hD3D11Mod;
    IDXGIFactoryN *dxgiFactory;
    IDXGIAdapter *dxgiAdapter;
    ID3D11DeviceN *d3dDevice;
    ID3D11DeviceContextN *d3dContext;
    IDXGISwapChainN *swapChain;
    DXGI_SWAP_EFFECT swapEffect;
    ID3D11RenderTargetView *mainRenderTargetView;
    ID3D11RenderTargetView *currentOffscreenRenderTargetView;
    ID3D11InputLayout *inputLayout;
    ID3D11Buffer *vertexBuffer;
    ID3D11VertexShader *vertexShader;
    ID3D11PixelShader *colorPixelShader;
    ID3D11PixelShader *texturePixelShader;
    ID3D11PixelShader *yuvPixelShader;
    ID3D11BlendState *blendModeBlend;
    ID3D11BlendState *blendModeAdd;
    ID3D11BlendState *blendModeMod;
    ID3D11SamplerState *nearestPixelSampler;
    ID3D11SamplerState *linearSampler;
    D3D_FEATURE_LEVEL featureLevel;

    /* Rasterizers */
    ID3D11RasterizerState *mainRasterizer;
    ID3D11RasterizerState *clippedRasterizer;

    /* Vertex buffer constants */
    VertexShaderConstants vertexShaderConstantsData;
    ID3D11Buffer *vertexShaderConstants;

    /* Cached renderer properties */
    DXGI_MODE_ROTATION rotation;
    ID3D11RenderTargetView *currentRenderTargetView;
    ID3D11RasterizerState *currentRasterizerState;
    ID3D11BlendState *currentBlendState;
    ID3D11PixelShader *currentShader;
    ID3D11ShaderResourceView *currentShaderResource;
    ID3D11SamplerState *currentSampler;
} D3D11_RenderData;

#endif /* _SDL_D3D11_RenderStructs_h */

