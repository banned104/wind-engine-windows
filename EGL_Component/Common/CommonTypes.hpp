#pragma once
#include <glm/glm.hpp>

#define INSTANCES_COUNT 4
#define INSTANCE_SCALE 0.1f



// 每个实例的偏移状态
struct InstanceOffset {
    float deltaX;
    float deltaY;
    float _padding1;  // 为了对齐
    float _padding2;  // 为了对齐
};

struct InstanceData {
    glm::mat4 modelMatrix;
    glm::vec4 color;
    uint32_t instanceId;
};

// 代码指定的数据
struct InstanceData_Adjust_Static {
    float scale;
    glm::vec3 position;
};

// 用户交互时改变的数据
struct InstanceData_Adjust_Dynamic {
    float speed;
    float axis_x_delta_percent;     // 传入变化量来改变风向
    float axis_y_delta_percent;
    // 添加每个实例的累积偏移
    InstanceOffset instanceOffsets[INSTANCES_COUNT];
};


struct Globals
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;

    // 每个实例的独立偏移数组
    InstanceOffset instanceOffsets[INSTANCES_COUNT];
    // 新增字段
    int pickedInstanceID;
    float deltaX;
    float deltaY;
    float _padding;  // 对齐
    
    unsigned int id;
};