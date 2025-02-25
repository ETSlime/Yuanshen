

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

// マテリアルバッファ
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    int noTexSampling;
    float Dummy[2]; //16byte境界用
};

cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// ライト用バッファ
struct LIGHT
{
    float4 Direction[5];
    float4 Position[5];
    float4 Diffuse[5];
    float4 Ambient[5];
    float4 Attenuation[5];
    int4 Flags[5];
    int Enable;
    int Dummy[3]; //16byte境界用
    matrix LightViewProj;
};

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}

struct FOG
{
    float4 Distance;
    float4 FogColor;
    int Enable;
    float Dummy[3]; //16byte境界用
};

// フォグ用バッファ
cbuffer FogBuffer : register(b5)
{
    FOG Fog;
};

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
    int fuchi;
    int fill[3];
};


cbuffer CameraBuffer : register(b7)
{
    float4 Camera;
}

cbuffer ProjViewBuffer : register(b8)
{
    LightViewProjBuffer lightBuffer;
}

cbuffer ProjViewBuffer2 : register(b9)
{
    matrix ProjView2;
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
    outPosition = mul(worldPosition, lightBuffer.ProjView[4]);
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
    outPosition = mul(worldPosition, lightBuffer.ProjView[4]);
}



//*****************************************************************************
// グローバル変数
//*****************************************************************************
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);


//=============================================================================
// ピクセルシェーダ
//=============================================================================