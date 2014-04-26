#version 130

uniform mat4 world_view_projection;

attribute vec4 position;
attribute vec4 color;
attribute vec2 texCoords;

varying vec4 color_v2f;
varying vec2 texCoord_v2f;

void main(void){
	color_v2f = color;
	gl_Position = world_view_projection * position;
    texCoord_v2f = texCoords;
}
