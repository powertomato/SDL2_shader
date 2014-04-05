#version 120

uniform mat4 world_view_projection;
uniform vec4 color;
attribute vec4 position;
attribute vec2 texCoords;
void main(void){
	gl_FrontColor = color;
    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = world_view_projection * position;
    gl_TexCoord[0] = vec4(texCoords,1,1); //gl_MultiTexCoord0;
}
