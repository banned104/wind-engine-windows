#version 460 core

// 添加必要的扩展以确保兼容性
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define INSTANCES_COUNT 4

// 从顶点着色器传入的、经过插值的数据
layout(location=0) in vec3 FragPos;
layout(location=1) in vec2 TexCoords;
layout(location=2) flat in uint InstanceID;
layout(location=3) in float layerIndex;
layout(location=4) in float heightFactor;
layout(location=5) in vec4 ColorFromVertex;

layout(location=0) out vec4 FragColor;

layout(std140, binding=0) uniform Globals {
    mat4 uProj;
    mat4 uView;
    mat4 uModel;

    float uTime;
    float uWaveAmp;
    float uWaveSpeed;
    int uPickedInstanceID;

    vec4 uColor;

    // 传递包围盒信息
    vec3 uBoundsMin;
    float deltaX;
    vec3 uBoundsMax;
    float deltaY;

    // 每个实例的独立偏移数组
    vec4 InstanceOffset[ INSTANCES_COUNT ];
};

// 材质结构体，现在包含多种纹理
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
};
uniform Material material;

uniform sampler2D fadeEdgeMaskTexture;

void main() {
    vec4 texColor;
    // 优化：预计算时间偏移，避免每片元执行mod运算
    float timeOffset = uTime * 0.1;
    vec2 moving_coords = vec2(TexCoords.x - timeOffset, TexCoords.y);

    // 优化：使用纹理数组替代分支，减少分支预测失败 --> sampler 数组的索引必须是编译时常量 不能使用 数组+layerIdx 索引的方法
    // 现代GPU分支预测和SIMD执行已经很优化 3个分支硬件处理很高效
    
    float opacity;
    if (layerIndex < 0.05) {
        texColor = texture(material.texture_diffuse1, moving_coords);
        opacity = 0.4;  // 第0层 2Edges Lines
    } else if (layerIndex - 1.0 < 0.05 )  {
        texColor = texture(material.texture_diffuse2, moving_coords);
        opacity = 0.4;  // 第1层 Middle Fog
    } else {
        texColor = texture(material.texture_diffuse3, vec2( moving_coords));
        opacity = 0.5;  // 第2层 Dotted Lines
    }

    vec3 windColor = vec3( 1. ) * 0.8;
    
    // 优化alpha裁剪判断，减少计算量
    if ((texColor.r + texColor.g + texColor.b)*0.3333 < 0.05) {
        discard;
        // FragColor = vec4(0.);
    } else {
        const float FADE_THREASHHOLD = 0.2;
        vec4 tempTexture = texture( fadeEdgeMaskTexture, TexCoords );
        float tempFactor = ( tempTexture.r + tempTexture.g + tempTexture.b )*0.33333;
        tempFactor = 1.0 - smoothstep( FADE_THREASHHOLD, 0.15, tempFactor ) * tempFactor;  // 反转
        tempFactor = mix( 0.0, tempFactor, step( FADE_THREASHHOLD, tempFactor ));           // 丢弃黑色部分
        FragColor = vec4( windColor, (texColor.r + texColor.g + texColor.b) * 0.33333 * opacity * tempFactor );
        // FragColor = vec4( windColor, tempFactor );
    }

    // 使用快速亮度计算
    // float brightness = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    // texColor.a = smoothstep(0.0, 0.7, brightness);

    // // 拾取高亮效果
    // if (uPickedInstanceID > 0 && abs(float(uPickedInstanceID) - float(InstanceID)) < 0.01) {
    // //if (abs(float(uPickedInstanceID) - float(InstanceID)) < 0.01) {
    //     texColor.r -= deltaX * 0.1;
    //     texColor.g -= deltaY * 0.1;
    //     texColor.b -= deltaX * 0.1;
    // }

}
