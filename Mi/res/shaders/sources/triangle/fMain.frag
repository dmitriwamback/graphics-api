#version 450

layout(location = 0) out vec4 fragp;

void main() {
    fragp = vec4(1.0, 0.0, 0.0, 1.0);
    //fragp.rgb = 1 - fragp.rgb;
}