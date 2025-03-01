

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

struct LightViewProjBuffer
{
    matrix ProjView[5];
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

//=============================================================================
// 頂点シェーダ
//=============================================================================
void VS(in float4 inPosition : POSITION0,
						  in float4 inNormal : NORMAL0,
						  in float4 inDiffuse : COLOR0,
						  in float2 inTexCoord : TEXCOORD0,

						  out float4 outPosition : SV_POSITION)
{
    float4 worldPosition = mul(inPosition, World);
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
    worldPosition = mul(worldPosition, World); // Apply world transformation

    // Calculate shadow map coordinates for each light source
    outPosition = mul(worldPosition, lightBuffer.ProjView[0]);
}