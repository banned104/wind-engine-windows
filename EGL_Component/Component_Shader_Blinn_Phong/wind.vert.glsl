#version 460 core

// 添加必要的扩展以确保兼容性
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define INSTANCES_COUNT 4

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;
layout(location=5) in mat4 aInstanceMatrix;
layout(location=9) in uint aInstanceId;
layout(location=10) in vec4 aColor;

// UBO: 包含 MVP 矩阵
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

// 输出到片段着色器
layout(location=0) out vec3 FragPos;
layout(location=1) out vec2 TexCoords;
layout(location=2) out uint InstanceID;
layout(location=3) out float layerIndex;
layout(location=4) out float heightFactor;
layout(location=5) out vec4 ColorFromVertex;

// uniform sampler2D vertexMovementTexture;

void main() {
    // 将顶点位置和法线变换到世界空间
    FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    
    // 根据包围盒计算高度因子 用于判断层索引
    vec3 modelPos = aPos;
    float heightRatio = ( modelPos.y - uBoundsMin.y ) / ( uBoundsMax.y - uBoundsMin.y );
    heightRatio = clamp( heightRatio, 0.0, 1.0 );

    // 根据高度确定层索引 0[0-0.33), 1[0.33-0.66), 2[0.66-1]
    layerIndex = step( 0.33, heightRatio) + step( 0.66, heightRatio );
    heightFactor = heightRatio;

    // 计算X轴位置因子 模型从X=0延伸至X正方形
    float xPositionFactor = modelPos.x / ( uBoundsMax.x - uBoundsMin.x);
    xPositionFactor = abs( xPositionFactor );
    xPositionFactor = clamp( xPositionFactor, 0.0, 1.0 );
    // 基于X轴位置确定渐变强度: x轴坐标越大  振动越强
    float distanceAmplifier = mix( 0.1, 1.0, xPositionFactor );
    
    // 为每层设置不同的Y轴振动参数
    float waveAmplitudeY, frequencyY, phaseOffsetY;

    if ( layerIndex == 0.0 ) {
        waveAmplitudeY = uWaveAmp * 0.5 * distanceAmplifier;
        frequencyY = 0.8;
        phaseOffsetY = 0.0;
    } else if ( layerIndex == 1.0 ) {
        waveAmplitudeY = uWaveAmp * 1.0 * distanceAmplifier;
        frequencyY = 1.2;
        phaseOffsetY = 0.52;
    } else {
        waveAmplitudeY = uWaveAmp * 1.5 * distanceAmplifier;
        frequencyY = 1.8;
        phaseOffsetY = 1.05;
    }
    
    // 创建复杂的Y轴波动模式
    float time = uTime * uWaveSpeed;
    
    // 主要波动：基于时间和XZ位置的组合
    float waveY_primary = sin( time * frequencyY + modelPos.x * 1.5 + modelPos.z * 0.8 + phaseOffsetY );
    
    // 次要波动：添加更自然的随机性
    float waveY_secondary = sin( time * frequencyY * 1.7 + modelPos.x * 0.5 + modelPos.z * 1.2 ) * 0.3;
    
    // 微细波动：模拟微风效果
    float waveY_detail = sin( time * frequencyY * 3.2 + modelPos.x * 2.1 + modelPos.z * 1.9 ) * 0.15;
    
    // 合成最终的Y轴偏移
    float totalWaveY = ( waveY_primary + waveY_secondary + waveY_detail ) * waveAmplitudeY;


    // 振动系数 使用纹理规划 单通道强度
    // vec2 rotateTexCoords = vec2( TexCoords.y, TexCoords.x );
    // vec4 waveColor = texture2D( vertexMovementTexture, rotateTexCoords );
    // float waveFromTexture = 1.0 - ( waveColor.r + waveColor.g + waveColor.b )/3.0;
    // totalWaveY *= waveFromTexture;
    /*
    const float  waveThresh = 0.3;      // 振动改变的位置
    const float waveSoftness = 0.02;    // 过渡的缓冲区域大小
    float mask = smoothstep( waveThresh - waveSoftness, waveThresh + waveSoftness,  )
    */
    
    // 只在Y轴方向应用振动
    FragPos.y += totalWaveY;



    ColorFromVertex = aColor;
    InstanceID = aInstanceId;
    TexCoords = aTexCoords;

    // 应用每个实例的独立偏移
    int instanceIndex = int(aInstanceId) - 1;
    if (instanceIndex >= 0 && instanceIndex < 4) {
        FragPos.x += InstanceOffset[instanceIndex].x  * TexCoords.x;
        FragPos.y -= InstanceOffset[instanceIndex].y  * TexCoords.x;
    }

    gl_Position = uProj * uView * vec4(FragPos, 1.0);
}