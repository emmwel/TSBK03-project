#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    float pixelSize = 1.0 / 512;
    out_Color = texture(texUnit, outTexCoord);

    // conv, x-led
    float kernel [5] = float[5](1.0, 4.0, 6.0, 4.0, 1.0);

    out_Color = kernel[0] * (texture(texUnit, outTexCoord - vec2(0, 2.0) * pixelSize)) +
                kernel[1] * (texture(texUnit, outTexCoord - vec2(0, 1.0) * pixelSize)) +
                kernel[2] * texture(texUnit, outTexCoord) +
                kernel[3] * (texture(texUnit, outTexCoord + vec2(0, 1.0)* pixelSize));
                kernel[4] * (texture(texUnit, outTexCoord + vec2(0, 2.0)* pixelSize));

    out_Color /= 16.0;
}
