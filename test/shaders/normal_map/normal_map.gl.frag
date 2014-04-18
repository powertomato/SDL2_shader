#version 120
/* based on https://github.com/mattdesl/lwjgl-basics/wiki/ShaderLesson6 */


#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : enable
#define sampler2D sampler2DRect
#define texture2D texture2DRect
#endif

vec4 convertColor(int type, vec4 color);
uniform int color_mode;

//attributes from vertex shader
// gl_Color, gl_TexCoord[0].st

//our texture samplers
uniform sampler2D u_texture;   //diffuse map
uniform sampler2D u_normals;   //normal map

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen
uniform vec3 LightPos;        //light position, normalized
uniform vec4 LightColor;      //light RGBA -- alpha is intensity
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
uniform vec3 Falloff;         //attenuation coefficients

void main() {
    //RGBA of our diffuse color
    vec4 DiffuseColor = texture2D(u_texture, gl_TexCoord[0].st);

    //RGB of our normal map
    vec3 NormalMap = texture2D(u_normals, gl_TexCoord[0].st).rgb;

    //The delta position of light
    vec3 LightDir = vec3(LightPos.xy - (gl_FragCoord.xy / Resolution.xy), LightPos.z);

    //Correct for aspect ratio
    LightDir.x *= Resolution.x / Resolution.y;

    //Determine distance (used for attenuation) BEFORE we normalize our LightDir
    float D = length(LightDir);

    //normalize our vectors
    vec3 N = normalize(NormalMap * 2.0 - 1.0);
    vec3 L = normalize(LightDir);

    //Pre-multiply light color with intensity
    //Then perform "N dot L" to determine our diffuse term
    vec3 Diffuse = (LightColor.rgb * LightColor.a) * max(dot(N, L), 0.0);

    //pre-multiply ambient color with intensity
    vec3 Ambient = AmbientColor.rgb * AmbientColor.a;

    //calculate attenuation
    float Attenuation = 1.0 / ( Falloff.x + (Falloff.y*D) + (Falloff.z*D*D) );

    //the calculation which brings it all together
    vec3 Intensity = Ambient + Diffuse * Attenuation;
    vec3 FinalColor = DiffuseColor.rgb * Intensity;
    gl_FragColor = gl_Color * vec4(FinalColor, DiffuseColor.a);
}
