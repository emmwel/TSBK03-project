#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;

// forces parameters
uniform float splitArea;
uniform float airDragCoefficient;
uniform float airDensity;
uniform float mass;

out vec4 out_Color;

void main(void)
{
    vec4 curPos = texture(texUnitPosition, outTexCoord);
    vec4 curVel = texture(texUnitVelocity, outTexCoord);

    float plane_y = 0.0 + 0.5;

    // calculate forces
    vec4 gravityForce = mass * vec4(0, -9.82, 0, 0);
    vec4 dragForce = 1/2 * airDragCoefficient * airDensity * splitArea * (curVel * curVel) * -1 * normalize(curVel);

    vec4 acceleration = (1/mass) * (gravityForce + dragForce);

    vec4 newVel = curVel + deltaTime * acceleration;

    vec4 newPos = curPos + deltaTime * newVel;

    if (newPos.y < plane_y) {
        newVel = 0.2 * -newVel;
    }

    out_Color = newVel;
}
