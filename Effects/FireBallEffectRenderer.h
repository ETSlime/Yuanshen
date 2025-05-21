//=============================================================================
//
// �΋��p�[�e�B�N����p�̍X�V�E�`�揈���N���X [FireBallEffectRenderer.h]
// Author : 
// �~����̕��ˁ^��]�^�����^�A�j���[�V�����Ȃǂ��p�����[�^�w��Ő��䂵�A
// Compute Shader + DrawIndirect �ɂ�鍂�����ȉ΋��G�t�F�N�g��`�悷��
//
//=============================================================================
#pragma once
#include "ParticleEffectRendererBase.h"

//*********************************************************
// �\����
//*********************************************************

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

	float startSpeedMin = 0.0f;
	float startSpeedMax = 0.0f;
	XMFLOAT2 padding = XMFLOAT2(0.0f, 0.0f); // float4�̃T�C�Y�ɍ��킹�邽�߂̃p�f�B���O
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

	float m_startSpeedMin = 0.0f;           // �����ŏ��l
	float m_startSpeedMax = 0.0f;           // �����ő�l

    UINT m_tilesX = 7;                      // �e�N�X�`���V�[�g������
    UINT m_tilesY = 7;                      // �e�N�X�`���V�[�g�c����

    ID3D11Buffer* m_cbFireBall = nullptr;
    ID3D11ShaderResourceView* m_diffuseTextureSRV = nullptr;
};