#version 420
in vec2 position;
in vec3 colour;
out vec3 Colour;
void main() {
	Colour = vec3(colour.x / 255, colour.y / 255, colour.z / 255);
	gl_Position = vec4((position.x / 363) - 1, (position.y / 369) - 1, 0.0, 1.0);
}