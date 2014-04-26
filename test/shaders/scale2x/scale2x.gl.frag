#version 130

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : enable
#define sampler2D sampler2DRect
#define texture2D texture2DRect
#endif

vec4 convertColor(int type, vec4 color);
uniform int color_mode;

varying vec4 color_v2f;
varying vec2 texCoord_v2f;

uniform sampler2D colorMap;
uniform int scaler;

bool col_compare( vec4 a, vec4 b) {
	return a==b;
}


//   A    --\ a' b'
// C P B  --/ c' d'
//   D 
//  a=P; b=P; c=P; d=P;
//  IF C==A AND C!=D AND A!=B => a'=A
//  IF A==B AND A!=C AND B!=D => b'=B
//  IF D==C AND D!=B AND C!=A => c'=C
//  IF B==D AND B!=A AND D!=C => d'=D
void scale2x( ){
	vec4 A,B,C,D,P;
	bool a_eq_b, b_eq_d, d_eq_c, c_eq_a;
	bool y_even = int(gl_FragCoord.y) % 2 == 0;
	bool x_even = int(gl_FragCoord.x) % 2 == 0;

	/* note: if you know the color-mode you don't have to do any conversion
	         that would likely speed up the shader */
	P = texture2D(colorMap, texCoord_v2f.st);
	P = convertColor(color_mode,P);

	A = texture2D(colorMap, texCoord_v2f.st - vec2(0,1) );
	A = convertColor(color_mode,A);
	B = texture2D(colorMap, texCoord_v2f.st + vec2(1,0) );
	B = convertColor(color_mode,B);
	C = texture2D(colorMap, texCoord_v2f.st - vec2(1,0) );
	C = convertColor(color_mode,C);
	D = texture2D(colorMap, texCoord_v2f.st + vec2(0,1) );
	D = convertColor(color_mode,D);

	a_eq_b = col_compare(A,B);
	b_eq_d = col_compare(B,D);
	d_eq_c = col_compare(D,C);
	c_eq_a = col_compare(C,A);

	if (  x_even &&  y_even ) {
		// a'
		if( c_eq_a && !d_eq_c && !a_eq_b ) {
			gl_FragColor = A * color_v2f;
			return;
		}
	} else 
	if ( !x_even &&  y_even ) {
		// b'
		if( a_eq_b && !c_eq_a && !b_eq_d ) {
			gl_FragColor = B * color_v2f;
			return;
		}
	} else
	if (  x_even && !y_even ) {
		// c'
		if( d_eq_c && !b_eq_d && !c_eq_a ) {
			gl_FragColor = C * color_v2f;
			return;
		}
	} else
	if ( !x_even && !y_even ) {
		// d'
		if( b_eq_d && !a_eq_b && !d_eq_c ) {
			gl_FragColor = D * color_v2f;
			return;
		}
	}
	gl_FragColor = P * color_v2f;
}

void main(void){

	if ( scaler == 0 ) {
		// no scaler
		vec4 P;
		P = texture2D(colorMap, texCoord_v2f.st);
		P = convertColor(color_mode,P);
		gl_FragColor = P * color_v2f;
		return;
	} else
	if ( scaler == 1 ) {
		// use scale2x/Mame2x
		scale2x();
		return;
	}
}
