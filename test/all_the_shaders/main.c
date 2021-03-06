
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#ifdef __MACOSX__
#include <SDL2_image/SDL_image.h>
#else
#include <SDL2/SDL_image.h>
#endif
#include "../src/test/SDL_test_font.h"
#include "../src/SDL_shader.h"
#include "../src/SDL_SYS_RenderStructs.h"


static char *shader_names[] = {
	"do_nothing",
	"greyscale",
	"sepia",
};
static SDL_Shader** shaders;
static int current_shader;
#define NUM_OF_SHADERS (sizeof(shader_names)/sizeof(char*))

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

#ifdef _WIN32
//prevents displaying the black console window
//on windows systems
//depending on the toolchain (settings) this is also required
#include <windows.h>
int WINAPI WinMain (HINSTANCE hThisInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpszArgument,
		int nCmdShow)
#else
int main(int argc, char** argv) 
#endif
{
#ifdef _WIN32
	LPSTR *argv = __argv;
	int argc = __argc;
	(void) argv;
	(void) argc;
#endif 
	SDL_Window *screen;
	SDL_Renderer *renderer;

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ){
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 1;
	}

	SDL_SetHint( SDL_HINT_RENDER_DRIVER, getenv("RENDER_DRIVER") ); 

	int width = 500;
	int height = 400;
	screen = SDL_CreateWindow("Caption",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width, height,
			SDL_WINDOW_RESIZABLE);
			// SDL_WINDOW_FULLSCREEN_DESKTOP );
	if ( screen == NULL ) {
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 2;
	}
	renderer = SDL_CreateRenderer( screen, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE );
	//renderer = SDL_CreateRenderer( screen, -1, SDL_RENDERER_SOFTWARE|SDL_RENDERER_TARGETTEXTURE );
	if ( renderer == NULL ) {
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 3;
	}

	SDL_RenderSetLogicalSize(renderer, width, height);

	SDL_SetWindowTitle( screen, renderer->info.name );

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	shaders = (SDL_Shader**) malloc( sizeof(SDL_Shader*)*NUM_OF_SHADERS );
	int i;
	for ( i=0; i<NUM_OF_SHADERS; i++) {
		SDL_Shader *shader;
		char buff_name[2048] = {0};
		strncat( buff_name, "../shaders/", 2048 );
		strncat( buff_name, shader_names[i], 2048 );
		strncat( buff_name, "/", 2048 );
		strncat( buff_name, shader_names[i], 2048 );
		printf("loading shader #%i %s\n", i, buff_name );
		shader = SDL_createShader( renderer, buff_name );
		if ( shader == NULL ){
			fprintf(stderr, "Error: %s \n", SDL_GetError());
			return 4;
		}
		shaders[i] = shader;
	}

	SDL_Texture* tex;
	SDL_Surface* srf;
	srf = IMG_Load( "../all.png" );
	if ( !srf ) {
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 5;
	}
	tex = SDL_CreateTextureFromSurface(renderer, srf );
	SDL_FreeSurface(srf);

	SDL_SetRenderTarget( renderer, NULL );
	SDL_Event e;
	SDL_SetRenderDrawColor(renderer, 0,0,0,1);
	int ret = 0;
	int quit = 0;
	while ( !quit ) {
		SDLTest_DrawString( renderer,   8,   8, shader_names[current_shader] );
		SDL_Rect dst = { 50, 50, tex->w, tex->h };
		ret = SDL_renderCopyShd( shaders[current_shader], tex, NULL, &dst );
		if ( ret!=0 ){
			fprintf(stderr,"Err: %s\n", SDL_GetError());
		}

		while (SDL_PollEvent(&e)){
			switch ( e.type ) {
			case SDL_QUIT: 
				quit = 1; 
				break;
			case SDL_KEYDOWN: 
				switch ( e.key.keysym.sym ) {
					case SDLK_SPACE:
						current_shader= mod( (current_shader-1), (NUM_OF_SHADERS));
						break;
					case SDLK_TAB:
						current_shader= mod( (current_shader+1), (NUM_OF_SHADERS));
						break;
				}
				break;
			case SDL_WINDOWEVENT:
				for( int i=0; i<NUM_OF_SHADERS; i++ ){
					SDL_updateViewport( shaders[i] );
				}
			}
		}

		SDL_RenderPresent(renderer);

		SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	}
	for( i=0; i<NUM_OF_SHADERS; i++ ){
		SDL_destroyShader( shaders[i] );
	}
	SDL_DestroyTexture( tex );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( screen );
	SDL_Quit();
	free(shaders);
	return 0;
}
