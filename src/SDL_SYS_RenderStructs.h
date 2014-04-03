
#ifndef _SDL_SYS_RenderStructs_h
#define _SDL_SYS_RenderStructs_h

typedef struct {
    float x;
    float y;
} SDL_FPoint;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} SDL_FRect;

typedef void SDL_SW_YUVTexture; /*TODO*/
struct SDL_Texture
{
    const void *magic;
    Uint32 format;              /**< The pixel format of the texture */
    int access;                 /**< SDL_TextureAccess */
    int w;                      /**< The width of the texture */
    int h;                      /**< The height of the texture */
    int modMode;                /**< The texture modulation mode */
    SDL_BlendMode blendMode;    /**< The texture blend mode */
    Uint8 r, g, b, a;           /**< Texture modulation values */

    SDL_Renderer *renderer;

    /* Support for formats not supported directly by the renderer */
    SDL_Texture *native;
    SDL_SW_YUVTexture *yuv;
    void *pixels;
    int pitch;
    SDL_Rect locked_rect;

    void *driverdata;           /**< Driver specific texture representation */

    SDL_Texture *prev;
    SDL_Texture *next;
};

typedef struct SDL_Renderer SDL_Renderer;
struct SDL_Renderer
{
    const void *magic;

    void (*WindowEvent) (SDL_Renderer * renderer, const SDL_WindowEvent *event);
    int (*GetOutputSize) (SDL_Renderer * renderer, int *w, int *h);
    int (*CreateTexture) (SDL_Renderer * renderer, SDL_Texture * texture);
    int (*SetTextureColorMod) (SDL_Renderer * renderer,
                               SDL_Texture * texture);
    int (*SetTextureAlphaMod) (SDL_Renderer * renderer,
                               SDL_Texture * texture);
    int (*SetTextureBlendMode) (SDL_Renderer * renderer,
                                SDL_Texture * texture);
    int (*UpdateTexture) (SDL_Renderer * renderer, SDL_Texture * texture,
                          const SDL_Rect * rect, const void *pixels,
                          int pitch);
    int (*UpdateTextureYUV) (SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * rect,
                            const Uint8 *Yplane, int Ypitch,
                            const Uint8 *Uplane, int Upitch,
                            const Uint8 *Vplane, int Vpitch);
    int (*LockTexture) (SDL_Renderer * renderer, SDL_Texture * texture,
                        const SDL_Rect * rect, void **pixels, int *pitch);
    void (*UnlockTexture) (SDL_Renderer * renderer, SDL_Texture * texture);
    int (*SetRenderTarget) (SDL_Renderer * renderer, SDL_Texture * texture);
    int (*UpdateViewport) (SDL_Renderer * renderer);
    int (*UpdateClipRect) (SDL_Renderer * renderer);
    int (*RenderClear) (SDL_Renderer * renderer);
    int (*RenderDrawPoints) (SDL_Renderer * renderer, const SDL_FPoint * points,
                             int count);
    int (*RenderDrawLines) (SDL_Renderer * renderer, const SDL_FPoint * points,
                            int count);
    int (*RenderFillRects) (SDL_Renderer * renderer, const SDL_FRect * rects,
                            int count);
    int (*RenderCopy) (SDL_Renderer * renderer, SDL_Texture * texture,
                       const SDL_Rect * srcrect, const SDL_FRect * dstrect);
    int (*RenderCopyEx) (SDL_Renderer * renderer, SDL_Texture * texture,
                       const SDL_Rect * srcquad, const SDL_FRect * dstrect,
                       const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);
    int (*RenderReadPixels) (SDL_Renderer * renderer, const SDL_Rect * rect,
                             Uint32 format, void * pixels, int pitch);
    void (*RenderPresent) (SDL_Renderer * renderer);
    void (*DestroyTexture) (SDL_Renderer * renderer, SDL_Texture * texture);

    void (*DestroyRenderer) (SDL_Renderer * renderer);

    int (*GL_BindTexture) (SDL_Renderer * renderer, SDL_Texture *texture, float *texw, float *texh);
    int (*GL_UnbindTexture) (SDL_Renderer * renderer, SDL_Texture *texture);

    /* The current renderer info */
    SDL_RendererInfo info;

    /* The window associated with the renderer */
    SDL_Window *window;
    SDL_bool hidden;

    /* The logical resolution for rendering */
    int logical_w;
    int logical_h;
    int logical_w_backup;
    int logical_h_backup;

    /* The drawable area within the window */
    SDL_Rect viewport;
    SDL_Rect viewport_backup;

    /* The clip rectangle within the window */
    SDL_Rect clip_rect;
    SDL_Rect clip_rect_backup;

    /* The render output coordinate scale */
    SDL_FPoint scale;
    SDL_FPoint scale_backup;

    /* The list of textures */
    SDL_Texture *textures;
    SDL_Texture *target;

    Uint8 r, g, b, a;                   /**< Color for drawing operations values */
    SDL_BlendMode blendMode;            /**< The drawing blend mode */

    void *driverdata;
};

#endif /* _SDL_SYS_RenderStructs_h */


