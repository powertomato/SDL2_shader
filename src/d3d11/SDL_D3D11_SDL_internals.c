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

static ID3D11RenderTargetView *
D3D11_GetCurrentRenderTargetView(SDL_Renderer * renderer)
{
    D3D11_RenderData *data = (D3D11_RenderData *) renderer->driverdata;
    if (data->currentOffscreenRenderTargetView) {
        return data->currentOffscreenRenderTargetView;
    } else {
        return data->mainRenderTargetView;
    }
}

static void
D3D11_RenderSetBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *)renderer->driverdata;
    ID3D11BlendState *blendState = NULL;
    switch (blendMode) {
    case SDL_BLENDMODE_BLEND:
        blendState = rendererData->blendModeBlend;
        break;
    case SDL_BLENDMODE_ADD:
        blendState = rendererData->blendModeAdd;
        break;
    case SDL_BLENDMODE_MOD:
        blendState = rendererData->blendModeMod;
        break;
    case SDL_BLENDMODE_NONE:
        blendState = NULL;
        break;
    }
    if (blendState != rendererData->currentBlendState) {
        ID3D11DeviceContext_OMSetBlendState(rendererData->d3dContext, blendState, 0, 0xFFFFFFFF);
        rendererData->currentBlendState = blendState;
    }
}

static void
D3D11_RenderStartDrawOp(SDL_Renderer * renderer)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *)renderer->driverdata;
    ID3D11RasterizerState *rasterizerState;
    ID3D11RenderTargetView *renderTargetView = D3D11_GetCurrentRenderTargetView(renderer);
    if (renderTargetView != rendererData->currentRenderTargetView) {
        ID3D11DeviceContext_OMSetRenderTargets(rendererData->d3dContext,
            1,
            &renderTargetView,
            NULL
            );
        rendererData->currentRenderTargetView = renderTargetView;
    }

    if (SDL_RectEmpty(&renderer->clip_rect)) {
        rasterizerState = rendererData->mainRasterizer;
    } else {
        rasterizerState = rendererData->clippedRasterizer;
    }
    if (rasterizerState != rendererData->currentRasterizerState) {
        ID3D11DeviceContext_RSSetState(rendererData->d3dContext, rasterizerState);
        rendererData->currentRasterizerState = rasterizerState;
    }
}

static void
D3D11_RenderFinishDrawOp(SDL_Renderer * renderer,
                         D3D11_PRIMITIVE_TOPOLOGY primitiveTopology,
                         UINT vertexCount)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;

    ID3D11DeviceContext_IASetPrimitiveTopology(rendererData->d3dContext, primitiveTopology);
    ID3D11DeviceContext_Draw(rendererData->d3dContext, vertexCount, 0);
}


static int
D3D11_UpdateVertexBuffer(SDL_Renderer *renderer,
                         const void * vertexData, size_t dataSizeInBytes)
{
    D3D11_RenderData *rendererData = (D3D11_RenderData *) renderer->driverdata;
    D3D11_BUFFER_DESC vertexBufferDesc;
    HRESULT result = S_OK;
    D3D11_SUBRESOURCE_DATA vertexBufferData;
    const UINT stride = sizeof(VertexPositionColor);
    const UINT offset = 0;

    if (rendererData->vertexBuffer) {
        ID3D11Buffer_GetDesc(rendererData->vertexBuffer, &vertexBufferDesc);
    } else {
        SDL_zero(vertexBufferDesc);
    }

    if (rendererData->vertexBuffer && vertexBufferDesc.ByteWidth >= dataSizeInBytes) {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        result = ID3D11DeviceContext_Map(rendererData->d3dContext,
            (ID3D11Resource *)rendererData->vertexBuffer,
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mappedResource
            );
        if (FAILED(result)) {
            //WIN_SetErrorFromHRESULT(__FUNCTION__ ", ID3D11DeviceContext1::Map [vertex buffer]", result);
            return -1;
        }
        SDL_memcpy(mappedResource.pData, vertexData, dataSizeInBytes);
        ID3D11DeviceContext_Unmap(rendererData->d3dContext, (ID3D11Resource *)rendererData->vertexBuffer, 0);
    } else {
        SAFE_RELEASE(rendererData->vertexBuffer);

        vertexBufferDesc.ByteWidth = dataSizeInBytes;
        vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        SDL_zero(vertexBufferData);
        vertexBufferData.pSysMem = vertexData;
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        result = ID3D11Device_CreateBuffer(rendererData->d3dDevice,
            &vertexBufferDesc,
            &vertexBufferData,
            &rendererData->vertexBuffer
            );
        if (FAILED(result)) {
            //WIN_SetErrorFromHRESULT(__FUNCTION__ ", ID3D11Device1::CreateBuffer [vertex buffer]", result);
            return -1;
        }

        ID3D11DeviceContext_IASetVertexBuffers(rendererData->d3dContext,
            0,
            1,
            &rendererData->vertexBuffer,
            &stride,
            &offset
            );
    }

    return 0;
}
