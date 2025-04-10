//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************
cbuffer DebugCB : register(b9)
{
    float4x4 worldViewProj;
    float4 color;
};

struct VS_INPUT
{
    float3 pos : POSITION;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

//=============================================================================
// ���_�V�F�[�_
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldViewProj);
    output.col = color;
    return output;
}

//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return input.col;
}