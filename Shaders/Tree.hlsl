//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************

#define LIGHT_MAX_NUM           5
#define LIGHT_MAX_NUM_CASCADE   4
#define SCREEN_WIDTH            1920
#define SHADOWMAP_SIZE          2048
#define ALPHA_THRESHOLD         0.5f // �J�b�g�A�E�g��臒l�i����ȉ��͉e�ɂȂ�Ȃ��j

struct WorldMatrixBuffer
{
    matrix world;
    matrix invWorld;
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

struct CascadeData
{
    float4x4 lightViewProj;
    float4x4 lightView;
    float4x4 lightProj;
    float splitDepth;
    float3 padding; // 16�o�C�g�̃A���C�����g���ێ����邽��
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

cbuffer CB_CascadeDataMainPass : register(b6)
{
    CascadeData g_CascadeArray[4];
    float4 g_CascadeSplits;
}

cbuffer CB_CascadeData : register(b8)
{
    CascadeData g_CascadeData;
};

cbuffer CameraPosBuffer : register(b7)
{
    float4 CameraPos;
}

struct VS_INPUT
{
    float3 Position : POSITION; // ���_�ʒu
    float3 Normal : NORMAL; // �@��
    float2 TexCoord : TEXCOORD0; // �e�N�X�`�����W
    float Weight : TEXCOORD1; // ���e���d��
    float3 Tangent : TANGENT;

    float3 OffsetPosition : POSITION1; // �C���X�^���X�ʒu�I�t�Z�b�g
    float4 Rotation : TEXCOORD2; // �N�H�[�^�j�I���ł̉�]
    float4 initialBillboardRot : TEXCOORD3; // �����r���{�[�h��]�p�x
    float Scale : TEXCOORD4; // �C���X�^���X�X�P�[��
    float Type : TEXCOORD5; // �C���X�^���X�^�C�v
};


struct VS_OUTPUT
{
    float4 Position : SV_POSITION;  // �o�͈ʒu (�N���b�v���)
    float2 TexCoord : TEXCOORD0; // �e�N�X�`�����W
    float4 ShadowCoord[LIGHT_MAX_NUM_CASCADE] : TEXCOORD1; // �e�v�Z�p���W
    float3 Normal : NORMAL;         // �o�͖@��
    float3 Tangent : TANGENT;
    float4 WorldPos : POSITION1;
};

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D g_diffuseTexture : register(t0);
Texture2D g_ShadowMap[LIGHT_MAX_NUM] : register(t1);
Texture2D g_NormalMap : register(t9);
Texture2D g_BumpMap : register(t10);
Texture2D g_OpacityMap : register(t11); // alpha�p�}�X�N�e�N�X�`��
Texture2D g_ReflectMap : register(t12);
Texture2D g_TranslucencyMap : register(t13);
SamplerState g_SamplerState : register(s0); // �T���v���[�X�e�[�g
SamplerComparisonState g_ShadowSampler : register(s1);
SamplerState g_SamplerStateOpacity : register(s2);

float3 RotateByQuaternion(float3 v, float4 q)
{
    // q = (x, y, z, w)
    float3 u = q.xyz;
    float s = q.w;

    return 2.0f * dot(u, v) * u
         + (s * s - dot(u, u)) * v
         + 2.0f * s * cross(u, v);
}


//=============================================================================
// ���_�V�F�[�_
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    matrix WorldViewProjection;
    WorldViewProjection = mul(WorldBuffer.world, View);
    WorldViewProjection = mul(WorldViewProjection, Projection);
    
    VS_OUTPUT output;
    
     // �N�H�[�^�j�I�����g�p���ĉ�]��K�p
    //float4 q = input.Rotation;
    float3 rotatedPosition = RotateByQuaternion(input.Position, input.Rotation);//input.Position + 2.0 * cross(q.xyz, cross(q.xyz, input.Position) + q.w * input.Position);
    
     // �X�P�[����K�p
    rotatedPosition *= input.Scale;

    // �C���X�^���X�ʒu�I�t�Z�b�g��K�p
    float3 finalPosition = rotatedPosition + input.OffsetPosition;
    output.WorldPos = float4(finalPosition, 1.0f);
    // �ˉe��Ԃւ̕ϊ�
    output.Position = mul(output.WorldPos, WorldViewProjection);

    // �e�v�Z�p�̃��C�g��ԕϊ�
    for (int i = 0; i < LIGHT_MAX_NUM_CASCADE; ++i)
    {
        output.ShadowCoord[i] = mul(float4(finalPosition, 1.0f), g_CascadeArray[i].lightViewProj);
    }

    // �o�̓e�N�X�`�����W�Ɩ@��
    output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;

    return output;
}

float GetOpacity(float2 uv)
{
    return g_OpacityMap.Sample(g_SamplerStateOpacity, uv).r;
}

float3 GetBumpNormal(float2 uv, float3 normal, float3 tangent)
{
    float3 bumpNormal = g_BumpMap.Sample(g_SamplerState, uv).rgb;
    
    bumpNormal = normalize(bumpNormal * 2.0 - 1.0);

    float3 bitangent = cross(normal, tangent);

    float3x3 TBN = float3x3(tangent, bitangent, normal);
    
    return normalize(mul(bumpNormal, TBN));
}

//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    //float3 color = DiffuseTexture.Sample(SampleType, input.TexCoord).rgb;

    //// (Lambert)
    //float3 lightDir = normalize(float3(-0.5, -1.0, -0.5));
    //float intensity = max(dot(input.Normal, lightDir), 0.2);

    //return float4(color * intensity, 1.0);
    
    float4 color;
    
    color = g_diffuseTexture.Sample(g_SamplerState, input.TexCoord);
    
    float alpha = 1.0;
    if (Material.opacityMapSampling)
    {
        float alpha = GetOpacity(input.TexCoord);
        clip(alpha - ALPHA_THRESHOLD);
    }

    if (Light.Enable == 0)
    {
        color = color * Material.Diffuse;
    }
    else
    {
        float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);


        for (int i = 0; i < LIGHT_MAX_NUM; i++)
        {
            float3 lightDir;
            float light;

            if (Light.Flags[i].OnOff == 1)
            {
                float4 ambient = color * Material.Diffuse * Light.Ambient[i];
                
                float3 normal = input.Normal.xyz;
                if (Material.bumpMapSampling)
                    normal = GetBumpNormal(input.TexCoord, input.Normal.xyz, input.Tangent.xyz);
                
                //if (Material.reflectMapSampling)
                //{
                //    float3 reflectVector = reflect(-input.ViewDir, normal);
                //    float3 reflectionColor = g_ReflectMap.Sample(g_SamplerState, reflectVector).rgb;
                //}
                
                if (Light.Flags[i].Type == 1)
                {
                    lightDir = normalize(Light.Direction[i].xyz);
                    light = saturate(dot(lightDir, normal));
                    float backlightFactor = saturate(dot(-lightDir, normal));

                    light = 0.5 - 0.5 * light;
                    tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
                }
                else if (Light.Flags[i].Type == 2)
                {
                    lightDir = normalize(Light.Position[i].xyz - normal);
                    light = dot(lightDir, normal);

                    tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

                    float distance = length(input.WorldPos - Light.Position[i]);

                    float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
                    tempColor *= att;
                }
                else
                {
                    tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
                }
                float shadowFactor = 1.0f;
                if (Light.Flags[i].Type == 1)
                {
                    float2 shadowTexCoord = float2(input.ShadowCoord[i].x, -input.ShadowCoord[i].y) / input.ShadowCoord[i].w * 0.5f + 0.5f;
                    float currentDepth = input.ShadowCoord[i].z / input.ShadowCoord[i].w;
                    currentDepth -= 0.005f;

                    int kernelSize = 1;
                    float2 shadowMapDimensions = float2(SHADOWMAP_SIZE, SHADOWMAP_SIZE);
                    float shadow = 0.0;
                    float2 texelSize = 1.0 / shadowMapDimensions;
                    float totalWeight = 0.0;
                    for (int x = -kernelSize; x <= kernelSize; x++)
                    {
                        for (int y = -kernelSize; y <= kernelSize; y++)
                        {
                            float weight = exp(-(x * x + y * y) / (2.0 * kernelSize * kernelSize)); // gaussian weight
                            shadow += g_ShadowMap[i].SampleCmpLevelZero(g_ShadowSampler, shadowTexCoord + float2(x, y) * texelSize, currentDepth) * weight;
                            totalWeight += weight;
                        }
                    }
                    shadowFactor = shadow / totalWeight;
                }

                tempColor *= shadowFactor;
				
                outColor += tempColor + ambient;
            }
        }
        
        color = outColor;
        if (Material.opacityMapSampling)
            color.a = alpha;
        else
            color.a = Material.Diffuse.a;
    }
        
    return color;
}



//----------------------------------
// �V���h�E�f�v�X�p�X (HLSL)
//----------------------------------

//=============================================================================
// ���_�V�F�[�_
//=============================================================================
VS_OUTPUT VSShadow(VS_INPUT input)
{
    VS_OUTPUT output;

     // �N�H�[�^�j�I�����g�p���ĉ�]��K�p
    //float4 q = input.Rotation;
    float3 rotatedPosition = RotateByQuaternion(input.Position, input.Rotation);//input.Position + 2.0 * cross(q.xyz, cross(q.xyz, input.Position) + q.w * input.Position);

     // �X�P�[����K�p
    rotatedPosition *= input.Scale;
    
    // �C���X�^���X�ʒu�I�t�Z�b�g��K�p
    float3 finalPosition = rotatedPosition + input.OffsetPosition;

    // �e�v�Z�p�̃��C�g��ԕϊ�
    output.Position = mul(float4(finalPosition, 1.0f), g_CascadeData.lightViewProj);
    // �e�N�X�`�����W
    output.TexCoord = input.TexCoord; 
    return output;
}

float4 PSShadowAlphaTest(VS_OUTPUT input) : SV_Target
{
    // UV���W�i�K�v�ł����VS�œn���j
    float2 uv = input.TexCoord;

    // �e�N�X�`������A���t�@�l�擾
    float alpha = GetOpacity(uv);

    // �A���t�@�e�X�g�F���ߕ����͔j��
    clip(alpha - ALPHA_THRESHOLD);

    // �[�x�̂ݏo�́i�ʏ�͒l�͎g��Ȃ��j
    return float4(0, 0, 0, 0);
}