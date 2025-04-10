

//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************
#define LIGHT_MAX_NUM   5

struct WorldMatrixBuffer
{
    matrix world;
    matrix invWorld;
};

// �}�e���A���o�b�t�@
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
    //float Dummy[1]; //16byte���E�p
};

struct LightViewProjBuffer
{
    matrix ProjView[5];
    int LightIndex;
    int padding[3];
};

struct LightFlags
{
    int Type;
    int OnOff;
    int Dummy1;
    int Dummy2;
};

// ���C�g�p�o�b�t�@
struct LIGHT
{
    float4 Direction[LIGHT_MAX_NUM];
    float4 Position[LIGHT_MAX_NUM];
    float4 Diffuse[LIGHT_MAX_NUM];
    float4 Ambient[LIGHT_MAX_NUM];
    float4 Attenuation[LIGHT_MAX_NUM];
    LightFlags Flags[LIGHT_MAX_NUM];
    matrix LightViewProj[LIGHT_MAX_NUM];
    int Enable;
    int3 Dummy; //16byte���E�p
};

// �}�g���N�X�o�b�t�@
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

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}

//cbuffer ProjViewBuffer : register(b8)
//{
//    LightViewProjBuffer lightBuffer;
//}

cbuffer BoneTransformBuffer : register(b11)
{
    matrix BoneTransforms[256];
}

struct SkinnedMeshVertexInputType
{
    float3 position : POSITION;
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
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D g_OpacityMap : register(t11);
SamplerState g_SamplerStateOpacity : register(s2);


float GetOpacity(float2 uv)
{
    return g_OpacityMap.Sample(g_SamplerStateOpacity, uv).r;
}

//=============================================================================
// ���_�V�F�[�_
//=============================================================================
void VS(in float3 inPosition : POSITION0,
						  in float4 inNormal : NORMAL0,
						  in float4 inDiffuse : COLOR0,
						  in float2 inTexCoord : TEXCOORD0,

						  out float4 outPosition : SV_POSITION)
{    
    float4 worldPosition = mul(float4(inPosition, 1.0f), WorldBuffer.world);
    outPosition = mul(worldPosition, Light.LightViewProj[0]);
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
    float4 worldPosition = mul(float4(input.position, 1.0f), boneTransform);
    worldPosition = mul(worldPosition, WorldBuffer.world); // Apply world transformation

    // Calculate shadow map coordinates for each light source
    outPosition = mul(worldPosition, Light.LightViewProj[0]);
}


//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
float DummyPS() : SV_Depth
{
    return 1.0f;
}