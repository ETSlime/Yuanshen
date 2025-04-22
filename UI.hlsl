//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************
struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

//=============================================================================
// ���_�V�F�[�_
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // �ϊ��s���K�p
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float4 texColor = g_Texture.Sample(g_SamplerState, input.uv);
    return texColor * input.color;
}