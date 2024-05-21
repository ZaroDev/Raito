#version 460 core

in vec2 TexCoord;
out vec4 OutColor;


uniform float u_Left;
uniform float u_Right;
uniform float u_Bottom;
uniform float u_Top;
uniform float u_ZNear;
uniform mat4 u_WorldMatrix;
uniform mat4 u_ViewMatrix;

float grid(vec3 worldPos, float gridStep){
    vec2 grid = fwidth(worldPos.xz);
    return max(grid.x, grid.y);
}

void main(){

    OutColor = vec4(1.0);

    // Eye direction
    vec3 eyeDirEyespace;
    eyeDirEyespace.x = u_Left + TexCoord.x * (u_Right - u_Left);
    eyeDirEyespace.y = u_Bottom + TexCoord.y * (u_Top - u_Bottom);
    eyeDirEyespace.z = -u_ZNear;

    vec3 eyeDirWorldSpace = normalize(mat3(u_WorldMatrix) * eyeDirEyespace);

    // Eye position
    vec3 eyePosEyespace = vec3(0.0);
    vec3 eyePosWorldspace = vec3(u_WorldMatrix * vec4(eyePosEyespace, 1.0));

    // Plane parameters
    vec3 planeNormalWorldspace = vec3(0.0, 1.0, 0.0);
    vec3 planePointWorldspace = vec3(0.0, 0.0, 0.0);

    // Ray-plane intersection
    float numerator = dot(planePointWorldspace - eyePosWorldspace, planeNormalWorldspace);
    float denominator = dot(eyeDirWorldSpace, planeNormalWorldspace);

    float t  = numerator / denominator;

    if(t > 0.0){
        vec3 hitWorldspace = eyePosWorldspace + eyeDirWorldSpace * t;
        OutColor = vec4(grid(hitWorldspace, 1.0));
    } else{
        gl_FragDepth = 0.0;
        discard;
    }


}