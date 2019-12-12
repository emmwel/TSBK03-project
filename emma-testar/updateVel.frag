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
  // Fetch position and velocity from textures
  vec3 curPos = texture(texUnitPosition, outTexCoord).xyz;
  vec3 curVel = texture(texUnitVelocity, outTexCoord).xyz;

  // TEMPORARY define surface position
  float plane_y = 0.0 + 0.5;

  // Separate velocity direction and magnitude
  float curVelNorm = length(curVel);
  vec3 curVelDirection = curVel / curVelNorm;

  // Calculate forces
  vec3 gravityForce = mass * vec3(0, -50.0, 0);
  // vec3 dragForce = -1/2 * airDragCoefficient * airDensity * splitArea * curVelNorm * curVelNorm * curVelDirection;
  float dragConstants = 0.5;
  vec3 dragForce = dragConstants * curVelNorm * curVelNorm * -curVelDirection;

  // Calculate acceleration from forces
  vec3 acceleration = (1/mass) * (gravityForce + dragForce);

  vec3 newVel = curVel + deltaTime * acceleration;

  // Check for collision with plane
  vec3 newPos = curPos + deltaTime * newVel;
  if (newPos.y < plane_y) {
    vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 impulse = 1.4 * normal * dot(normal, newVel);
    newVel -= impulse;
  }

  // Calculate new velocity from acceleration
  out_Color = vec4(newVel, 1.0);
}
