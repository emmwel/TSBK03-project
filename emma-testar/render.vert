#version 150

in  vec3 in_Position;
in  vec3 in_Normal;
in vec2 in_TexCoord;

out vec3 exNormal; // Phong
out vec3 exSurface; // Phong (specular)

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D texUnit;

void main(void)
{
	exNormal = inverse(transpose(mat3(modelviewMatrix))) * in_Normal; // Phong, "fake" normal transformation

	exSurface = vec3(modelviewMatrix * vec4(in_Position, 1.0)); // Don't include projection here - we only want to go to view coordinates

    // get position from texture
    vec4 newpos = in_Position + texture(texUnit, in_TexCoord);
	//vec3 newpos = in_Position + vec3(0, 0.1*gl_InstanceID, 0);
	gl_Position = projectionMatrix * modelviewMatrix * vec4(newpos, 1.0); // This should include projection
}
