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

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoords;
layout(location=5) in mat4 aInstanceMatrix;
layout(location=9) in uint aInstanceId;  // 实例ID
layout(location=10) in vec4 aColor;      // 实例颜色

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
    float deltaX;  // 保留用于兼容性
    vec3 uBoundsMax;
    float deltaY;  // 保留用于兼容性

    // 每个实例的独立偏移数组
    // float instanceOffsetsX[INSTANCES_COUNT];  // INSTANCES_COUNT = 4
    // float instanceOffsetsY[INSTANCES_COUNT];
    // float _padding1[INSTANCES_COUNT];  // 用于对齐
    // float _padding2[INSTANCES_COUNT];  // 用于对齐

    // struct InstanceOffset[ INSTANCES_COUNT ];
    vec4 InstanceOffset[ INSTANCES_COUNT ];
};



// 输出到片段着色器
layout(location=0) out vec3 FragPos;   // 片段在世界空间中的位置
layout(location=1) out vec2 TexCoords; // 纹理坐标
layout(location=2) out uint InstanceID;
layout(location=3) out float layerIndex;       // 层索引
layout(location=4) out float heightFactor;     // 高度因子
layout(location=5) out vec4 ColorFromVertex;

uniform sampler2D vertexMovementTexture;


void main() {
    // 将顶点位置和法线变换到世界空间
    FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    // 注意：法线需要用逆转置矩阵变换以避免非等比缩放导致的问题
    // 为简化，这里我们假设 uModel 没有非等比缩放
    // Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
    

    /*------------------------------------------ 基本波动 ------------------------------------------*/
    // 根据包围盒计算高度因子 用于判断层索引
    vec3 modelPos = aPos;
    float heightRatio = ( modelPos.y - uBoundsMin.y ) / ( uBoundsMax.y - uBoundsMin.y );
    heightRatio = clamp( heightRatio, 0.0, 1.0 );

    // 根据高度确定层索引 0[0-0.33), 1[0.33-0.66), 2[0.66-1]
    layerIndex = step( 0.33, heightRatio) + step( 0.66, heightRatio );
    heightFactor = heightRatio;

    // 计算X轴位置因子 模型从X=0延伸至X正方形
    float xPositionFactor = modelPos.x / ( uBoundsMax.x - uBoundsMin.x);
    xPositionFactor = clamp( xPositionFactor, 0.0, 1.0 );

    // 基于X轴位置确定渐变强度: x轴坐标越大  振动越强
    float distanceAmplifier = mix( 0.1, 1.0, xPositionFactor );     // 从 10% ~ 100% 的振动强度
    
    // 为每层设置不同的Y轴振动参数
    float waveAmplitudeY, frequencyY, phaseOffsetY;

    if ( layerIndex == 0.0 ) {      // 模型三层的最底层 - 稳重振动
        waveAmplitudeY = uWaveAmp * 0.5 * distanceAmplifier;        // 小振幅
        frequencyY = 0.8;                                           // 低频率
        phaseOffsetY = 0.0;                                         // 无相位偏移
        
    } else if ( layerIndex == 1.0 ) { // 中层 - 中等振动
        waveAmplitudeY = uWaveAmp * 1.0 * distanceAmplifier;        // 中等振幅
        frequencyY = 1.2;                                           // 中等频率
        phaseOffsetY = 0.52;                                        // π/6 相位偏移
        
    } else {                        // 顶层 - 活跃振动
        waveAmplitudeY = uWaveAmp * 1.5 * distanceAmplifier;        // 大振幅
        frequencyY = 1.8;                                           // 高频率
        phaseOffsetY = 1.05;                                        // π/3 相位偏移
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
    
    // 只在Y轴方向应用振动
    FragPos.y += totalWaveY;

/*------------------------------------------ 整体随触控运动 ------------------------------------------*/
    ColorFromVertex = aColor;   // 传递颜色
    InstanceID = aInstanceId;
    TexCoords = aTexCoords;

    // if ( int(aInstanceId) == uPickedInstanceID  ) {
    //     // 选中实例时，根据累积的deltaX和deltaY进行偏移
    //     FragPos.x += deltaX;
    //     FragPos.y -= deltaY;  // 注意Y轴方向，触摸屏坐标系与OpenGL坐标系相反
    // }

    // 应用每个实例的独立偏移
    int instanceIndex = int(aInstanceId) - 1;  // 实例ID从1开始，数组索引从0开始
    if (instanceIndex >= 0 && instanceIndex < 4) {
        FragPos.x += InstanceOffset[instanceIndex].x  * TexCoords.x;    // 朝向屏幕的方向是X轴正方向 X坐标值越大移动距离越大 也就实现了出风口不动
        FragPos.y -= InstanceOffset[instanceIndex].y  * TexCoords.x;  
    }

    gl_Position = uProj * uView * vec4(FragPos, 1.0);
}