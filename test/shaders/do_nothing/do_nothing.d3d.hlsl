
texture xColoredTexture;
float4x4 world_view_projection : WORLDVIEWPROJECTION;

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



sampler ColoredTextureSampler = sampler_state {
	texture =   <xColoredTexture>;
	magfilter = LINEAR;
	minfilter = LINEAR;
	mipfilter = LINEAR;
	AddressU =  mirror;
	AddressV =  mirror;
};

VertexToPixel VertexShaderMain( AppToVertex vs_in ) {
	VertexToPixel vs_out = (VertexToPixel)0;
	vs_out.pos = mul(world_view_projection, vs_in.pos);
	vs_out.texcoord = vs_in.texcoord;
	vs_out.color = vs_in.color;

	return vs_out;
}

PixelToFrame PixelShaderMain(VertexToPixel ps_in) {
	PixelToFrame ps_out = (PixelToFrame)0;
	float4 color = tex2D(ColoredTextureSampler, ps_in.texcoord );
	ps_out.color = color * ps_in.color;
	return ps_out;
}

