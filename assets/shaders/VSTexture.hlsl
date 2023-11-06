struct VSOut
{
    float2 tex : TexCoord;
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

VSOut main(float3 pos : Position, float2 tex : TexCoord)
{
    matrix transform = mul(projection, view);
    transform = mul(transform, model);
    
    VSOut vout;
    vout.pos = mul(transform, float4(pos, 1.0f));
    vout.tex = tex;
    return vout;
}