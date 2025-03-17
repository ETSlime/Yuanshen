//*****************************************************************************
// 定数バッファ
//*****************************************************************************

struct LightViewProjBuffer
{
    matrix ViewProj[5];
    int LightIndex;
    int padding[3];
};

// マトリクスバッファ
cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

cbuffer ProjViewBuffer : register(b8)
{
    LightViewProjBuffer lightViewProj;
}

struct VS_INPUT
{
    float3 Position : POSITION; // 頂点位置
    float3 Normal : NORMAL; // 法線
    float2 TexCoord : TEXCOORD; // テクスチャ座標
    float Weight : TEXCOORD1; // 風影響重み

    float3 OffsetPosition : POSITION1; // インスタンス位置オフセット
    float4 Rotation : TEXCOORD2; // クォータニオンでの回転
    float4 initialBillboardRot : TEXCOORD3; // 初期ビルボード回転角度
    float Scale : TEXCOORD4; // インスタンススケール
    float Type : TEXCOORD5; // インスタンスタイプ
};


struct VS_OUTPUT
{
    float4 Position : SV_POSITION;  // 出力位置 (クリップ空間)
    float2 TexCoord : TEXCOORD;     // テクスチャ座標
    float4 ShadowCoord : TEXCOORD1; // 影計算用座標
    float3 Normal : NORMAL;         // 出力法線
};

//*****************************************************************************
// グローバル変数
//*****************************************************************************
Texture2D DiffuseTexture : register(t0);
Texture2D g_ShadowMap[5] : register(t1);
SamplerState SampleType : register(s0); // サンプラーステート
SamplerComparisonState ShadowSampler : register(s1);
//=============================================================================
// 頂点シェーダ
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    matrix WorldViewProjection;
    WorldViewProjection = mul(World, View);
    WorldViewProjection = mul(WorldViewProjection, Projection);
    
    VS_OUTPUT output;
    
     // クォータニオンを使用して回転を適用
    float4 q = input.Rotation;
    float3 rotatedPosition = input.Position + 2.0 * cross(q.xyz, cross(q.xyz, input.Position) + q.w * input.Position);
    
     // スケールを適用
    rotatedPosition *= input.Scale;

    // インスタンス位置オフセットを適用
    float3 finalPosition = rotatedPosition + input.OffsetPosition;

    // 射影空間への変換
    output.Position = mul(float4(finalPosition, 1.0f), WorldViewProjection);

    // 影計算用のライト空間変換
    output.ShadowCoord = mul(float4(finalPosition, 1.0f), lightViewProj.ViewProj[0]);

    // 出力テクスチャ座標と法線
    output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;

    return output;
}


//*****************************************************************************
// グローバル変数
//*****************************************************************************


//=============================================================================
// ピクセルシェーダ
//=============================================================================
//float CalculateShadow(float4 shadowCoord)
//{
//    float2 shadowTexCoord = shadowCoord.xy / shadowCoord.w;
//    float depth = shadowCoord.z / shadowCoord.w;
//    if (shadowTexCoord.x < 0.0 || shadowTexCoord.x > 1.0 || shadowTexCoord.y < 0.0 || shadowTexCoord.y > 1.0)
//        return 1.0;
//    return ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowTexCoord, depth);
//}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 color = DiffuseTexture.Sample(SampleType, input.TexCoord).rgb;

    // (Lambert)
    float3 lightDir = normalize(float3(-0.5, -1.0, -0.5));
    float intensity = max(dot(input.Normal, lightDir), 0.2);

    return float4(color * intensity, 1.0);
}


//----------------------------------
// シャドウデプスパス (HLSL)
//----------------------------------

//*****************************************************************************
// 定数バッファ
//*****************************************************************************
VS_OUTPUT VSShadow(VS_INPUT input)
{
    VS_OUTPUT output;

     // クォータニオンを使用して回転を適用
    float4 q = input.Rotation;
    float3 rotatedPosition = input.Position + 2.0 * cross(q.xyz, cross(q.xyz, input.Position) + q.w * input.Position);

     // スケールを適用
    rotatedPosition *= input.Scale;
    
    // インスタンス位置オフセットを適用
    float3 finalPosition = rotatedPosition + input.OffsetPosition;

    // 影計算用のライト空間変換
    output.Position = mul(float4(finalPosition, 1.0f), lightViewProj.ViewProj[0]);

    return output;
}
//=============================================================================
// 頂点シェーダ
//=============================================================================