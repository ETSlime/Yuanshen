//=============================================================================
//
// FireBallEffectRenderer ���� [FireBallEffectRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "ParticleEffectRendererBase.h"

//*********************************************************
// �\����
//*********************************************************

// �΋��p�̃p�[�e�B�N���f�[�^�iStructuredBuffer�j
struct FireBallParticle
{
    XMFLOAT3 position;              // ���݈ʒu
    float lifeRemaining;            // �c������i�b�j
    float life;
    XMFLOAT3 velocity;              // ���x
    float rotation;                 // ��]�p�i���W�A���j

    float size;                     // �T�C�Y
    float frameIndex;               // �A�j���[�V�����̌��݃t���[���ifloat�A�����Ή��j
    float frameSpeed;              // 1�b������Đ����x�i��ԑΉ��p
    XMFLOAT4 color;
};

struct CBFireBall
{
    UINT tilesX = 7;
    UINT tilesY = 7;
    float initialFrameOffset = 0.0f;
    float frameLerpCurve = 1.0f;

    float rotationSpeed = 0.0f;
    float coneAngleDegree = 25.0f;
    float coneRadius = 0.6193843f;
    float coneLength = 5.0f;
};

class FireBallEffectRenderer : public ParticleEffectRendererBase
{
public:
    virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    virtual void Shutdown(void) override;
    EffectType GetEffectType() const override { return EffectType::FireBall; }
    bool UseDrawIndirect(void) const override { return true; }
    virtual void ConfigureEffect(const ParticleEffectParams& params) override;

private:
    virtual bool CreateParticleBuffer(void) override;
    virtual void DispatchComputeShader(void) override; // �p�[�e�B�N���X�V�iDispatch�j
    virtual void DrawParticles(void) override;

    bool CreateFireBallCB(void);
    void UploadFireBallCB(void);

    bool CreateDiffuseTex(void);

    float m_spawnConeAngleDegree = 25.0f;   // �R�[���p�x�i�x�j
    float m_spawnConeRadius = 0.6193843f;   // �R�[����ʂ̔��a
    float m_spawnConeLength = 5.0f;         // �R�[���̍���
    float m_rotationSpeed = 0.0f;
    float m_frameLerpCurve = 0.0f;

    UINT m_tilesX = 7;                      // �e�N�X�`���V�[�g������
    UINT m_tilesY = 7;                      // �e�N�X�`���V�[�g�c����

    ID3D11Buffer* m_cbFireBall = nullptr;
    ID3D11ShaderResourceView* m_diffuseTextureSRV = nullptr;
};