
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
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /*M_PI*/

char* scaler_names[] = {
	"No scaler",
	"scale2x/MAME2x"
};

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

	int width = 800;
	int height = 600;
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


	int val = 3;
	SDL_shader_hint( SDL_D3D_PS_MAJOR_VERSION, &val );
	val = 0;
	SDL_shader_hint( SDL_D3D_VS_MINOR_VERSION, &val );

	SDL_Shader* shader = SDL_createShader( renderer, "../shaders/scale2x/scale2x" );
	if ( shader == NULL ) {
		fprintf( stderr, "Error: %s \n", SDL_GetError() );
	}
	SDL_assert(shader != NULL);

	SDL_Uniform* scaler = SDL_createUniform( shader, "scaler" );
	SDL_assert(scaler);
	SDL_setUniform_i( scaler, 0 );

	SDL_Texture *image;
	SDL_Texture *target_ld, *target_hd;
	SDL_Surface *srf;

	srf = IMG_Load( "../pixel_art.png" );

	SDL_PixelFormat* fmt = SDL_AllocFormat( SDL_PIXELFORMAT_ARGB8888 );
	SDL_assert(fmt != NULL);
	SDL_Surface* srf2 = SDL_ConvertSurface(srf, fmt, 0);
	SDL_assert(srf2 != NULL);

	if ( !srf ) {
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 5;
	}
	image = SDL_CreateTextureFromSurface(renderer, srf2 );

	SDL_FreeSurface(srf);
	SDL_FreeSurface(srf2);


	target_ld = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET, 400, 300 );
	target_hd = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET, 800, 600 );

	SDL_SetRenderTarget( renderer, NULL );
	SDL_Event e;
	SDL_SetRenderDrawColor(renderer, 0,0,0,255);
	int ret = 0;
	int quit = 0;
	while ( !quit ) {
		static int scaler_num=0;
		// Step 1: 
		//   Render something to the low-res texture
		SDL_SetRenderTarget( renderer, target_ld );
		SDL_RenderCopy( renderer, image, NULL, NULL );
		SDLTest_DrawString( renderer,   8,   8, scaler_names[scaler_num] );
		// Step 2:
		//   Copy low-res texture to the high-res texture, using scale shader
		SDL_SetRenderTarget( renderer, target_hd );
		SDL_Rect dst = {0,0,800,600};
		ret = SDL_renderCopyShd( shader, target_ld ,NULL,&dst);
		//ret = SDL_RenderCopy( renderer, target_ld, NULL, NULL );
		if ( ret!=0 ){
			fprintf(stderr,"Err: %s\n", SDL_GetError());
		}
		// Step 3:
		//    Render something to high-res texture

		// Step 4:
		//   Copy the high-res texture to the screen
		SDL_SetRenderTarget( renderer, NULL );
		//SDL_renderCopyShd( shader, target_hd, NULL, &dst );
		SDL_RenderCopy( renderer, target_hd, NULL, NULL );

		while (SDL_PollEvent(&e)){
			switch ( e.type ) {
			case SDL_QUIT: 
				quit = 1; 
				break;
			case SDL_KEYDOWN: 
				switch ( e.key.keysym.sym ) {
					case SDLK_SPACE:
						scaler_num = mod(scaler_num+1, 2);
						SDL_bindShader( shader );
						SDL_setUniform_i( scaler, scaler_num );
						break;
					case SDLK_TAB:
						scaler_num = mod(scaler_num-1, 2);
						SDL_bindShader( shader );
						SDL_setUniform_i( scaler, scaler_num );
						break;
				}
				break;
			//case SDL_WINDOWEVENT:
			//	SDL_updateViewport( shader );
			}
		}

		SDL_RenderPresent(renderer);

		SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	}


	SDL_destroyShader( shader );
	SDL_DestroyTexture( image );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( screen );
	SDL_Quit();
	return 0;
}
