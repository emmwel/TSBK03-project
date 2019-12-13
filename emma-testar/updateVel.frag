#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;

// depth variables
uniform sampler2D texUnitDepth;
uniform mat4 orthogonalProjectionMatrix;
uniform mat4 worldToView;

// forces parameters
uniform float splitArea;
uniform float airDragCoefficient;
uniform float airDensity;
uniform float mass;

out vec4 out_Color;

vec3 worldPosFromDepth(float depth, vec2 depthTexCoord) {

    // scale z [-1, 1]
    float z = depth * 2.0 - 1.0;

    // in orthogonal camera coordinates
    vec4 clipSpacePosition = vec4(depthTexCoord * 2.0 - 1.0, z, 1.0);

    // go from projection to view
    vec4 viewSpacePosition = inverse(orthogonalProjectionMatrix) * clipSpacePosition;

    // view to world
    vec4 worldSpacePosition = inverse(worldToView) * viewSpacePosition;

    return worldSpacePosition.xyz;
}



void main(void)
{
  // Fetch position and velocity from textures
  vec3 curPos = texture(texUnitPosition, outTexCoord).xyz;
  vec3 curVel = texture(texUnitVelocity, outTexCoord).xyz;

  float s = (curPos.x + 100) * 4;
  float t = (curPos.z - 100) * (-4);
  vec2 depthTexCoord_in = vec2(s, t);
  float depth_in = texture(texUnitDepth, depthTexCoord_in).x;
  vec3 depthWorld = worldPosFromDepth(depth_in, depthTexCoord_in);

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
  if (newPos.y < depthWorld.z) {
    vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 impulse = 1.4 * normal * dot(normal, newVel);
    newVel -= impulse;
  }

  // Calculate new velocity from acceleration
  out_Color = vec4(newVel, 1.0);
}
