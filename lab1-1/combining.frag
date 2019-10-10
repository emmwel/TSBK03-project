#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
uniform sampler2D texUnit2;
out vec4 out_Color;

void main(void)
{
    vec4 a = texture(texUnit, outTexCoord);
    vec4 b = texture(texUnit2, outTexCoord);

    out_Color = (a * 1.0 + b * 1.0);
}
