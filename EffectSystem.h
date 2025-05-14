//=============================================================================
//
// EffectSystem���� [EffectSystem.h]
// Author : 
//
//=============================================================================
#pragma once
#include "ParticleEffectRendererBase.h"
#include "SingletonBase.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_EFFECT_NUM 512 // �ő�G�t�F�[�N�g��
#define MAX_FIRE_EFFECT_NUM 256 // �ő�t�@�C���[�t�F�[�N�g��
#define MAX_SMOKE_EFFECT_NUM 128 // �ő�X���[�N�t�F�[�N�g��
#define MAX_SOFTBODY_EFFECT_NUM 64 // �ő�\�t�g�{�f�B�t�F�[�N�g��

//*********************************************************
// �\����
//*********************************************************
struct ParticleEffectParams
{
    EffectType type = EffectType::None;
    XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    float scale = 1.0f;
    float lifeMin = 1.0f;
    float lifeMax = 2.0f;
    float spawnRateMin = 5.0f;
    float spawnRateMax = 15.0f;
    XMFLOAT3 acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);
    UINT numParticles = MAX_PARTICLES;
    XMFLOAT4 startColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    XMFLOAT4 endColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    virtual ~ParticleEffectParams() = default; // RTTI�Adynamic_cast �̂���
};

struct FireBallEffectParams : public ParticleEffectParams
{
    UINT tilesX = 7;
    UINT tilesY = 7;
    float coneAngleDegree = 25.0f;
    float coneRadius = 0.6f;
    float coneLength = 5.0f;
    float frameLerpCurve = 1.0f;
    float rotationSpeed = 0.5f;
};

class EffectSystem : public SingletonBase<EffectSystem>
{
public:
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown(void);
    void Update(void);
    void Draw(const XMMATRIX& viewProj);

    inline bool IsParticleEffectType(EffectType type)
    {
        return (type > EffectType::Particle_Start && type < EffectType::Particle_End);
    }

    // �G�t�F�N�g����
    ParticleEffectRendererBase* SpawnParticleEffect(ParticleEffectParams params);

    // �ėp�����폜
    void RemoveEffect(IEffectRenderer* effect);
    // ���\�[�X�̉���
    void ClearAllEffectBindings(void);

private:

    SimpleArray<IEffectRenderer*> m_allEffects;       // �S�������X�g
    SimpleArray<IEffectRenderer*> m_billboardSimpleEffects;
    SimpleArray<IEffectRenderer*> m_billboardFlipbookEffects;
    SimpleArray<IEffectRenderer*> m_fireEffects;      // �t�@�C���[
    SimpleArray<IEffectRenderer*> m_smokeEffects;     // �X���[�N
    SimpleArray<IEffectRenderer*> m_softBodyEffects;  // �\�t�g�{�f�B

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();
};