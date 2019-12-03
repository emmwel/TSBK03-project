#version 150

out vec4 outColor;

in vec2 texCoord;
uniform sampler2D texLookUnit;

void main(void)
{
	vec4 t = texture(texLookUnit, texCoord);
	if (t.a < 0.01) discard;
	else
		outColor = t;
}
