#version 300 es
precision mediump float;
layout( location = 0 ) out uint out_id;

flat in int instanceID;

void main()
{
    // 如果instanceID为0，说明该片段应该被丢弃
    if (instanceID == 0) {
        discard;
    }
    
    out_id = uint(instanceID);
}