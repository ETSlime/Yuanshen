//=============================================================================
//
// FireBallEffectRenderer 処理 [FireBallEffectRenderer.cpp]
// Author : 
//
//=============================================================================
#include "FireBallEffectRenderer.h"
#include "TextureMgr.h"
#include "EffectSystem.h"

//*****************************************************************************
// マクロ定義
//***************************************************************************** 
#define FIREBALL_DIFFUSE_PATH      "data/TEXTURE/Effects/Fireball.png"

bool FireBallEffectRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    // ブレンドモードをアルファブレンドに設定
    m_blendMode = BLEND_MODE_ALPHABLEND;

    // 基底クラスの初期化
    if (!ParticleEffectRendererBase::Initialize(device, context))
        return false;

    // FireBallEffect用シェーダーの読み込み
    if (!LoadShaders(EffectType::FireBall))
        return false;

    // ディフューズテクスチャの読み込み
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
}

bool FireBallEffectRenderer::CreateParticleBuffer(void)
{
    HRESULT hr = S_OK;

    // 初期パーティクル配列を生成（すべてゼロ寿命）
    SimpleArray<FireBallParticle> initData(m_maxParticles);
    for (UINT i = 0; i < m_maxParticles; ++i)
    {
        initData[i].position = m_position;
        initData[i].velocity = XMFLOAT3(0, 0, 0);
        initData[i].life = 0.0f; // 初期ライフは0
        initData[i].lifeRemaining = 0.0f; // 残りライフ
        initData[i].rotation = 0.0f;
        initData[i].size = 1.0f;
        initData[i].frameIndex = 0.0f;
        initData[i].frameSpeed = 0.0f;
    }

    // 構造化バッファの設定
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(FireBallParticle) * m_maxParticles;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FireBallParticle);

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = initData.data();

    hr = m_device->CreateBuffer(&bufferDesc, &subData, &m_particleBuffer);
    if (FAILED(hr)) 
        return false;

    // SRV（Shader Resource View）の作成
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = m_maxParticles;

    hr = m_device->CreateShaderResourceView(m_particleBuffer, &srvDesc, &m_particleSRV);
    if (FAILED(hr)) return false;

    // UAV（Unordered Access View）の作成
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = m_maxParticles;

    hr = m_device->CreateUnorderedAccessView(m_particleBuffer, &uavDesc, &m_particleUAV);
    if (FAILED(hr)) 
        return false;

    return true;
}

void FireBallEffectRenderer::DispatchComputeShader(void)
{
    // FireBall専用CBのアップロード
    UploadFireBallCB();

    // Update用のComputeShaderをバインド
    m_context->CSSetShader(m_updateParticlesCS.cs, nullptr, 0);

    // スレッドグループ数を計算（例: 64スレッド/グループ）
    UINT threadGroupCount = GET_THREAD_GROUP_COUNT(m_maxParticles, 64);
    m_context->Dispatch(threadGroupCount, 1, 1);

    // Emit処理は基底クラスに任せる
    DispatchEmitParticlesCS();
}

void FireBallEffectRenderer::DrawParticles(void)
{
    // FireBallテクスチャをピクセルシェーダーにバインド
    m_shaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, m_diffuseTextureSRV);

    // 間接描画（DrawIndirect）で描画
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