float4x4 matWorldViewProj;
float4 lightNormal = { 0.3f, 1.0f, 0.5f, 0.0f };
float4 ambient = { 0.3f, 0.3f, 0.3f, 1.0f };

texture texture1;
sampler textureSampler = sampler_state {
    Texture = (texture1);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

void main(
    in  float4 in_position  : POSITION,
    in  float4 in_normal : NORMAL0,
    in  float4 in_texcood : TEXCOORD0,

    out float4 out_position : POSITION,
    out float4 out_diffuse : COLOR0,
    out float4 out_texcood : TEXCOORD0)
{
    out_position = mul(in_position, matWorldViewProj);

    float light_intensity = dot(in_normal, lightNormal);
//    out_diffuse.rgb = max(0, light_intensity) + ambient;
    out_diffuse.rgb = ambient;
    out_diffuse.a = 1.0f;

    out_texcood = in_texcood;
}

