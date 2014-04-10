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
	texcol = texcol * color;

	/* http://www.techrepublic.com/blog/how-do-i/
		how-do-i-convert-images-to-grayscale-and-sepia-tone-using-c/# */
	gl_FragColor.a = texcol.a;
	gl_FragColor.r = texcol.r * 0.393 + texcol.g * 0.769 + texcol.b * 0.189;
	gl_FragColor.g = texcol.r * 0.349 + texcol.g * 0.686 + texcol.b * 0.168;
	gl_FragColor.b = texcol.r * 0.272 + texcol.g * 0.534 + texcol.b * 0.131;
}
