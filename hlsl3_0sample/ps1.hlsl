
texture texture1: register(s0);
sampler2D myTexture : register(s0); // s0 はサンプラーインデックス
sampler textureSampler = sampler_state {
    Texture = (texture1);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

//void main(
//    in float4 ScreenColor : COLOR0,
//    in float2 in_texcood : TEXCOORD0,
//
//    out float4 outColor : COLOR)
//{
//    float4 workColor = (float4)0;
//    //workColor = tex2D(textureSampler, in_texcood);
//    outColor = (ScreenColor * workColor);
//    outColor = ambient;
//}
float4 main(float2 texCoord : TEXCOORD) : SV_Target
{
    return tex2D(myTexture, texCoord);
}
