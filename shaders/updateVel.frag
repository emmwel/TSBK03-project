#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;

// Depth variables
uniform sampler2D texUnitDepth;
uniform float planeWidth;
uniform float zFar;
uniform float zNear;
uniform float camHeight;

// Forces parameters
uniform float splitArea;
uniform float airDragCoefficient;
uniform float airDensity;
uniform float mass;
uniform vec3 windDirection;

// Normals
uniform sampler2D texUnitNormals;

// Lifetime
uniform float maxLifetime;

out vec4 out_Color;

// Calculate depth in world coordinates
float worldPosZFromDepth(float depth) {

    // Scale Z between [0, clip-space-size]
    float depth_flipped = (1-depth) * (zFar - zNear);

    return depth_flipped + (camHeight - zFar);
}

void main(void)
{
  // Fetch position, age and velocity from textures
  vec4 curPos4D = texture(texUnitPosition, outTexCoord);
  vec3 curPos = curPos4D.xyz;
  float age = curPos4D[3];
  vec3 curVel = texture(texUnitVelocity, outTexCoord).xyz;

  // Calculate texture coordinates between [0, 1] and get depth
  float u = (curPos.x + planeWidth/2) / planeWidth;
  float v = -1 * (curPos.z - planeWidth/2) / planeWidth;
  vec2 depthTexCoord_in = vec2(u, v);
  float depth_in = texture(texUnitDepth, depthTexCoord_in).x;

  // Transform depth to world coordinates
  float depth_world = worldPosZFromDepth(depth_in);

  // Separate velocity direction and magnitude
  float curVelNorm = length(curVel);
  vec3 curVelDirection = curVel / curVelNorm;

  // --------- Calculate forces ---------
  vec3 gravityForce = mass * vec3(0, -9.82, 0);
  vec3 dragForce = -1/2 * airDragCoefficient * airDensity * splitArea * curVelNorm * curVelNorm * curVelDirection;

  // Calculate wind force
  vec3 windForce;
  if(curPos.y < 0.2){
    windForce = vec3(0.0);
  }
  else{
    float windSpeed = 0.01;
    vec3 windDirection = vec3(0.865, 0.0, 0.5);
    windForce = windSpeed * windDirection;
  }

  // Calculate acceleration from forces
  vec3 acceleration = (1/mass) * (gravityForce + dragForce + windForce);

  // Calculate new velocity from acceleration
  vec3 newVel = curVel + deltaTime * acceleration;

  // Slow down hail closer to the ground (simulated rolling friction)
  if(curPos.y < 5.0){
    float factor = (5.0-curPos.y)/5.0;
    factor = clamp(factor, 0, 0.05);
    newVel.xz -= factor * newVel.xz;
    newVel.y -= factor/2.0 * newVel.y;
  }

  // Check for collision with plane
  vec3 newPos = curPos + deltaTime * newVel;
  if (newPos.y < depth_world) {
    vec3 normal = texture(texUnitNormals, vec2(u, v)).xyz;
    vec3 impulse = 1.15 * normal * dot(normal, newVel);
    newVel -= impulse;
  }

  // Check if the particle is alive or not
  if (age > maxLifetime) {
    newVel = vec3(0.0, 0.0, 0.0);
  }

  // Output velocity
  out_Color = vec4(newVel, 1.0);
}
