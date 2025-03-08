//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************

struct LightViewProjBuffer
{
    matrix ViewProj[5];
    int LightIndex;
    int padding[3];
};

// �}�g���N�X�o�b�t�@
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


cbuffer PerFrameBuffer : register(b12)
{
    float Time;
    float3 WindDirection;
    float WindStrength;
    float padding1;
    float2 NoiseTextureResolution;
    float2 padding2;
}


//struct VS_INPUT
//{
//    float3 Position : POSITION; // ���_�ʒu
//    float3 Normal : NORMAL;     // �@��
//    float2 TexCoord : TEXCOORD; // �e�N�X�`�����W
//    float Weight : TEXCOORD1;   // ���e���d��
//    float3 OffsetPosition : POSITION1; // �C���X�^���X�ʒu�I�t�Z�b�g
//    float Scale : TEXCOORD2;    // �C���X�^���X�X�P�[��
//};

struct VS_INPUT
{
    float3 Position : POSITION; // ���_�ʒu
    float3 Normal : NORMAL; // �@��
    float2 TexCoord : TEXCOORD; // �e�N�X�`�����W
    float Weight : TEXCOORD1; // ���e���d��
    float3 OffsetPosition : POSITION1; // �C���X�^���X�ʒu�I�t�Z�b�g
    float4 Rotation : TEXCOORD2; // �N�H�[�^�j�I���ł̉�]
    float Scale : TEXCOORD3; // �C���X�^���X�X�P�[��
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;  // �o�͈ʒu (�N���b�v���)
    float2 TexCoord : TEXCOORD;     // �e�N�X�`�����W
    float4 ShadowCoord : TEXCOORD1; // �e�v�Z�p���W
    float3 Normal : NORMAL;         // �o�͖@��
};

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D NoiseTexture : register(t0); // �m�C�Y�e�N�X�`�� (���A�j���[�V�����p)
Texture2D DiffuseTexture : register(t1);
Texture2D ShadowMap : register(t2);
SamplerState SampleType : register(s0); // �T���v���[�X�e�[�g
SamplerComparisonState ShadowSampler : register(s1);
//=============================================================================
// ���_�V�F�[�_
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    matrix WorldViewProjection;
    WorldViewProjection = mul(World, View);
    WorldViewProjection = mul(WorldViewProjection, Projection);
    
    VS_OUTPUT output;
    
     // �N�H�[�^�j�I�����g�p���ĉ�]��K�p
    float4 q = input.Rotation;
    float3 rotatedPosition = input.Position + 2.0 * cross(q.xyz, cross(q.xyz, input.Position) + q.w * input.Position);
    
     // �X�P�[����K�p
    rotatedPosition *= input.Scale;

    // ���̉e���v�Z
    //float noiseValue = NoiseTexture.Sample(SampleType, input.TexCoord).r; // �m�C�Y�擾
    int2 texCoordInt = int2(input.TexCoord * float2(256.0f, 256.0f));
    float noiseValue = NoiseTexture.Load(int3(texCoordInt, 0)).r;
    float windEffect = sin(Time * 0.002 + input.Position.x * 0.000002 + input.Position.z * 0.000002 + noiseValue * 0.001); // ���A�j���[�V�����v�Z
    float3 offset = input.Weight * WindStrength * windEffect * WindDirection * input.Scale * 25; // ���Έ�
    
    // ���_�ʒu�ɕ��̕Έ� + �C���X�^���X�ʒu�I�t�Z�b�g + �X�P�[���K�p
    //float3 finalPosition = (input.Position + offset) * input.Scale + input.OffsetPosition;
    float3 finalPosition = rotatedPosition + offset + input.OffsetPosition;

    // �ˉe��Ԃւ̕ϊ�
    output.Position = mul(float4(finalPosition, 1.0f), WorldViewProjection);

    // �e�v�Z�p�̃��C�g��ԕϊ�
    output.ShadowCoord = mul(float4(finalPosition, 1.0f), lightViewProj.ViewProj[0]);

    // �o�̓e�N�X�`�����W�Ɩ@��
    output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;

    return output;
}


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************


//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
float CalculateShadow(float4 shadowCoord)
{
    float2 shadowTexCoord = shadowCoord.xy / shadowCoord.w;
    float depth = shadowCoord.z / shadowCoord.w;
    if (shadowTexCoord.x < 0.0 || shadowTexCoord.x > 1.0 || shadowTexCoord.y < 0.0 || shadowTexCoord.y > 1.0)
        return 1.0;
    return ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowTexCoord, depth);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float4 baseColor = DiffuseTexture.Sample(SampleType, input.TexCoord);
    
    if (baseColor.a == 0.0f)
        discard;
    
    float3 lightDir = normalize(float3(0.5, -1.0, 0.5));
    float NdotL = saturate(dot(input.Normal, -lightDir));
    float shadowFactor = CalculateShadow(input.ShadowCoord);
    float3 lighting = baseColor.rgb * (0.3 + 0.7 * NdotL * shadowFactor);
    return float4(lighting, baseColor.a);
}


//----------------------------------
// �V���h�E�f�v�X�p�X (HLSL)
//----------------------------------

//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************
VS_OUTPUT VSShadow(VS_INPUT input)
{
    VS_OUTPUT output;

    //// �e�p�X�ł̓p�t�H�[�}���X����̂��ߕ��Έڂ��ȗ��\
    //float4 a = float4((input.Position * input.Scale + input.OffsetPosition), 1.f); // �ŏI���_�ʒu�v�Z

    // ���C�g��Ԃւ̕ϊ�
    output.Position = mul(float4((input.Position * input.Scale + input.OffsetPosition), 1.f), lightViewProj.ViewProj[0]);
    return output;
}
//=============================================================================
// ���_�V�F�[�_
//=============================================================================