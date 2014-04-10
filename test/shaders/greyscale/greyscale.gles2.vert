#version 100

uniform mat4 world_view_projection;
uniform vec4 color;

attribute vec4 position;
attribute vec2 texCoords;

varying vec4 col_out;
varying vec2 texCoord_out;

void main(){
	col_out = color;
    gl_Position = world_view_projection * position;
    texCoord_out = texCoords;
}
