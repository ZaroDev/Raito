#version 460 core

layout(location = 0) in vec3 aPos;

layout(std430, binding = 0) readonly buffer Models {
    mat4 Model[];
};


uniform mat4 u_View;
uniform mat4 u_Projection;

flat out uint OccludeId;

void main(){
    vec3 position = vec3(((gl_VertexID & 0x4) == 0) ? -1.0 : 1.0,
                            ((gl_VertexID & 0x2) == 0) ? -1.0 : 1.0,
                            ((gl_VertexID & 0x1) == 0) ? -1.0 : 1.0);
    
    mat4 model = Model[gl_InstanceID];
    vec4 worldPos = model * vec4(aPos, 1.0);

    worldPos = (worldPos.w < 0.0) ? vec4(clamp(worldPos.xy, vec2(-0.999), vec2(0.999)), 0.0001, 1.0) : worldPos;
    OccludeId = gl_InstanceID;
   
    gl_Position = u_Projection * u_View * worldPos;
}