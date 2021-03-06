
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

#define NUM_OF_TEXTURES (sizeof(formats)/sizeof(Uint32))

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
	SDL_Shader *shader;

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ){
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 1;
	}

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

	SDL_SetHint( SDL_HINT_RENDER_DRIVER, getenv("RENDER_DRIVER") ); 

	int width = 500;
	int height = 700;
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


	printf("renderer name: %s\n" , renderer->info.name );
	printf("SDL_PIXELFORMAT_UNKNOWN=    %d\n",SDL_PIXELFORMAT_UNKNOWN);
	printf("SDL_PIXELFORMAT_INDEX1LSB=  %d\n",SDL_PIXELFORMAT_INDEX1LSB);
	printf("SDL_PIXELFORMAT_INDEX1MSB=  %d\n",SDL_PIXELFORMAT_INDEX1MSB);
	printf("SDL_PIXELFORMAT_INDEX4LSB=  %d\n",SDL_PIXELFORMAT_INDEX4LSB);
	printf("SDL_PIXELFORMAT_INDEX4MSB=  %d\n",SDL_PIXELFORMAT_INDEX4MSB);
	printf("SDL_PIXELFORMAT_INDEX8=     %d\n",SDL_PIXELFORMAT_INDEX8);
	printf("SDL_PIXELFORMAT_RGB332=     %d\n",SDL_PIXELFORMAT_RGB332);
	printf("SDL_PIXELFORMAT_RGB444=     %d\n",SDL_PIXELFORMAT_RGB444);
	printf("SDL_PIXELFORMAT_RGB555=     %d\n",SDL_PIXELFORMAT_RGB555);
	printf("SDL_PIXELFORMAT_BGR555=     %d\n",SDL_PIXELFORMAT_BGR555);
	printf("SDL_PIXELFORMAT_ARGB4444=   %d\n",SDL_PIXELFORMAT_ARGB4444);
	printf("SDL_PIXELFORMAT_RGBA4444=   %d\n",SDL_PIXELFORMAT_RGBA4444);
	printf("SDL_PIXELFORMAT_ABGR4444=   %d\n",SDL_PIXELFORMAT_ABGR4444);
	printf("SDL_PIXELFORMAT_BGRA4444=   %d\n",SDL_PIXELFORMAT_BGRA4444);
	printf("SDL_PIXELFORMAT_ARGB1555=   %d\n",SDL_PIXELFORMAT_ARGB1555);
	printf("SDL_PIXELFORMAT_RGBA5551=   %d\n",SDL_PIXELFORMAT_RGBA5551);
	printf("SDL_PIXELFORMAT_ABGR1555=   %d\n",SDL_PIXELFORMAT_ABGR1555);
	printf("SDL_PIXELFORMAT_BGRA5551=   %d\n",SDL_PIXELFORMAT_BGRA5551);
	printf("SDL_PIXELFORMAT_RGB565=     %d\n",SDL_PIXELFORMAT_RGB565);
	printf("SDL_PIXELFORMAT_BGR565=     %d\n",SDL_PIXELFORMAT_BGR565);
	printf("SDL_PIXELFORMAT_RGB24=      %d\n",SDL_PIXELFORMAT_RGB24);
	printf("SDL_PIXELFORMAT_BGR24=      %d\n",SDL_PIXELFORMAT_BGR24);
	printf("SDL_PIXELFORMAT_RGB888=     %d\n",SDL_PIXELFORMAT_RGB888);
	printf("SDL_PIXELFORMAT_RGBX8888=   %d\n",SDL_PIXELFORMAT_RGBX8888);
	printf("SDL_PIXELFORMAT_BGR888=     %d\n",SDL_PIXELFORMAT_BGR888);
	printf("SDL_PIXELFORMAT_BGRX8888=   %d\n",SDL_PIXELFORMAT_BGRX8888);
	printf("SDL_PIXELFORMAT_ARGB8888=   %d\n",SDL_PIXELFORMAT_ARGB8888);
	printf("SDL_PIXELFORMAT_RGBA8888=   %d\n",SDL_PIXELFORMAT_RGBA8888);
	printf("SDL_PIXELFORMAT_ABGR8888=   %d\n",SDL_PIXELFORMAT_ABGR8888);
	printf("SDL_PIXELFORMAT_BGRA8888=   %d\n",SDL_PIXELFORMAT_BGRA8888);
	printf("SDL_PIXELFORMAT_ARGB2101010=%d\n",SDL_PIXELFORMAT_ARGB2101010);

	shader = SDL_createShader( renderer, "../shaders/do_nothing/do_nothing" );
	if ( shader == NULL ){
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 4;
	}

	SDL_Surface* srf;
	srf = IMG_Load( "../img.png" );
	if ( !srf ) {
		fprintf(stderr, "Error: %s \n", SDL_GetError());
		return 5;
	}

	int i;
	Uint32 formats[] = {
		/* Indexed formats and YUV are not supported */
		SDL_PIXELFORMAT_RGB332,
		SDL_PIXELFORMAT_RGB444,
		SDL_PIXELFORMAT_RGB555,
		SDL_PIXELFORMAT_BGR555,
		SDL_PIXELFORMAT_ARGB4444,
		SDL_PIXELFORMAT_RGBA4444,
		SDL_PIXELFORMAT_ABGR4444,
		SDL_PIXELFORMAT_BGRA4444,
		SDL_PIXELFORMAT_ARGB1555,
		SDL_PIXELFORMAT_RGBA5551,
		SDL_PIXELFORMAT_ABGR1555,
		SDL_PIXELFORMAT_BGRA5551,
		SDL_PIXELFORMAT_RGB565,
		SDL_PIXELFORMAT_BGR565,
		SDL_PIXELFORMAT_RGB24,
		SDL_PIXELFORMAT_BGR24,
		SDL_PIXELFORMAT_RGB888,
		SDL_PIXELFORMAT_RGBX8888,
		SDL_PIXELFORMAT_BGR888,
		SDL_PIXELFORMAT_BGRX8888,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_PIXELFORMAT_ABGR8888,
		SDL_PIXELFORMAT_BGRA8888,
		SDL_PIXELFORMAT_ARGB2101010
	};
	
	char fmt_names[][NUM_OF_TEXTURES] = {
		"RGB332",
		"RGB444",
		"RGB555",
		"BGR555",
		"ARGB4444",
		"RGBA4444",
		"ABGR4444",
		"BGRA4444",
		"ARGB1555",
		"RGBA5551",
		"ABGR1555",
		"BGRA5551",
		"RGB565",
		"BGR565",
		"RGB24",
		"BGR24",
		"RGB888",
		"RGBX8888",
		"BGR888",
		"BGRX8888",
		"ARGB8888",
		"RGBA8888",
		"ABGR8888",
		"BGRA8888",
		"ARGB2101010"
	};

	SDL_BlendMode blendModes[] = {
		SDL_BLENDMODE_NONE,
		SDL_BLENDMODE_BLEND,
		SDL_BLENDMODE_ADD,
		SDL_BLENDMODE_MOD
	};
	char* blendNames[] = {
		"NONE",
		"BLEND",
		"ADD",
		"MOD"
	};
	int current_blend = 0;

	int colors[][4] = {
		{255,255,255,255},
		{255,  0,  0,255},
		{  0,255,  0,255},
		{  0,  0,255,255},
		{255,255,255,127}
	};
	char* colorNames[] = {
		"white",
		"red",
		"green",
		"blue",
		"semi transp."
	};
	int current_color = 0;

	SDL_Texture* textures[ NUM_OF_TEXTURES ];
	
	for ( i=0; i< (int)(NUM_OF_TEXTURES); i++) {
		SDL_PixelFormat* fmt = SDL_AllocFormat( formats[i] );
			SDL_assert(fmt != NULL);
		SDL_Surface* srf2 = SDL_ConvertSurface(srf, fmt, 0);
			SDL_assert(srf2 != NULL);
		textures[i] = SDL_CreateTextureFromSurface(renderer, srf2 );

		SDL_SetTextureBlendMode(textures[i], blendModes[current_blend]);
		SDL_FreeSurface( srf2 );
		SDL_FreeFormat(fmt);

		if ( !textures[i] ){
			return 1000 + i;
		}

		if ( textures[i]->native ){
			printf("native_tex: %d \n",textures[i]->native->format );
		}else{
			printf("SDL_tex: %d \n",textures[i]->format );
		}

	}
	SDL_FreeSurface(srf);

	SDL_SetRenderTarget( renderer, NULL );
	SDL_Event e;
	SDL_SetRenderDrawColor(renderer, 0,0,0,1);
	int ret = 0;
	int quit = 0;

	while ( !quit ) {
		SDLTest_DrawString( renderer,   8,   8, blendNames[current_blend] );
		SDLTest_DrawString( renderer, 108,   8, colorNames[current_color] );
		for ( i=0; i< (int)(NUM_OF_TEXTURES); i++) {
			int x=30+(i%8)*55;
			int y=30+(i/8)*75;
			SDL_Rect dst = {x,y,50,50};
			ret = SDL_renderCopyShd( shader, textures[i], NULL, &dst );
			if ( ret!=0 ){
				fprintf(stderr,"Err: %s\n", SDL_GetError());
			}
			SDLTest_DrawString( renderer, dst.x,dst.y+55 + (i%2==0 ? 10 : 0), fmt_names[i] );
			
			dst.y += (NUM_OF_TEXTURES)/8 * 75 + 85;
			SDL_RenderCopy( renderer, textures[i], NULL, &dst );
			SDLTest_DrawString( renderer, dst.x,dst.y+55 + (i%2==0 ? 10 : 0), fmt_names[i] );

		}
		while (SDL_PollEvent(&e)){
			switch ( e.type ) {
				case SDL_QUIT: 
					quit = 1; 
					break;
				case SDL_KEYDOWN: 
					switch ( e.key.keysym.sym ) {
						case SDLK_SPACE:
							current_blend = (current_blend+1)%4;
							for ( i=0; i< (int)(NUM_OF_TEXTURES); i++) {
								SDL_SetTextureBlendMode(textures[i], blendModes[current_blend]);
							}
							break;
						case SDLK_TAB:
							current_color = (current_color+1)%5;
							for ( i=0; i< (int)(NUM_OF_TEXTURES); i++) {
								SDL_SetTextureColorMod(textures[i], 
										colors[current_color][0],
										colors[current_color][1],
										colors[current_color][2]);
								SDL_SetTextureAlphaMod(textures[i], colors[current_color][3] );
							}
							break;
					}
					break;
				case SDL_WINDOWEVENT:
					SDL_updateViewport( shader );
			}
		}

		SDL_RenderPresent(renderer);

		SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	}
	for ( i=0; i<(int)NUM_OF_TEXTURES; i++ ) {
		SDL_DestroyTexture( textures[i] );
		textures[i] = NULL;
	}
	SDL_destroyShader( shader );
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( screen );
	SDL_Quit();

	return 0;
}
