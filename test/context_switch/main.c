
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_test_font.h>
#include <SDL2/SDL_image.h>
#include "../src/SDL_shader.h"
#include "../src/SDL_SYS_RenderStructs.h"

#define NUM_OF_SHADERS (sizeof(shader_names)/sizeof(char*))


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

	SDL_Shader *shader;
	shader = SDL_createShader( renderer, "../shaders/do_nothing/do_nothing" );

	SDL_Texture* tex;
	SDL_Surface* srf;
	srf = IMG_Load( "../img.png" );
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
		SDLTest_DrawString( renderer,   8,   8, "test" );
		SDL_Rect dst;
		// Shd -> Copy -> Shd
		SDL_SetTextureColorMod(tex, 255,255,255);
		dst = (SDL_Rect) { 25, 25, tex->w, tex->h };
		ret = SDL_renderCopyShd( shader, tex, NULL, &dst );

		SDL_SetTextureColorMod(tex, 255,0,0);
		dst = (SDL_Rect){ 100, 25, tex->w, tex->h };
		ret = SDL_RenderCopy( renderer, tex, NULL, &dst );

		SDL_SetTextureColorMod(tex, 255,255,255);
		dst = (SDL_Rect){ 175, 25, tex->w, tex->h };
		ret = SDL_renderCopyShd( shader, tex, NULL, &dst );

		// Copy -> Shd -> Copy
		SDL_SetTextureColorMod(tex, 255,255,255);
		dst = (SDL_Rect){ 25, 100, tex->w, tex->h };
		ret = SDL_RenderCopy( renderer, tex, NULL, &dst );

		SDL_SetTextureColorMod(tex, 0,255,0);
		dst = (SDL_Rect){ 100, 100, tex->w, tex->h };
		ret = SDL_renderCopyShd( shader, tex, NULL, &dst );

		SDL_SetTextureColorMod(tex, 255,255,255);
		dst = (SDL_Rect){ 175, 100, tex->w, tex->h };
		ret = SDL_RenderCopy( renderer, tex, NULL, &dst );

		// Draw -> Shd -> Draw
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		dst = (SDL_Rect){ 25, 175, tex->w, tex->h };
		ret = SDL_RenderFillRect( renderer, &dst );

		SDL_SetTextureColorMod(tex, 255,0,255);
		dst = (SDL_Rect){ 100, 175, tex->w, tex->h };
		ret = SDL_renderCopyShd( shader, tex, NULL, &dst );

		dst = (SDL_Rect){ 175, 175, tex->w, tex->h };
		ret = SDL_RenderFillRect( renderer, &dst );

		// Shd -> Draw -> Shd
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		dst = (SDL_Rect){ 25, 250, tex->w, tex->h };
		ret = SDL_renderCopyShd( shader, tex, NULL, &dst );

		SDL_SetTextureColorMod(tex, 255,0,255);
		dst = (SDL_Rect){ 100, 250, tex->w, tex->h };
		ret = SDL_RenderFillRect( renderer, &dst );

		dst = (SDL_Rect){ 175, 250, tex->w, tex->h };
		ret = SDL_RenderFillRect( renderer, &dst );
		ret = SDL_renderCopyShd( shader, tex, NULL, &dst );


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
						break;
					case SDLK_TAB:
						break;
				}
				break;
			}
		}

		SDL_RenderPresent(renderer);

		SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	}

	return 0;
}
