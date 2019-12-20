#version 150

in vec3 in_Position;
in vec2 in_TexCoord;
out vec2 texCoord;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D texPositionsUnit;
uniform float pixelSize;
uniform int textureWidthHeight;

out float z_dist;

void main(void)
{
	texCoord = in_TexCoord;

	// Get position from texture
	vec4 tex_position = texture(
		texPositionsUnit,
		vec2(
			mod(gl_InstanceID * pixelSize, textureWidthHeight),
			gl_InstanceID * pixelSize / textureWidthHeight
		)
	);

	// Add texture position to current position
	vec3 tex_pos3D = vec3(tex_position);
	vec3 newpos = in_Position + tex_pos3D;
	//vec3 newpos = in_Position;

	// Set z_dist to calculate fog
	z_dist = newpos.z;

	// This should include projection
	gl_Position = projectionMatrix * modelviewMatrix * vec4(newpos, 1.0);
}
