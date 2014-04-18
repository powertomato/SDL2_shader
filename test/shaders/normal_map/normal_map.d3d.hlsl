/* based on https://github.com/mattdesl/lwjgl-basics/wiki/ShaderLesson6 */

texture xColoredTexture;
float4x4 world_view_projection : WORLDVIEWPROJECTION;

float2 Resolution;      //resolution of screen
float3 LightPos;        //light position, normalized
float4 LightColor;      //light RGBA -- alpha is intensity
float4 AmbientColor;    //ambient RGBA -- alpha is intensity 
float3 Falloff;         //attenuation coefficients

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



sampler u_texture = sampler_state {
	texture =   <xColoredTexture>;
	magfilter = LINEAR;
	minfilter = LINEAR;
	mipfilter = LINEAR;
	AddressU =  mirror;
	AddressV =  mirror;
};

sampler u_normals = sampler_state {
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

    //RGBA of our diffuse color
    float4 DiffuseColor = tex2D(u_texture, ps_in.texcoord );

    //RGB of our normal map
    float3 NormalMap = tex2D(u_normals, ps_in.texcoord).rgb;

    //The delta position of light
	ps_in.texcoord.y = 1-ps_in.texcoord.y;
    float3 LightDir = float3(LightPos.xy - (ps_in.texcoord ), LightPos.z);

    //Correct for aspect ratio
    LightDir.x *= Resolution.x / Resolution.y;

    //Determine distance (used for attenuation) BEFORE we normalize our LightDir
    float D = length(LightDir);

    //normalize our vectors
    float3 N = normalize(NormalMap * 2.0 - 1.0);
    float3 L = normalize(LightDir);

    //Pre-multiply light color with intensity
    //Then perform "N dot L" to determine our diffuse term
    float3 Diffuse = (LightColor.rgb * LightColor.a) * max(dot(N, L), 0.0);

    //pre-multiply ambient color with intensity
    float3 Ambient = AmbientColor.rgb * AmbientColor.a;

    //calculate attenuation
    float Attenuation = 1.0 / ( Falloff.x + (Falloff.y*D) + (Falloff.z*D*D) );

    //the calculation which brings it all together
    float3 Intensity = Ambient + Diffuse * Attenuation;
    float3 FinalColor = DiffuseColor.rgb * Intensity;

	ps_out.color = float4(FinalColor, DiffuseColor.a) * ps_in.color;
	return ps_out;
}

