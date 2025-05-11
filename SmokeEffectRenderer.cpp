//=============================================================================
//
// SmokeEffectRenderer���� [SmokeEffectRenderer.cpp]
// Author : 
//
//=============================================================================
#include "SmokeEffectRenderer.h"
#include "TextureMgr.h"

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
    if (!LoadShaders(EffectType::Smoke))
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

bool SmokeEffectRenderer::CreateParticleBuffer(void)
{
    HRESULT hr = S_OK;

    SimpleArray<SmokeParticle> initData(m_maxParticles);
    for (UINT i = 0; i < m_maxParticles; ++i)
    {
        initData[i].position = m_position;
        initData[i].velocity = XMFLOAT3(0.0f, 0.3f + (i % 10) * 0.05f, 0.0f);
        initData[i].size = 0.5f * m_scale;
        initData[i].life = 0.0f; // �������C�t��0
        initData[i].lifeRemaining = 0.0f; // �c�胉�C�t
        initData[i].color = XMFLOAT4(0.6f, 0.6f, 0.6f, 0.5f); // ���F
        initData[i].rotation = 0.0f; // ������]�p�x
    }

    // �o�b�t�@�쐬
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(SmokeParticle) * m_maxParticles;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(SmokeParticle);

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = initData.data();

    hr = m_device->CreateBuffer(&bufferDesc, &subData, &m_particleBuffer);
    if (FAILED(hr))
        return false;

    // SRV�쐬
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = m_maxParticles;
    hr = m_device->CreateShaderResourceView(m_particleBuffer, &srvDesc, &m_particleSRV);
    if (FAILED(hr))
        return false;

    // UAV�쐬
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = m_maxParticles;
    hr = m_device->CreateUnorderedAccessView(m_particleBuffer, &uavDesc, &m_particleUAV);
    if (FAILED(hr))
        return false;

    return true;
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
