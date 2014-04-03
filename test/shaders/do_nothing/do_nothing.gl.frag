#version 120

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : enable
#define sampler2D sampler2DRect
#define texture2D texture2DRect
#endif

vec4 convertColor(int type, vec4 color);
uniform int color_mode;
uniform vec4 color;

uniform sampler2D colorMap;

void main(void){
	vec4 texcol = texture2D(colorMap, gl_TexCoord[0].st); 
	texcol = convertColor(color_mode,texcol);
	gl_FragColor = texcol * color;
}
