#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    out_Color = texture(texUnit, outTexCoord);
    out_Color = max(out_Color - 1.0, 0.0);
}
