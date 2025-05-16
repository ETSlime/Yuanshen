//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define SLOT_CB_EFFECT_UPDATE       b5
#define SLOT_CB_EFFECT_DRAW         b6
#define SLOT_CB_EFFECT_PARTICLE     b7
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
    float3 g_Acceleration;
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

cbuffer CBFireBall : register(SLOT_CB_EFFECT_PARTICLE)
{
    uint g_TilesX;
    uint g_TilesY;
    float g_InitialFrameOffset;
    float g_FrameLerpCurve;

    float g_RotationSpeed; 
    float g_ConeAngleDegree;  // �R�[���̊p�x
    float g_ConeRadius; // �R�[���̔��a
    float g_ConeLength; // �R�[���̍���
    
    float g_StartSpeedMin;
    float g_StartSpeedMax;
    float2 padding;
};


// ���q�\����
struct BillboardFlipbookParticle
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
    float frameIndex;
    float frameSpeed;
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
    float2 TexCoordA : TEXCOORD0;
    float2 TexCoordB : TEXCOORD1;
    float Blend : TEXCOORD2;
    float4 Color : COLOR0;
};

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
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
RWStructuredBuffer<BillboardFlipbookParticle> g_ParticlesUAV : register(SLOT_UAV_PARTICLE); // �������ݗp�p�[�e�B�N���f�[�^�o�b�t�@
StructuredBuffer<BillboardFlipbookParticle> g_ParticlesSRV : register(SLOT_SRV_PARTICLE); // �ǂݎ��p�p�[�e�B�N���f�[�^�o�b�t�@
StructuredBuffer<uint> g_AliveListSRV : register(SLOT_SRV_ALIVE_LIST); // �ǂݎ��p�����p�[�e�B�N�����X�g
ConsumeStructuredBuffer<uint> g_FreeListConsumeUAV : register(SLOT_UAV_FREE_LIST_CONSUME); // �ǂݎ��p�󂫃p�[�e�B�N�����X�g

float Hash11(float x)
{
    return frac(sin(x * 12.9898f) * 43758.5453f);
}

//=============================================================================
// �R���s���[�g�V�F�[�_
//=============================================================================
[numthreads(64, 1, 1)]
void UpdateCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint id = dispatchThreadID.x;
    if (id >= g_MaxParticleCount)
        return; // �͈͊O�͖���

    BillboardFlipbookParticle p = g_ParticlesUAV[id];
    
    // ==== �������Ă���ꍇ�F�X�V���� ====
    if (p.lifeRemaining > 0.0f)
    {
        // �c�����������
        p.lifeRemaining -= g_DeltaTime;
        float lifeRatio = saturate(p.lifeRemaining / max(p.life, 0.0001f)); // ������
        
        // �ړ�����
        p.position += p.velocity * g_DeltaTime; // �ʒu�X�V
        
        // ��]
        p.rotation += g_RotationSpeed * g_DeltaTime;

        // �A�j���[�V�����X�V�i����`�Ȑ�����j
        float animProgress = pow(1 - lifeRatio, g_FrameLerpCurve);
        p.frameIndex = animProgress * (float) (g_TilesX * g_TilesY - 1);
                
        // RGBA �S�̂̃O���f�[�V����
        float4 baseColor = lerp(g_endColor, g_startColor, 1.0 - lifeRatio);

        // Alpha �𖾓x�X�P�[���Ƃ��Ă����p�iAdditive�΍�j
        // �����Ŗ��邳�𒲐����鋤�ʔ{��
        float brightnessScale = baseColor.a * 0.6f;

        // ���邳�}�����RGB
        float3 finalColor = baseColor.rgb * brightnessScale;

        // �ŏI�J���[�iRGB�͗}���ς݁AA��Blend�ɔC����j
        p.color = float4(finalColor, baseColor.a);
        
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
    float randA = Hash11((float) id + g_TotalTime * 1.0f);
    float randB = Hash11((float) id + g_TotalTime * 3.14f);
    float randC = Hash11((float) id + g_TotalTime * 2.71f);
    float randD = Hash11((float) id + g_TotalTime * 0.577f);
    
    /// �R�[���p�x�����W�A���ϊ����ĕ����𐶐�
    float angleRad = radians(g_ConeAngleDegree);
    
    // �R�[���������� cos�� �͈̔� [cos(�p�x), 1.0] �Ń����_���ɕ��z
    float cosTheta = lerp(cos(angleRad), 1.0f, randB);
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = randA * 6.2831f;
    // ���˕����iY�������̃R�[���j
    float3 coneDir = float3(
    sinTheta * cos(phi),
    cosTheta,
    sinTheta * sin(phi));
    
    // ���ˈʒu�iCone�����ɉ����������Œ����ɉ����������j
    float3 position = coneDir * (randC * g_ConeLength); // 0~ConeLength�̊ԂŃ����_��

    // �p�[�e�B�N�����Đ������鏈���i�ʒu�E���x�E�T�C�Y�ȂǏ������j
    BillboardFlipbookParticle p = (BillboardFlipbookParticle) 0; // �����I�ɑS�t�B�[���h��������
    p.position = position;
    p.velocity = coneDir * lerp(g_StartSpeedMin, g_StartSpeedMax, randC) * g_Scale * 0.5f; // ���ˑ��x
    
    // ���C�t�ݒ�
    float life = lerp(g_LifeMin, g_LifeMax, randD);
    p.life = life;
    p.lifeRemaining = life;
    
    // ��]�����l�i0�`2�΁j
    p.rotation = randB * 6.2831f;
    
    // �����T�C�Y
    p.size = lerp(2.0f, 3.0f, randC) * g_Scale;
    
    // �F
    p.startColor = g_startColor;
    p.endColor = g_endColor;

    // �A�j���[�V���������t���[��
    p.frameIndex = 0.0f; // �A�j���[�V�����J�n�t���[��
    p.frameSpeed = (float) (g_TilesX * g_TilesY) / life;

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
    BillboardFlipbookParticle p = g_ParticlesSRV[input[0].ParticleID];
    
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
    
    // �A�j���[�V�����pUV�̊�bUV�i����~�E���j
    float2 uvBase[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 1.0f)
    };
    
    // �t���[���ԍ��ƕ�ԌW���iflipbook blending�p�j
    float totalFrames = g_TilesX * g_TilesY;
    float curIndex = p.frameIndex;
    int frameA = (int) floor(curIndex);
    int frameB = min(frameA + 1, (int) totalFrames - 1);
    float blend = saturate(curIndex - frameA);

    int2 tileA = int2(frameA % g_TilesX, frameA / g_TilesX);
    int2 tileB = int2(frameB % g_TilesX, frameB / g_TilesX);

    // UV�X�P�[��
    float2 tileSize = 1.0f / float2(g_TilesX, g_TilesY);
    
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
        2, 3, 0 // ��2�O�p�`
    };
    
    for (int i = 0; i < 6; ++i)
    {
        int idx = triangleIndices[i];
        float3 worldPos = mul(float4(corners[idx], 1.0f), g_World);
        float4 posWVP = mul(float4(worldPos, 1.0f), g_ViewProj);

        
        GS_OUTPUT o;
        o.PosH = posWVP;
        
        // ���݃t���[���Ǝ��t���[����UV�i�s�N�Z���V�F�[�_�[���ŕ�Ԃ���j
        o.TexCoordA = tileA * tileSize + uvBase[idx] * tileSize;
        o.TexCoordB = tileB * tileSize + uvBase[idx] * tileSize;
        //o.TexCoordA = texcoords[idx];
        o.Blend = blend;
        o.Color = p.color;
        
        stream.Append(o);

        // 3���Ƃ�RestartStrip()
        if ((i + 1) % 3 == 0)
        {
            stream.RestartStrip();
        }
    }
    
    
    //BillboardFlipbookParticle p = g_ParticlesSRV[input[0].ParticleID];
    
    //if (p.lifeRemaining <= 0.001f || p.life <= 0.001f)
    //    return;

    //float3 pos = p.position;
    //float size = p.size;

    //// �r���{�[�h�W�J�iView��1��ڂ�2��ڂ���j
    //float3 right = normalize(float3(g_View._11, g_View._21, g_View._31));
    //float3 up = normalize(float3(g_View._12, g_View._22, g_View._32));
    
    //// ��]�p�擾�i�p�[�e�B�N�����j
    //float angle = p.rotation;
    //float cosA = cos(angle);
    //float sinA = sin(angle);
    
    //// ��]��̎��x�N�g��
    //float3 rightRot = cosA * right + sinA * up;
    //float3 upRot = -sinA * right + cosA * up;

    //// ���� �� �E�� �� �E�� �� ���� ���ɓW�J
    //float3 corners[4] =
    //{
    //    pos + (-rightRot + upRot) * size,
    //    pos + (rightRot + upRot) * size,
    //    pos + (rightRot - upRot) * size,
    //    pos + (-rightRot - upRot) * size
    //};

    //float2 texcoords[4] =
    //{
    //    float2(0.0f, 0.0f),
    //    float2(1.0f, 0.0f),
    //    float2(1.0f, 1.0f),
    //    float2(0.0f, 1.0f)
    //};

    //int triangleIndices[6] =
    //{
    //    0, 1, 2, // ��1�O�p�`
    //    2, 3, 0 // ��2�O�p�`
    //};
    
    //for (int i = 0; i < 6; ++i)
    //{
    //    int idx = triangleIndices[i];
    //    GS_OUTPUT o = (GS_OUTPUT) 0;
    //    float4 worldPos = mul(float4(corners[idx], 1.0f), g_World);
    //    o.PosH = mul(worldPos, g_ViewProj);
    //    o.TexCoordA = texcoords[idx];
    //    o.Color = p.color;
    //    stream.Append(o);

    //    // 3���Ƃ�RestartStrip()
    //    if ((i + 1) % 3 == 0)
    //    {
    //        stream.RestartStrip();
    //    }
    //}
}

//=============================================================================
// �s�N�Z���V�F�[�_
//=============================================================================
float4 PS(GS_OUTPUT input) : SV_Target
{
    float4 colA = g_DiffuseTex.Sample(g_Sampler, input.TexCoordA);
    float4 colB = g_DiffuseTex.Sample(g_Sampler, input.TexCoordB);
    float4 texCol = lerp(colA, colB, input.Blend);

    texCol *= input.Color;

    return texCol;
    
        // �m�C�Y�e�N�X�`�����T���v�����O
    //float noise = g_DiffuseTex.Sample(g_Sampler, input.TexCoordA).r;

    //// �m�C�Y���������Ĕj�ꂽ�������o��
    //noise = saturate((noise - 0.4f) * 3.0f);

    //float4 finalColor = input.Color;
    //finalColor.a *= noise; // ���Ƀm�C�Y�K�p
    
    //return finalColor;
}