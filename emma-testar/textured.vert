#version 150

in  vec3 in_Position;
//in  vec3 inColor;
//in  vec3 inNormal;
in vec2 in_TexCoord;

//out vec3 exColor; // Gouraud
//out vec3 exNormal; // Phong
out vec2 texCoord;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D texPositionsUnit;
uniform float pixelSize;

void main(void)
{
	texCoord = in_TexCoord;

	// get position from texture
    vec4 tex_position = texture(texPositionsUnit, vec2(gl_InstanceID * pixelSize, 0.0));

    // add texture position to current position
    vec3 tex_pos3D = vec3(tex_position);
    vec3 newpos = in_Position + tex_pos3D;

    // This should include projection
	gl_Position = projectionMatrix * modelviewMatrix * vec4(newpos, 1.0);
}


//#version 150
//
//in  vec3 inPosition;
////in  vec3 inColor;
////in  vec3 inNormal;
//in vec2 inTexCoord;
//
////out vec3 exColor; // Gouraud
////out vec3 exNormal; // Phong
//out vec2 texCoord;
//
//uniform mat4 modelviewMatrix;
//uniform mat4 projectionMatrix;
//uniform sampler2D texPositionsUnit;
//uniform float pixelSize;
//
//
//void main(void)
//{
//	texCoord = inTexCoord;
//
//	gl_Position = projectionMatrix * modelviewMatrix * vec4(inPosition, 1.0);
//}
