float4x4 matWorldViewProj;

void main(
    in  float4 in_position  : POSITION,
    in  float4 in_normal : NORMAL0,
    in  float4 in_texcood : TEXCOORD0,

    out float4 out_position : POSITION,
    out float4 out_diffuse : COLOR0,
    out float4 out_texcood : TEXCOORD0)
{
    // mainä÷êîÇÃäOÇ…èëÇ≠Ç∆É_ÉÅ
    float4 lightNormal = { 0.3f, 1.0f, 0.5f, 0.0f };
    out_position = mul(in_position, matWorldViewProj);

    float light_intensity = dot(in_position, lightNormal);
    out_diffuse.rgb = max(0, light_intensity)+0.3;
    out_diffuse.a = 1.0f;

    out_texcood = in_texcood;
}

