sampler2D mySampler : register(s0);

float4 main(float2 texCoord : TEXCOORD,
            float4 color : COLOR0) : SV_Target
{
    float4 texColor = tex2D(mySampler, texCoord);
    float4 finalColor = texColor * color;
    return finalColor;
}

