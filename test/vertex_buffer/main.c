
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_test_font.h>
#include <SDL2/SDL_image.h>
#include "../src/SDL_shader.h"
#include "../src/SDL_SYS_RenderStructs.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /*M_PI*/


/*static char *shader_names[] = {
	"do_nothing",
	"greyscale",
	"sepia",
};
static SDL_Shader** shaders;
static int current_shader;
#define NUM_OF_SHADERS (sizeof(shader_names)/sizeof(char*))*/

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

//            a5
//          ,-'\,
//   a4 ,-''     \,
//     |           \,
//     |     +- - - :-a3
//     |           /'
//   a2'-.       /'
//        `'--./'
//            a1

	SDL_shader_hint(SDL_GL_TEX0_NAME,"color_map" );
	SDL_shader_hint(SDL_GL_TEX1_NAME,"color_map2" );
	SDL_Shader* shader = SDL_createShader( renderer, "../shaders/multitex/multitex" );
	SDL_Vertex *vertices = shader->createVertexBuffer( 5 );
	float angles[] = { 2*M_PI/5*4, 2*M_PI/5*3, 0, 2*M_PI/5*2, 2*M_PI/5*1 };
	for ( int i=0; i<5; i++ ){
		vertices->setVertexColor( vertices,i,1,255,255,255,255 );
		vertices->setVertexPosition( vertices,i,1,
				150*cos(angles[i])+250, 150*sin(angles[i])+200,0.0f );
		vertices->setVertexTexCoord( vertices,i,1,
				(cos(angles[i])/2+0.5), (sin(angles[i])/2+0.5) );
	}

	SDL_Texture* tex[2];
	SDL_Surface* srf;
	char* names[] = {"../img.png", "../img2.png"};
	for( int i=0; i<2; i++ ){
		srf = IMG_Load( names[i] );

		SDL_PixelFormat* fmt = SDL_AllocFormat( SDL_PIXELFORMAT_ARGB8888 );
			SDL_assert(fmt != NULL);
		SDL_Surface* srf2 = SDL_ConvertSurface(srf, fmt, 0);
			SDL_assert(srf2 != NULL);

		if ( !srf ) {
			fprintf(stderr, "Error: %s \n", SDL_GetError());
			return 5;
		}
		tex[i] = SDL_CreateTextureFromSurface(renderer, srf2 );

		SDL_FreeSurface(srf);
		SDL_FreeSurface( srf2 );
	}

	SDL_SetRenderTarget( renderer, NULL );
	SDL_Event e;
	SDL_SetRenderDrawColor(renderer, 0,0,0,1);
	int ret = 0;
	int quit = 0;
	while ( !quit ) {
		//SDLTest_DrawString( renderer,   8,   8, shader_names[current_shader] );
		ret = SDL_renderVertexBuffer( shader, vertices, tex, 2 );
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
	SDL_DestroyTexture( tex[0] );
	SDL_DestroyTexture( tex[1] );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( screen );
	SDL_Quit();
	return 0;
}
