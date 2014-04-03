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
	vec4 color = texture2D(colorMap, texCoord_out.st); 
	gl_FragColor = convertColor(color_mode,color);
}
