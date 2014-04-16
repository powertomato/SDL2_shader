#version 120

uniform mat4 world_view_projection;
attribute vec4 position;
attribute vec2 texCoords;
attribute vec4 color;
void main(void){
	gl_FrontColor = color;
    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = world_view_projection * position;
    gl_TexCoord[0] = vec4(texCoords,1,1); //gl_MultiTexCoord0;
}
