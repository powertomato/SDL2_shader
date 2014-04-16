#version 100

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : enable
#define sampler2D sampler2DRect
#define texture2D texture2DRect
#endif

precision mediump float;

varying vec4 col_out;
varying vec2 texCoord_out;

vec4 convertColor(int type, vec4 color);
uniform int color_mode;

uniform sampler2D color_map;
uniform sampler2D color_map2;

void main(void){
	vec4 texcol = texture2D(color_map, texCoord_out.st); 
	texcol = convertColor(color_mode, texcol);

	vec4 texcol2 = texture2D(color_map2, texCoord_out.st);
	texcol2 = convertColor(color_mode, texcol2);

	gl_FragColor = mix(texcol,texcol2,0.5) * col_out;
}
