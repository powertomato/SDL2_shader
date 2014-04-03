
typedef struct GLES2_ShaderInstance
{
    GLenum type;
    GLenum format;
    int length;
    const void *data;
} GLES2_ShaderInstance;

typedef enum
{
    GLES2_ATTRIBUTE_POSITION = 0,
    GLES2_ATTRIBUTE_TEXCOORD = 1,
    GLES2_ATTRIBUTE_ANGLE = 2,
    GLES2_ATTRIBUTE_CENTER = 3,
} GLES2_Attribute;

typedef enum
{
    GLES2_SHADER_VERTEX_DEFAULT,
    GLES2_SHADER_FRAGMENT_SOLID_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_ABGR_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_ARGB_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_BGR_SRC,
    GLES2_SHADER_FRAGMENT_TEXTURE_RGB_SRC
} GLES2_ShaderType;

typedef struct GLES2_FBOList GLES2_FBOList;
struct GLES2_FBOList
{
   Uint32 w, h;
   GLuint FBO;
   GLES2_FBOList *next;
};

typedef struct GLES2_TextureData
{
    GLenum texture;
    GLenum texture_type;
    GLenum pixel_format;
    GLenum pixel_type;
    void *pixel_data;
    size_t pitch;
    GLES2_FBOList *fbo;
} GLES2_TextureData;

typedef struct GLES2_ShaderCacheEntry
{
    GLuint id;
    GLES2_ShaderType type;
    const GLES2_ShaderInstance *instance;
    int references;
    Uint8 modulation_r, modulation_g, modulation_b, modulation_a;
    struct GLES2_ShaderCacheEntry *prev;
    struct GLES2_ShaderCacheEntry *next;
} GLES2_ShaderCacheEntry;

typedef struct GLES2_ShaderCache
{
    int count;
    GLES2_ShaderCacheEntry *head;
} GLES2_ShaderCache;

typedef struct GLES2_ProgramCacheEntry
{
    GLuint id;
    SDL_BlendMode blend_mode;
    GLES2_ShaderCacheEntry *vertex_shader;
    GLES2_ShaderCacheEntry *fragment_shader;
    GLuint uniform_locations[16];
    Uint8 color_r, color_g, color_b, color_a;
    Uint8 modulation_r, modulation_g, modulation_b, modulation_a;
    GLfloat projection[4][4];
    struct GLES2_ProgramCacheEntry *prev;
    struct GLES2_ProgramCacheEntry *next;
} GLES2_ProgramCacheEntry;

typedef struct GLES2_ProgramCache
{
    int count;
    GLES2_ProgramCacheEntry *head;
    GLES2_ProgramCacheEntry *tail;
} GLES2_ProgramCache;


typedef struct GLES2_DriverContext
{
    SDL_GLContext *context;

    SDL_bool debug_enabled;

    struct {
        int blendMode;
        SDL_bool tex_coords;
    } current;

#define SDL_PROC(ret,func,params) ret (APIENTRY *func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC
    GLES2_FBOList *framebuffers;
    GLuint window_framebuffer;

    int shader_format_count;
    GLenum *shader_formats;
    GLES2_ShaderCache shader_cache;
    GLES2_ProgramCache program_cache;
    GLES2_ProgramCacheEntry *current_program;
    Uint8 clear_r, clear_g, clear_b, clear_a;
} GLES2_DriverContext;
