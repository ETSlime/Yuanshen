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
static const uint TILE_X = 7;
static const uint TILE_Y = 7;
static const uint TOTAL_FRAME = TILE_X * TILE_Y;
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
    float g_ConeAngleDegree;
    float g_ConeRadius;
    float g_ConeLength;
};


// ���q�\����
struct FireBallParticle
{
    float3 position;
    float lifeRemaining;
    float life;
    float3 velocity;
    float rotation;
    float size;
    float frameIndex;
    float frameSpeed;
    float4 color;
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
RWStructuredBuffer<FireBallParticle> g_ParticlesUAV : register(SLOT_UAV_PARTICLE); // �������ݗp�p�[�e�B�N���f�[�^�o�b�t�@
StructuredBuffer<FireBallParticle> g_ParticlesSRV : register(SLOT_SRV_PARTICLE); // �ǂݎ��p�p�[�e�B�N���f�[�^�o�b�t�@
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

    FireBallParticle p = g_ParticlesUAV[id];
    
    // ==== �������Ă���ꍇ�F�X�V���� ====
    if (p.lifeRemaining > 0.0f)
    {
        // �c�����������
        p.lifeRemaining -= g_DeltaTime;

        // ���x�X�V
        p.velocity += g_Acceleration * g_DeltaTime;

        // �ʒu�X�V
        p.position += p.velocity * g_DeltaTime;
        
        // ��]
        p.rotation += g_RotationSpeed * g_DeltaTime;

        // �A�j���[�V�����X�V�i����`�Ȑ�����j
        float lifeRatio = saturate(1.0f - p.lifeRemaining / p.life);
        float animProgress = pow(lifeRatio, g_FrameLerpCurve);
        p.frameIndex = animProgress * (float) (g_TilesX * g_TilesY - 1);
        
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
    uint seed = slotID * 1664525 + 1013904223 + asuint(g_TotalTime * 1000.0f);
    float randA = frac(sin(seed * 12.9898) * 43758.5453);
    float randB = frac(sin(seed * 78.233) * 24634.6345);
    float randC = frac(sin(seed * 11.31) * 19493.2242);
    float randD = frac(sin(seed * 91.123) * 51932.7753);
    
    // �R�[���p�x�����W�A���ϊ����ĕ����𐶐�
    float angleRad = radians(g_ConeAngleDegree);
    float theta = randA * 6.2831f;
    float y = cos(randB * angleRad);
    float r = sqrt(1.0 - y * y);
    float3 dir = normalize(float3(r * cos(theta), y, r * sin(theta)));
    
    // ���ˈʒu�I�t�Z�b�g�i�R�[���̒����Ɋ�Â��j
    float3 position = dir * (randC * g_ConeLength);

    // �p�[�e�B�N�����Đ������鏈���i�ʒu�E���x�E�T�C�Y�ȂǏ������j
    FireBallParticle p = (FireBallParticle) 0; // �����I�ɑS�t�B�[���h��������
    p.position = position;
    p.velocity = dir * 2.0f;
    
    // ���C�t�ݒ�
    float life = lerp(g_LifeMin, g_LifeMax, randD);
    p.life = life;
    p.lifeRemaining = life;
    
    // ��]�����l�i0�`2�΁j
    p.rotation = randB * 6.2831f;
    
    // �����T�C�Y
    p.size = lerp(2.0f, 3.0f, randC) * g_Scale;

    // �A�j���[�V���������t���[��
    p.frameIndex = 0.0f;
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
    FireBallParticle p = g_ParticlesSRV[input[0].ParticleID];
    
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
    float totalFrames = TILE_X * TILE_Y;
    float curIndex = p.frameIndex;
    int frameA = (int) floor(curIndex);
    int frameB = min(frameA + 1, (int) totalFrames - 1);
    float blend = saturate(curIndex - frameA);

    int2 tileA = int2(frameA % TILE_X, frameA / TILE_X);
    int2 tileB = int2(frameB % TILE_X, frameB / TILE_X);

    // UV�X�P�[��
    float2 tileSize = 1.0f / float2(TILE_X, TILE_Y);
    
     // �����䗦 
    float lifeRatio = saturate(p.lifeRemaining / p.life);
    
    // �ȈՃO���f�[�V�����i�� �� �� �� ���Łj
    float3 col = lerp(float3(1.0, 1.0, 0.3), float3(1.2, 0.5, 0.0), 1.0 - lifeRatio);
    float alpha = pow(lifeRatio, 1.2); // �t�F�[�h�A�E�g

    float4 finalColor = float4(col, alpha);

    int triangleIndices[6] =
    {
        0, 1, 2, // ��1�O�p�`
        2, 3, 0  // ��2�O�p�`
    };
    
    for (int i = 0; i < 6; ++i)
    {
        int idx = triangleIndices[i];
        float3 worldPos = corners[idx];
        float4 posWVP = mul(float4(worldPos, 1.0f), g_ViewProj);

        
        GS_OUTPUT o;
        o.PosH = posWVP;
        
        // ���݃t���[���Ǝ��t���[����UV�i�s�N�Z���V�F�[�_�[���ŕ�Ԃ���j
        o.TexCoordA = tileA * tileSize + uvBase[idx] * tileSize;
        o.TexCoordB = tileB * tileSize + uvBase[idx] * tileSize;
        o.Blend = blend;
        o.Color = finalColor;
        
        stream.Append(o);

        // 3���Ƃ�RestartStrip()
        if ((i + 1) % 3 == 0)
        {
            stream.RestartStrip();
        }
    }
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
}