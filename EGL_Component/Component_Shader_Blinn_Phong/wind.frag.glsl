#version 330 core
//precision mediump float;
precision highp float;
#define INSTANCES_COUNT 4
// struct InstanceOffset {
//     float deltaX;
//     float deltaY;
//     float _padding1;  // 为了对齐
//     float _padding2;  // 为了对齐
// };

// 从顶点着色器传入的、经过插值的数据
layout(location=0) in vec3 FragPos;
layout(location=1) in vec2 TexCoords;
layout(location=2) flat in uint InstanceID;
layout(location=3) in float layerIndex;       // 层索引
layout(location=4) in float heightFactor;     // 高度因子
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
    float deltaX;  // 保留用于兼容性
    vec3 uBoundsMax;
    float deltaY;  // 保留用于兼容性

    // 每个实例的独立偏移数组
    // float instanceOffsetsX[4];  // INSTANCES_COUNT = 4
    // float instanceOffsetsY[4];
    // float _padding1[4];  // 用于对齐
    // float _padding2[4];  // 用于对齐

    // struct InstanceOffset[ INSTANCES_COUNT ];
    vec4 InstanceOffset[ INSTANCES_COUNT ];
};

// 材质结构体，现在包含多种纹理
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
};
uniform Material material;

void main() {

    vec4 texColor;
    // 优化：预计算时间偏移，避免每片元执行mod运算
    float timeOffset = uTime * 0.1;
    vec2 moving_coords = vec2(TexCoords.x - timeOffset, TexCoords.y);

    // 优化：使用纹理数组替代分支，减少分支预测失败
    // 但由于当前使用单独的纹理，保持原分支但优化判断
    int layerIdx = int(layerIndex + 0.5); // 四舍五入到最近整数
    if (layerIdx == 0) {
        texColor = texture(material.texture_diffuse1, moving_coords);
    } else if (layerIdx == 1) {
        texColor = texture(material.texture_diffuse2, moving_coords);
    } else {
        texColor = texture(material.texture_diffuse3, moving_coords);
    }

    // 优化alpha裁剪判断，减少计算量
    if (texColor.r < 0.1 || texColor.g < 0.1 || texColor.b < 0.1) {
        discard;
    }

    // 使用快速亮度计算
    float brightness = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    texColor.a = smoothstep(0.0, 0.7, brightness);

    // 拾取高亮效果
    if (abs(float(uPickedInstanceID) - float(InstanceID)) < 0.01) {
        texColor.r -= deltaX * 0.1;
        texColor.g -= deltaY * 0.1;
        texColor.b -= deltaX * 0.1;
    }

    FragColor = texColor;
}
