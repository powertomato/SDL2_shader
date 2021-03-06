#version 120

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : enable
#define sampler2D sampler2DRect
#define texture2D texture2DRect
#endif

vec4 convertColor(int type, vec4 color);
uniform int color_mode;

uniform sampler2D colorMap;

void main(void){
	vec4 texcol = texture2D(colorMap, gl_TexCoord[0].st); 
	texcol = convertColor(color_mode,texcol);
	texcol = texcol * gl_Color;

	float grey = 0.2126 *texcol.r + 0.7152 *texcol.g + 0.0722 *texcol.b;
	gl_FragColor = vec4(grey,grey,grey,texcol.a);
}
