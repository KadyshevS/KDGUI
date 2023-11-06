struct VSOut
{
    float3 color : Color;
    float4 pos : SV_Position;
};

cbuffer ModelCBuffer : register(b0)
{
    matrix model;
};

cbuffer CameraCBuffer : register(b1)
{
    matrix view;
    matrix projection;
};

VSOut main(float3 pos : Position, float3 color : Color)
{
    matrix transform = mul(projection, view);
    transform = mul(transform, model);
    
    VSOut vout;
    vout.pos = mul(transform, float4(pos, 1.0f));
    vout.color = color;
    return vout;
}