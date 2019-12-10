#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    float depth = texture(texUnit, outTexCoord).x;
    float zNear = 1.0;
    float zFar = 100;
    float c = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));

    out_Color = vec4(c, c, c, 1.0);
}
