//=============================================================================
//
// �X���[�N�G�t�F�N�g����� [SmokeEffectRenderer.h]
// Author : 
// - ���������G���G���ȉ���`����
// - Compute Shader�Ńp�[�e�B�N���X�V�IGPU�̗͂łӂ���Ɖ��o
// - DrawIndirect�Œ���ʃX���[�N���T�N�T�N�`��
//
//=============================================================================
#pragma once
#include "ParticleEffectRendererBase.h"
#include "SimpleArray.h"


class SmokeEffectRenderer : public ParticleEffectRendererBase
{
public:

    virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    virtual void Shutdown(void) override;
    EffectType GetEffectType() const override { return EffectType::Smoke; }
    bool UseDrawIndirect() const override { return true; }

private:
    virtual void DispatchComputeShader(void) override; // �p�[�e�B�N���X�V�iDispatch�j
    virtual void DrawParticles(void) override;

    bool CreateDiffuseTex(void);

    ID3D11ShaderResourceView* m_diffuseTextureSRV = nullptr; 
};

