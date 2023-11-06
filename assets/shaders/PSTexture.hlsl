Texture2D tex1;

SamplerState splr;

float4 main(float2 tex : TexCoord) : SV_Target
{
    return tex1.Sample(splr, tex);
}