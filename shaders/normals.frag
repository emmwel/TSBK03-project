#version 150

in vec2 depthTexCoord0;
uniform sampler2D texUnitDepth;
uniform float zFar;
uniform float zNear;
uniform float camHeight;
uniform float planeWidth;
out vec4 out_Color;

// Calculate depth in world coordinates
float worldPosZFromDepth(float depth) {
    // Scale Z between [0, clip-space-size]
    float depth_flipped = (1-depth) * (zFar - zNear);
    return depth_flipped + (camHeight - zFar);
}

void main(void) {
    // Get texture coordinates for 2 nearby points
    vec2 offset = vec2(1.0) / textureSize(texUnitDepth, 0).xy;
    vec2 depthTexCoord1 = depthTexCoord0 + vec2(offset.x, 0.0);
    vec2 depthTexCoord2 = depthTexCoord0 + vec2(0.0, offset.y);

    // sample depth for nearby points
    float depth_0 = texture(texUnitDepth, depthTexCoord0).x;
    float depth_1 = texture(texUnitDepth, depthTexCoord1).x;
    float depth_2 = texture(texUnitDepth, depthTexCoord2).x;

    // Transform to world coordinates, note that the depth image was taken from above, so the given coordinates are on the y-axis
    float y_0 = worldPosZFromDepth(depth_0);
    float y_1 = worldPosZFromDepth(depth_1);
    float y_2 = worldPosZFromDepth(depth_2);

    // Transform texture coordinates into world coordinates
    float x_0 = (depthTexCoord0.x * planeWidth) - planeWidth/2;
    float x_1 = (depthTexCoord1.x * planeWidth) - planeWidth/2;
    float x_2 = (depthTexCoord2.x * planeWidth) - planeWidth/2;
    float z_0 = (-1 * depthTexCoord0.y * planeWidth) + planeWidth/2;
    float z_1 = (-1 * depthTexCoord1.y * planeWidth) + planeWidth/2;
    float z_2 = (-1 * depthTexCoord2.y * planeWidth) + planeWidth/2;

    // Create normal
    vec3 pos0 = vec3(x_0, y_0, z_0);
    vec3 pos1 = vec3(x_1, y_1, z_1);
    vec3 pos2 = vec3(x_2, y_2, z_2);

    // Get normal from cross product of the two vectors
    out_Color = vec4(normalize(cross(pos1.xyz - pos0.xyz, pos2.xyz - pos0.xyz)), 1.0);
}
