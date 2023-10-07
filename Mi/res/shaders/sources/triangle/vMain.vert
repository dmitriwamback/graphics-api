#version 450

// coming soon

layout(binding = 0) uniform MiStandardUniformBuffer {
    mat4 projection;
    mat4 lookAt;
    float attenuation;
} u_in;


vec2 positions[3] = vec2[](
    vec2( 0.0, -0.5),
    vec2( 0.5,  0.5),
    vec2(-0.5,  0.5)
);
layout(location=0) out float attenuation;

void main() {

    attenuation = u_in.attenuation;
    //gl_Position = u_in.projection * u_in.lookAt * vert_in.transformation * [p];
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    //gl_Position = vec4(inVertex.xy, 0.0, 1.0);
}