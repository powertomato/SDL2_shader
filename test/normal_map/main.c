
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

	SDL_shader_hint(SDL_GL_TEX0_NAME,"u_texture" );
	SDL_shader_hint(SDL_GL_TEX1_NAME,"u_normals" );
	SDL_Shader* shader = SDL_createShader( renderer, "../shaders/normal_map/normal_map" );

	if ( shader == NULL ) {
		fprintf( stderr, "Error: %s \n", SDL_GetError() );
	}
	SDL_assert(shader != NULL);

	SDL_Uniform* resolution = SDL_createUniform( shader, "Resolution" );
	SDL_Uniform* lightPos = SDL_createUniform( shader, "LightPos" );
	SDL_Uniform* lightColor = SDL_createUniform( shader, "LightColor" );
	SDL_Uniform* ambientColor = SDL_createUniform( shader, "AmbientColor" );
	SDL_Uniform* falloff = SDL_createUniform( shader, "Falloff" );

	SDL_assert(resolution != NULL);
	SDL_assert(lightPos != NULL);
	SDL_assert(lightColor != NULL);
	SDL_assert(ambientColor != NULL);
	SDL_assert(falloff != NULL);

	SDL_setUniform_f2( resolution, 800, 600 );
	SDL_setUniform_f3( lightPos, 0.5,0.5,0.075f );
	SDL_setUniform_f4( lightColor, 1, 0.7f, 0.7f, 7 );
	SDL_setUniform_f4( ambientColor, 1, 1, 1, 0.3f );
	SDL_setUniform_f3( falloff, 0.4f, 3, 20 );
	
	SDL_Texture* tex[2];
	SDL_Surface* srf;
	char* names[] = {"../stone.png", "../stone_n.png"};
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
		SDL_FreeSurface(srf2);
	}

	SDL_SetRenderTarget( renderer, NULL );
	SDL_Event e;
	SDL_SetRenderDrawColor(renderer, 0,0,0,1);
	int ret = 0;
	int quit = 0;
	while ( !quit ) {
		//SDLTest_DrawString( renderer,   8,   8, shader_names[current_shader] );
		SDL_Rect dst = {0,0,800,600};
		ret = SDL_renderCopyShdArray( shader, tex,NULL,&dst,2 );
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
			case SDL_MOUSEMOTION:
				shader->bindShader( shader );
				SDL_setUniform_f3( lightPos,
					(float) e.motion.x/800.0f,
					(float) 1.0f-e.motion.y/600.0f,
					0.450f );
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


	SDL_destroyUniform(shader, resolution);
	SDL_destroyUniform(shader, lightPos);
	SDL_destroyUniform(shader, lightColor);
	SDL_destroyUniform(shader, ambientColor);
	SDL_destroyUniform(shader, falloff);

	SDL_destroyShader( shader );
	SDL_DestroyTexture( tex[0] );
	SDL_DestroyTexture( tex[1] );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( screen );
	SDL_Quit();
	return 0;
}
