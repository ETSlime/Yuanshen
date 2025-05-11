//=============================================================================
//
// FireEffectRenderer処理 [FireEffectRenderer.cpp]
// Author : 
//
//=============================================================================
#include "FireEffectRenderer.h"
#include "TextureMgr.h"

bool FireEffectRenderer::s_pipelineBound = false;

FireEffectRenderer::~FireEffectRenderer()
{
    SafeRelease(&m_blendState);
    SafeRelease(&m_particleBuffer);
    SafeRelease(&m_particleSRV);
    SafeRelease(&m_particleUAV);
    SafeRelease(&m_flameTextureSRV);
    SafeRelease(&m_cbFireEffect);
}

bool FireEffectRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_device = device;
    m_context = context;

    // --- 加法ブレンドステート作成 ---
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = device->CreateBlendState(&blendDesc, &m_blendState);
    if (FAILED(hr))
        return false;

    m_flameTextureSRV = TextureMgr::get_instance().CreateTexture("data/TEXTURE/FireEffect.png");
    if (!m_flameTextureSRV)
		return false;

    bool success = true;
    success &= CreateParticleBuffer();
    success &= LoadShaders();

    return success;
}

void FireEffectRenderer::Update(void)
{
    DispatchComputeShader();
}

void FireEffectRenderer::Draw(const XMMATRIX& viewProj)
{
    XMMATRIX worldMatrix = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    DrawParticles(worldMatrix, viewProj);
}

// バッファの初期化
bool FireEffectRenderer::CreateParticleBuffer(void)
{
    HRESULT hr = S_OK;

    SimpleArray<FireParticle> initData(m_maxParticles);
    for (UINT i = 0; i < m_maxParticles; ++i)
    {
        initData[i].position = m_position;
        initData[i].velocity = XMFLOAT3(0, 1.0f + (i % 10) * 0.1f, 0);
        initData[i].life = 0.0f;
        initData[i].size = 0.0f;
        initData[i].color = XMFLOAT4(1, 0.5f, 0, 1);
    }

    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.Usage = D3D11_USAGE_DEFAULT;
    bufDesc.ByteWidth = sizeof(FireParticle) * m_maxParticles;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(FireParticle);

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = initData.data();

    hr = m_device->CreateBuffer(&bufDesc, &subData, &m_particleBuffer);
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = m_maxParticles;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    hr = m_device->CreateShaderResourceView(m_particleBuffer, &srvDesc, &m_particleSRV);
    if (FAILED(hr))
        return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = m_maxParticles;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    hr = m_device->CreateUnorderedAccessView(m_particleBuffer, &uavDesc, &m_particleUAV);
    if (FAILED(hr))
        return false;

    // 定数バッファ作成
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(CBFireEffect);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = m_device->CreateBuffer(&cbDesc, nullptr, &m_cbFireEffect);
    if (FAILED(hr))
        return false;

    return true;
}

bool FireEffectRenderer::LoadShaders(void)
{
    bool loadShaders = true;

    loadShaders &= ShaderManager::get_instance().HasComputerShader(ShaderSetID::FireEffect, ComputePassType::Update);
    if (loadShaders)
        m_computeShader = ShaderManager::get_instance().GetComputeShader(ShaderSetID::FireEffect, ComputePassType::Update);

    loadShaders &= ShaderManager::get_instance().HasShaderSet(ShaderSetID::FireEffect);
    if (loadShaders)
        m_fireShaderSet = ShaderManager::get_instance().GetShaderSet(ShaderSetID::FireEffect);

    return loadShaders;
}

// 粒子更新（CS）
void FireEffectRenderer::DispatchComputeShader(void)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(m_context->Map(m_cbFireEffect, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CBFireEffect* cb = reinterpret_cast<CBFireEffect*>(mapped.pData);
        cb->deltaTime = m_timer.GetDeltaTime();
        cb->totalTime = m_timer.GetElapsedTime();
        cb->scale = m_scale;
        m_context->Unmap(m_cbFireEffect, 0);
    }

    m_context->CSSetShader(m_computeShader.cs, nullptr, 0);
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::CS, SLOT_CB_EFFECT_PARTICLE, m_cbFireEffect);
    m_ShaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_PARTICLE, m_particleUAV);
    m_context->Dispatch((m_maxParticles + 255) / 256, 1, 1);

    // リソースの解除
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    m_ShaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_PARTICLE, nullUAV);
}

void FireEffectRenderer::SetupPipeline()
{
    if (s_pipelineBound)
        return; // すでに設定済みならスキップ

    m_context->IASetInputLayout(m_fireShaderSet.inputLayout);

    m_context->VSSetShader(m_fireShaderSet.vs, nullptr, 0);
    m_context->GSSetShader(m_fireShaderSet.gs, nullptr, 0);
    m_context->PSSetShader(m_fireShaderSet.ps, nullptr, 0);

    m_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    //float blendFactor[4] = { 0, 0, 0, 0 };
    //m_context->OMSetBlendState(m_blendState, blendFactor, 0xFFFFFFFF);
    m_renderer.SetBlendState(BLEND_MODE_ADD);

    s_pipelineBound = true; // バインド済みにマーク
}

// 描画処理
void FireEffectRenderer::DrawParticles(const XMMATRIX& worldMatrix, const XMMATRIX& viewProj)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(m_context->Map(m_cbFireEffect, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CBFireEffect* cb = reinterpret_cast<CBFireEffect*>(mapped.pData);
        cb->world = XMMatrixTranspose(worldMatrix);
        cb->viewProj = XMMatrixTranspose(viewProj);
        m_context->Unmap(m_cbFireEffect, 0);
    }

    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_EFFECT_PARTICLE, m_cbFireEffect);
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::GS, SLOT_CB_EFFECT_PARTICLE, m_cbFireEffect);

    m_ShaderResourceBinder.BindShaderResource(ShaderStage::GS, SLOT_SRV_PARTICLE, m_particleSRV);
    m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, m_flameTextureSRV);

    m_context->Draw(m_maxParticles, 0);
}