#version 150

in vec3 position;
in vec3 position_right;
in vec3 position_bottom;
in vec3 position_top;
in vec3 position_left;
in vec4 color;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform int mode;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  if(mode == 0){
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
	col = color;
  }
  else{
	vec3 smoothed_pos = (position + position_right + position_bottom + position_top + position_left) / 5;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(smoothed_pos, 1.0f);
	float eps = 0.0001;
	col = max(color, vec4(eps)) / max(position.z, eps) * smoothed_pos.z;
  }
}

