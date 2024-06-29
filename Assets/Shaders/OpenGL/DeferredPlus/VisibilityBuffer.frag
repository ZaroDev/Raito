#version 460 core
layout(early_fragment_tests) in;

layout(std430, binding = 1) writeonly buffer VisibilityBuffer{
    uint visBuffer[];
};

in flat uint OccludeId;

void main() {
    visBuffer[OccludeId] = 1;
}