#version 300 es
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aTexCoords;
// layout (location = 2) in vec3 aNormal;
// layout (location = 3) in mat4 instanceTransform;

layout(std140) uniform Globals {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;

    // 包围盒
    vec4 uBoundsMin;
    vec4 uBoundsMax;

    int id;
};

out vec2 TexCoords;
flat out int instanceID;

void main()
{
    // 先计算变换后的位置
    vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
    
    // 检查世界坐标是否在包围盒内
    // 如果任何坐标分量小于最小值或大于最大值，则丢弃该顶点
    if (any(lessThan(worldPos.xyz, uBoundsMin.xyz)) || any(greaterThan(worldPos.xyz, uBoundsMax.xyz))) {
        // 将顶点移到裁剪空间外，使其被丢弃
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // 超出NDC范围
        instanceID = 0; // 设置为0表示无效
        return;
    }
    
    // 正常变换
    gl_Position = projMatrix * viewMatrix * worldPos;
    instanceID = id;
}