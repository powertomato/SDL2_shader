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

uniform sampler2D colorMap;

void main(void){
	vec4 texcol = texture2D(colorMap, texCoord_out.st); 
	texcol = convertColor(color_mode, texcol);
	texcol = texcol * col_out;

	float grey = 0.2126 *texcol.r + 0.7152 *texcol.g + 0.0722 *texcol.b;
	gl_FragColor = vec4(grey,grey,grey,texcol.a);

}
