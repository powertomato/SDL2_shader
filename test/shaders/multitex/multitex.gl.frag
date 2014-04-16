#version 120

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : enable
#define sampler2D sampler2DRect
#define texture2D texture2DRect
#endif

vec4 convertColor(int type, vec4 color);
uniform int color_mode;

uniform sampler2D color_map;
uniform sampler2D color_map2;

void main(void){
	vec4 texcol = texture2D(color_map, gl_TexCoord[0].st); 
	texcol = convertColor(color_mode,texcol);
	vec4 texcol2 = texture2D(color_map2, gl_TexCoord[0].st); 
	texcol2 = convertColor(color_mode,texcol2);

	gl_FragColor = mix(texcol,texcol2,0.5) * gl_Color;
}
