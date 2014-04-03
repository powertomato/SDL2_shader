
#ifndef _SDL_D3D_RenderStructs_h
#define _SDL_D3D_RenderStructs_h

#include <SDL2/SDL_opengl.h>

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

