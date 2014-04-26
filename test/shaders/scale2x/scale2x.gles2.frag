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
	if ( mod(int(gl_FragCoord.y), 2) == 0.0 ) {
		vec4 texcol = texture2D(colorMap, texCoord_out.st);
		texcol = convertColor(color_mode,texcol);
		gl_FragColor = texcol * col_out;
	} else {
		gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	}
}
