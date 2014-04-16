#version 100

uniform mat4 world_view_projection;

attribute vec4 position;
attribute vec2 texCoords;
attribute vec4 color;

varying vec4 col_out;
varying vec2 texCoord_out;

void main(){
	col_out = color;
    gl_Position = world_view_projection * position;
    texCoord_out = texCoords;
}
