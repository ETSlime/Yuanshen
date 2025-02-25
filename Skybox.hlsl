//*****************************************************************************
// 定数バッファ
//*****************************************************************************
cbuffer SkyBoxBuffer : register(b12)
{
    matrix view;
    matrix projection;
    float blendFactor;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    int faceIndex : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    int faceIndex : TEXCOORD1;
};


//=============================================================================
// 頂点シェーダ
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 pos = float4(input.position, 1.0f);
    output.position = mul(pos, view);
    output.position = mul(output.position, projection);

    output.texCoord = input.texCoord;
    return output;
}


//*****************************************************************************
// グローバル変数
//*****************************************************************************
Texture2D skyboxDayTextures[6] : register(t0);
Texture2D skyboxNightTextures[6] : register(t6);
SamplerState samplerState : register(s2);

//=============================================================================
// ピクセルシェーダ
//=============================================================================
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    int faceIndex = input.faceIndex;
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    switch (input.faceIndex)
    {
        case 0:
            color = skyboxDayTextures[0].Sample(samplerState, input.texCoord);
            break;
        case 1:
            color = skyboxDayTextures[1].Sample(samplerState, input.texCoord);
            break;
        case 2:
            color = skyboxDayTextures[2].Sample(samplerState, input.texCoord);
            break;
        case 3:
            color = skyboxDayTextures[3].Sample(samplerState, input.texCoord);
            break;
        case 4:
            color = skyboxDayTextures[4].Sample(samplerState, input.texCoord);
            break;
        case 5:
            color = skyboxDayTextures[5].Sample(samplerState, input.texCoord);
            break;
    }
    
    return color;

}