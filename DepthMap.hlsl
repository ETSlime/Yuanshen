

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

struct WorldMatrixBuffer
{
    matrix world;
    matrix invWorld;
};

// マテリアルバッファ
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    int noTexSampling;
    int lightmapSampling;
    int normalMapSampling;
    int bumpMapSampling;
    int opacityMapSampling;
    int reflectMapSampling;
    int translucencyMapSampling;
    //float Dummy[1]; //16byte境界用
};

struct LightViewProjBuffer
{
    matrix ProjView[5];
    int LightIndex;
    int padding[3];
};

// マトリクスバッファ
cbuffer WorldBuffer : register(b0)
{
    WorldMatrixBuffer WorldBuffer;
}

cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

cbuffer ProjViewBuffer : register(b8)
{
    LightViewProjBuffer lightBuffer;
}

cbuffer BoneTransformBuffer : register(b11)
{
    matrix BoneTransforms[256];
}

struct SkinnedMeshVertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
    float4 weights : BLENDWEIGHT;
    float4 boneIndices : BLENDINDICES;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float4 worldPos : POSITION;
    float4 shadowCoord[5] : TEXCOORD1;
};

//*****************************************************************************
// グローバル変数
//*****************************************************************************
Texture2D g_OpacityMap : register(t11);
SamplerState g_SamplerStateOpacity : register(s2);


float GetOpacity(float2 uv)
{
    return g_OpacityMap.Sample(g_SamplerStateOpacity, uv).r;
}

//=============================================================================
// 頂点シェーダ
//=============================================================================
void VS(in float4 inPosition : POSITION0,
						  in float4 inNormal : NORMAL0,
						  in float4 inDiffuse : COLOR0,
						  in float2 inTexCoord : TEXCOORD0,

						  out float4 outPosition : SV_POSITION)
{    
    float4 worldPosition = mul(inPosition, WorldBuffer.world);
    outPosition = mul(worldPosition, lightBuffer.ProjView[0]);
}

void SkinnedMeshVertexShaderPolygon(SkinnedMeshVertexInputType input,
                                              out float4 outPosition : SV_POSITION)
{    
    // Initialize the bone transform matrix to zero
    matrix boneTransform = float4x4(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    );

    // Calculate the bone transformation matrix based on weights and bone indices
    for (int i = 0; i < 4; ++i)
    {
        if (input.weights[i] > 0)
        {
            boneTransform += input.weights[i] * BoneTransforms[input.boneIndices[i]];
        }
    }

    // Transform the vertex position from model space to world space
    float4 worldPosition = mul(input.position, boneTransform);
    worldPosition = mul(worldPosition, WorldBuffer.world); // Apply world transformation

    // Calculate shadow map coordinates for each light source
    outPosition = mul(worldPosition, lightBuffer.ProjView[0]);
}


//=============================================================================
// ピクセルシェーダ
//=============================================================================
