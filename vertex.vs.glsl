#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform mat4 layer1st;
uniform mat4 layer2nd;
uniform mat4 layer3rd;
uniform mat4 layer4th;
uniform mat4 layer5th;
uniform mat4 layer6th;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

void main()
{
	gl_Position = um4p * um4mv * layer1st * layer2nd * layer3rd * layer4th * layer5th * layer6th * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
}