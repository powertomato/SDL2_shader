
texture xColoredTexture;
float4x4 world_view_projection : WORLDVIEWPROJECTION;
int scaler;

struct AppToVertex {
	float4 pos        : POSITION;
	float2 texcoord   : TEXCOORD0;
	float4 color      : COLOR0;
};

struct VertexToPixel {
	float4 pos        : POSITION;
	float2 texcoord   : TEXCOORD0;
	float4 color      : COLOR0;
};

struct PixelToFrame {
	float4 color : COLOR0;
};



sampler colorMap = sampler_state {
	texture =   <xColoredTexture>;
	magfilter = LINEAR;
	minfilter = LINEAR;
	mipfilter = LINEAR;
	AddressU =  mirror;
	AddressV =  mirror;
};


bool col_compare( float4 a, float4 b) {
	return !any(a-b);
}

//   A    --\ a' b'
// C P B  --/ c' d'
//   D 
//  a=P; b=P; c=P; d=P;
//  IF C==A AND C!=D AND A!=B => a'=A
//  IF A==B AND A!=C AND B!=D => b'=B
//  IF D==C AND D!=B AND C!=A => c'=C
//  IF B==D AND B!=A AND D!=C => d'=D
float4 scale2x( VertexToPixel ps_in ){
	float4 A,B,C,D,P;
	bool a_eq_b, b_eq_d, d_eq_c, c_eq_a;
	bool y_even = int( ps_in.texcoord.y * 600 ) % 2 == 0;
	bool x_even = int( ps_in.texcoord.x * 800 ) % 2 == 0;

	P = tex2D(colorMap, ps_in.texcoord );

	A = tex2D( colorMap, ps_in.texcoord - float2(0,1)/300 );
	B = tex2D( colorMap, ps_in.texcoord + float2(1,0)/400 );
	C = tex2D( colorMap, ps_in.texcoord - float2(1,0)/400 );
	D = tex2D( colorMap, ps_in.texcoord + float2(0,1)/300 );

	a_eq_b = col_compare(A,B);
	b_eq_d = col_compare(B,D);
	d_eq_c = col_compare(D,C);
	c_eq_a = col_compare(C,A);

	if (  x_even &&  y_even ) {
		// a'
		if( c_eq_a && !d_eq_c && !a_eq_b ) {
			return A;
		}
	} else 
	if ( !x_even &&  y_even ) {
		// b'
		if( a_eq_b && !c_eq_a && !b_eq_d ) {
			return B;
		}
	} else
	if (  x_even && !y_even ) {
		// c'
		if( d_eq_c && !b_eq_d && !c_eq_a ) {
			return C;
		}
	} else
	if ( !x_even && !y_even ) {
		// d'
		if( b_eq_d && !a_eq_b && !d_eq_c ) {
			return D;
		}
	}
	return P;
}

VertexToPixel VertexShaderMain( AppToVertex vs_in ) {
	VertexToPixel vs_out = (VertexToPixel)0;
	vs_out.pos = mul(world_view_projection, vs_in.pos);
	vs_out.texcoord = vs_in.texcoord;
	vs_out.color = vs_in.color;

	return vs_out;
}

PixelToFrame PixelShaderMain(VertexToPixel ps_in) {
	PixelToFrame ps_out = (PixelToFrame)0;
	float4 color;
	if ( scaler == 0 ) {
		color = tex2D(colorMap, ps_in.texcoord );
	} else 
	if ( scaler == 1 ) {
		color = scale2x( ps_in );
	}

	ps_out.color = color * ps_in.color;
	return ps_out;
}

