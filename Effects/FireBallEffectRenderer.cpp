//=============================================================================
//
// FireBallEffectRenderer ���� [FireBallEffectRenderer.cpp]
// Author : 
//
//=============================================================================
#include "FireBallEffectRenderer.h"
#include "Core/TextureMgr.h"
#include "Effects/EffectSystem.h"

//*****************************************************************************
// �}�N����`
//***************************************************************************** 
#define FIREBALL_DIFFUSE_PATH      "data/TEXTURE/Effects/Fireball.png"

bool FireBallEffectRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // �u�����h���[�h���A���t�@�u�����h�ɐݒ�
    m_blendMode = BLEND_MODE_ADD;

    // ���N���X�̏�����
    if (!ParticleEffectRendererBase::Initialize(device, context))
        return false;

    // FireBallEffect�p�V�F�[�_�[�̓ǂݍ���
    if (!LoadShaders())
        return false;

    // �f�B�t���[�Y�e�N�X�`���̓ǂݍ���
    if (!CreateDiffuseTex())
        return false;
    
    if (!CreateFireBallCB())
        return false;

    return true;
}

void FireBallEffectRenderer::Shutdown(void)
{
    ParticleEffectRendererBase::Shutdown();

    SafeRelease(&m_diffuseTextureSRV);
    SafeRelease(&m_cbFireBall);
}

void FireBallEffectRenderer::ConfigureEffect(const ParticleEffectParams& params)
{
    ParticleEffectRendererBase::ConfigureEffect(params);

    const FireBallEffectParams* fbParams = dynamic_cast<const FireBallEffectParams*>(&params);
    if (!fbParams)
        return;

    m_tilesX = fbParams->tilesX;
    m_tilesY = fbParams->tilesY;
    m_spawnConeAngleDegree = fbParams->coneAngleDegree;
    m_spawnConeRadius = fbParams->coneRadius;
    m_spawnConeLength = fbParams->coneLength;
    m_rotationSpeed = fbParams->rotationSpeed;
    m_frameLerpCurve = fbParams->frameLerpCurve;
	m_startSpeedMin = fbParams->startSpeedMin;
    m_startSpeedMax = fbParams->startSpeedMax;
}

void FireBallEffectRenderer::DispatchComputeShader(void)
{
    // FireBall��pCB�̃A�b�v���[�h
    UploadFireBallCB();

    // Update�p��ComputeShader���o�C���h
    m_context->CSSetShader(m_updateParticlesCS.cs, nullptr, 0);

    // �X���b�h�O���[�v�����v�Z�i��: 64�X���b�h/�O���[�v�j
    UINT threadGroupCount = GET_THREAD_GROUP_COUNT(m_maxParticles, 64);
    m_context->Dispatch(threadGroupCount, 1, 1);

    // Emit�����͊��N���X�ɔC����
    DispatchEmitParticlesCS();
}

void FireBallEffectRenderer::DrawParticles(void)
{
    // FireBall�e�N�X�`�����s�N�Z���V�F�[�_�[�Ƀo�C���h
    m_shaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, m_diffuseTextureSRV);

    m_shaderResourceBinder.BindConstantBuffer(ShaderStage::GS, SLOT_CB_EFFECT_PARTICLE, m_cbFireBall);

    // �Ԑڕ`��iDrawIndirect�j�ŕ`��
    DrawIndirect();
}

bool FireBallEffectRenderer::CreateFireBallCB(void)
{
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(CBFireBall);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;

    HRESULT hr = m_device->CreateBuffer(&cbDesc, nullptr, &m_cbFireBall);
    if (FAILED(hr))
        return false;

    return true;
}

void FireBallEffectRenderer::UploadFireBallCB(void)
{
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(m_cbFireBall, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CBFireBall* cb = reinterpret_cast<CBFireBall*>(mapped.pData);
        cb->tilesX = m_tilesX;
        cb->tilesY = m_tilesY;
        cb->initialFrameOffset = 0.0f;
        cb->frameLerpCurve = m_frameLerpCurve;
        cb->rotationSpeed = m_rotationSpeed;
        cb->coneAngleDegree = m_spawnConeAngleDegree;
        cb->coneLength = m_spawnConeLength;
        cb->coneRadius = m_spawnConeRadius;
		cb->startSpeedMin = m_startSpeedMin;
		cb->startSpeedMax = m_startSpeedMax;
        m_context->Unmap(m_cbFireBall, 0);
    }

    m_shaderResourceBinder.BindConstantBuffer(ShaderStage::CS, SLOT_CB_EFFECT_PARTICLE, m_cbFireBall);
}

bool FireBallEffectRenderer::CreateDiffuseTex(void)
{
    m_diffuseTextureSRV = TextureMgr::get_instance().CreateTexture(FIREBALL_DIFFUSE_PATH);
    if (!m_diffuseTextureSRV)
        return false;

    return true;
}