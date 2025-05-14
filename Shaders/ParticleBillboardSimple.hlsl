//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define SLOT_CB_EFFECT_UPDATE       b5
#define SLOT_CB_EFFECT_DRAW         b6
#define SLOT_UAV_PARTICLE           u0
#define SLOT_UAV_ALIVE_LIST         u1
#define SLOT_UAV_FREE_LIST          u2
#define SLOT_UAV_FREE_LIST_CONSUME  u3
#define SLOT_SRV_PARTICLE           t6
#define SLOT_SRV_ALIVE_LIST         t7
#define SLOT_TEX_DIFFUSE            t0
#define SLOT_SAMPLER_DEFAULT        s0
#define SLOT_CB_VIEW_MATRIX         b1
//*****************************************************************************
// �萔�o�b�t�@
//*****************************************************************************
cbuffer CBParticleUpdate : register(SLOT_CB_EFFECT_UPDATE)
{
    float3 g_Acceleration; // �����x�i��Ffloat3(0, 1.5f, 0)�j
    float g_Scale; // �T�C�Y�X�P�[��
    
    float g_DeltaTime; // �o�ߎ���
    float g_TotalTime; // �S�̌o�ߎ���
    float g_LifeMin; // �ŏ����C�t
    float g_LifeMax; // �ő僉�C�t
    float g_SpawnRateMin; // ���b���ː�
    float g_SpawnRateMax; // ���b���ː�
    
    uint g_MaxParticleCount; // �ő�p�[�e�B�N����
    uint g_ParticlesToEmitThisFrame; // ���̃t���[���ɔ��˂���p�[�e�B�N����
    
    float4 g_startColor;
    float4 g_endColor;
};

cbuffer CBParticleDraw : register(SLOT_CB_EFFECT_DRAW)
{
    matrix g_World;
    matrix g_ViewProj;
};

cbuffer ViewBuffer : register(SLOT_CB_VIEW_MATRIX)
{
    matrix g_View;
}

// ���q�\����
struct BillboardSimpleParticle
{
    float3 position;
    float3 velocity;
    float size;
    float life;
    float lifeRemaining;
    float rotation;
    float4 color;
    float4 startColor;
    float4 endColor;
};

struct VS_INPUT
{
    uint instanceID : SV_InstanceID; // DrawInstancedIndirect �œn�����
};

// VS �� �P��ParticleID��n��
struct VS_OUTPUT
{
    uint ParticleID : PARTICLE_ID;
};

struct GS_INPUT
{
    uint ParticleID : PARTICLE_ID;
};

// GS �� Billboard�W�J
struct GS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR0;
};

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR0;
};

float Rand(float seed)
{
    return frac(sin(seed * 12.9898) * 43758.5453);
}

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
Texture2D g_DiffuseTex : register(SLOT_TEX_DIFFUSE); // �m�C�Y�e�N�X�`��
SamplerState g_Sampler : register(SLOT_SAMPLER_DEFAULT); // �T���v���[�X�e�[�g

// �o�b�t�@
AppendStructuredBuffer<uint> g_AliveListUAV : register(SLOT_UAV_ALIVE_LIST); // �������ݗp�����p�[�e�B�N�����X�g
AppendStructuredBuffer<uint> g_FreeListUAV : register(SLOT_UAV_FREE_LIST); // �������ݗp�󂫃p�[�e�B�N�����X�g
RWStructuredBuffer<BillboardSimpleParticle> g_ParticlesUAV : register(SLOT_UAV_PARTICLE); // �������ݗp�p�[�e�B�N���f�[�^�o�b�t�@
StructuredBuffer<BillboardSimpleParticle> g_ParticlesSRV : register(SLOT_SRV_PARTICLE); // �ǂݎ��p�p�[�e�B�N���f�[�^�o�b�t�@
StructuredBuffer<uint> g_AliveListSRV : register(SLOT_SRV_ALIVE_LIST); // �ǂݎ��p�����p�[�e�B�N�����X�g
ConsumeStructuredBuffer<uint> g_FreeListConsumeUAV : register(SLOT_UAV_FREE_LIST_CONSUME); // �ǂݎ��p�󂫃p�[�e�B�N�����X�g

//=============================================================================
// �R���s���[�g�V�F�[�_
//=============================================================================
[numthreads(64, 1, 1)]
void UpdateCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint id = dispatchThreadID.x;
    if (id >= g_MaxParticleCount)
        return; // �͈͊O�͖���

    BillboardSimpleParticle p = g_ParticlesUAV[id];
    
    // ==== �������Ă���ꍇ�F�X�V���� ====
    if (p.lifeRemaining > 0.0f)
    {
        // �c�����������
        p.lifeRemaining -= g_DeltaTime;

        // ���x�X�V
        p.velocity += g_Acceleration * g_DeltaTime;

        // �ʒu�X�V
        p.position += p.velocity * g_DeltaTime;

        // �T�C�Y����
        p.size += g_DeltaTime * g_Scale * 0.1f;
        
        // ���ɂ����œ����ɂȂ�
        float lifeRatio = saturate(p.lifeRemaining / p.life);
        p.color = p.startColor * lifeRatio;
        
        // �����߂�
        g_ParticlesUAV[id] = p;
        
        // �����ȗ��q�� AliveList �ɓo�^       
        g_AliveListUAV.Append(id);
    }
    else
    {
        // ���S�����ꍇ�� FreeList �ɓo�^
        g_FreeListUAV.Append(id);
    }
}

[numthreads(64, 1, 1)]
void EmitCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint id = dispatchThreadID.x;
    if (id >= g_ParticlesToEmitThisFrame)
        return;

    // �󂫃X���b�g���擾
    uint slotID = g_FreeListConsumeUAV.Consume();

    // ���������iTotalTime�Ŏ��Ԉˑ����j
    float seed = frac(sin(id * 17.3f + g_TotalTime) * 43758.5453f);
    float randA = frac(sin(id * 31.7f + g_TotalTime) * 12345.67f);
    float randB = frac(sin(id * 61.3f + g_TotalTime) * 67891.23f);
    float randAngle = randB * 6.2831f;
    float radius = randA * 0.2f * g_Scale;

    // �p�[�e�B�N�����Đ������鏈���i�ʒu�E���x�E�T�C�Y�ȂǏ������j
    BillboardSimpleParticle p = (BillboardSimpleParticle) 0; // �����I�ɑS�t�B�[���h��������
    
    // �����ʒu�i���S���ӂɏ����U�炷�j
    p.position = float3(cos(randAngle), 0, sin(randAngle)) * radius;

    // �������x�i����� + ����j    
    float3 horiz = float3(cos(randAngle), 0.0f, sin(randAngle)) * 0.5f;
    p.velocity = float3(0, 1.5f, 0) + horiz;
    
    // �����T�C�Y
    p.size = lerp(0.3f, 0.5f, randB) * g_Scale;
    
    // �F�i�ÓI�A�K�v�Ȃ�fade��PS�ŏ����j
    p.startColor = g_startColor;
    
    // ���C�t�ݒ�
    p.life = lerp(g_LifeMin, g_LifeMax, randA);
    p.lifeRemaining = p.life;
    
    // [0 ~ 2��] �̉�]�p
    p.rotation = randAngle;

    // �����߂�
    g_ParticlesUAV[slotID] = p;
}

//=============================================================================
// ���_�V�F�[�_�F�p�[�e�B�N��ID���W�I���g���V�F�[�_�[�ɓn������
//=============================================================================
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // �C���X�^���XID���g���āA���ۂ̗��q�C���f�b�N�X���擾
    output.ParticleID = g_AliveListSRV[input.instanceID];

    return output;
}

//=============================================================================
// �W�I���g���V�F�[�_�F���q���r���{�[�h�����Ďl�p�`�ɓW�J
//=============================================================================
[maxvertexcount(6)]
void GS(point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> stream)
{
    BillboardSimpleParticle p = g_ParticlesSRV[input[0].ParticleID];
    
    if (p.lifeRemaining <= 0.001f || p.life <= 0.001f)
        return;

    float3 pos = p.position;
    float size = p.size;

    // �r���{�[�h�W�J�iView��1��ڂ�2��ڂ���j
    float3 right = normalize(float3(g_View._11, g_View._21, g_View._31));
    float3 up = normalize(float3(g_View._12, g_View._22, g_View._32));
    
    // ��]�p�擾�i�p�[�e�B�N�����j
    float angle = p.rotation;
    float cosA = cos(angle);
    float sinA = sin(angle);
    
    // ��]��̎��x�N�g��
    float3 rightRot = cosA * right + sinA * up;
    float3 upRot = -sinA * right + cosA * up;

    // ���� �� �E�� �� �E�� �� ���� ���ɓW�J
    float3 corners[4] =
    {
        pos + (-rightRot + upRot) * size,
        pos + (rightRot + upRot) * size,
        pos + (rightRot - upRot) * size,
        pos + (-rightRot - upRot) * size
    };

    float2 texcoords[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
    };

    int triangleIndices[6] =
    {
        0, 1, 2, // ��1�O�p�`
        2, 3, 0  // ��2�O�p�`
    };
    
    for (int i = 0; i < 6; ++i)
    {
        int idx = triangleIndices[i];
        GS_OUTPUT o;
        float4 worldPos = mul(float4(corners[idx], 1.0f), g_World);
        o.PosH = mul(worldPos, g_ViewProj);
        o.TexCoord = texcoords[idx];
        o.Color = p.color;
        stream.Append(o);

        // 3���Ƃ�RestartStrip()
        if ((i + 1) % 3 == 0)
        {
            stream.RestartStrip();
        }
    }
}

//=============================================================================
// �s�N�Z���V�F�[�_�F�m�C�Y�Ŕj�ꂽ����\��
//=============================================================================
float4 PS(PS_INPUT input) : SV_Target
{
    // �m�C�Y�e�N�X�`�����T���v�����O
    float noise = g_DiffuseTex.Sample(g_Sampler, input.TexCoord).r;

    // �m�C�Y���������Ĕj�ꂽ�������o��
    noise = saturate((noise - 0.4f) * 3.0f);

    float4 finalColor = input.Color;
    finalColor.a *= noise; // ���Ƀm�C�Y�K�p

    return finalColor;
}