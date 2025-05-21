//=============================================================================
//
// SmokeEffectRenderer���� [SmokeEffectRenderer.cpp]
// Author : 
//
//=============================================================================
#include "SmokeEffectRenderer.h"
#include "Core/TextureMgr.h"

//*****************************************************************************
// �}�N����`
//***************************************************************************** 
#define SMOKE_DIFFUSE_PATH      "data/TEXTURE/Effects/Smoke.png"

bool SmokeEffectRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // �u�����h���[�h���A���t�@�u�����h�ɐݒ�
    m_blendMode = BLEND_MODE_ALPHABLEND;

    // ���N���X�̏�����
    if (!ParticleEffectRendererBase::Initialize(device, context))
        return false;

    // �V�F�[�_�[�ǂݍ���
    if (!LoadShaders())
        return false;

    // �e�N�X�`���ǂݍ���
    if (!CreateDiffuseTex())
        return false;

    return true;
}

void SmokeEffectRenderer::Shutdown(void)
{
    ParticleEffectRendererBase::Shutdown();

	SafeRelease(&m_diffuseTextureSRV);
}

bool SmokeEffectRenderer::CreateDiffuseTex(void)
{
    m_diffuseTextureSRV = TextureMgr::get_instance().CreateTexture(SMOKE_DIFFUSE_PATH);
    if (!m_diffuseTextureSRV)
        return false;

    return true;
}

void SmokeEffectRenderer::DispatchComputeShader(void)
{
    // CS�̃o�C���h
    m_context->CSSetShader(m_updateParticlesCS.cs, nullptr, 0);

    // CS�̃f�B�X�p�b�`
    UINT threadGroupCount = GET_THREAD_GROUP_COUNT(m_maxParticles, 64); // �X���b�h�O���[�v���v�Z
    m_context->Dispatch(threadGroupCount, 1, 1);

    // Emit�pCS �͊��N���X�ɔC����iPrepare + Bind + Dispatch�j
    DispatchEmitParticlesCS();
}

void SmokeEffectRenderer::DrawParticles(void)
{
    // �X���[�N��p���\�[�X�o�C���h�i�m�C�Y�e�N�X�`���j
    m_shaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, m_diffuseTextureSRV);

    DrawIndirect();
}
