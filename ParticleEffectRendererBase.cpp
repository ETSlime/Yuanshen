//=============================================================================
//
// ParticleEffectRendererBase処理 [ParticleEffectRendererBase.cpp]
// Author : 
//
//=============================================================================
#include "ParticleEffectRendererBase.h"
#include "EffectSystem.h"
#include "ParticleStructs.h"
//*****************************************************************************
// グローバル変数
//*****************************************************************************
HashMap<uint64_t, bool, HashUInt64, EqualUInt64> ParticleEffectRendererBase::s_pipelineBoundMap(
    MAX_EFFECT_NUM,
    HashUInt64(),
    EqualUInt64()
);
ParticleEffectRendererBase::CreateBufferFunc 
ParticleEffectRendererBase::s_bufferFactoryTable[static_cast<UINT>(ParticleShaderGroup::Count)] = {};

bool ParticleEffectRendererBase::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_device = device;
    m_context = context;

    if (!m_device || !m_context)
        return false;

    EffectType type = GetEffectType();
    m_shaderGroup = GetShaderGroupForEffect(type);
    m_computeGroup = GetComputeGroupForEffect(type);

    RegisterarticleBufferFactories();

    // パーティクルバッファ作成
    if (!CreateParticleBuffer())
        return false;

    // 定数バッファ作成
    if (!CreateConstantBuffer())
        return false;

    if (UseDrawIndirect())
    {
        // 引数バッファ作成
        if (!CreateDrawArgsBuffer())
            return false;

        // アクティブなパーティクルリストバッファ作成
        if (!CreateAliveListBuffer())
			return false;

        // フリーリストバッファ作成
		if (!CreateFreeListBuffer())
			return false;

        // フリーリストを初期化
        InitializeFreeList();
    }

#ifdef _DEBUG
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(DrawArgs);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;

    device->CreateBuffer(&desc, nullptr, &m_drawArgsStaging);
#endif // _DEBUG

    return true;
}

void ParticleEffectRendererBase::Shutdown(void)
{
    SafeRelease(&m_cbParticleUpdate);
    SafeRelease(&m_cbParticleDraw);
    SafeRelease(&m_particleBuffer);
    SafeRelease(&m_particleSRV);
    SafeRelease(&m_particleUAV);
    SafeRelease(&m_cbParticleEffect);
    SafeRelease(&m_drawArgsBuffer);
    SafeRelease(&m_aliveListBuffer);
    SafeRelease(&m_aliveListUAV);
    SafeRelease(&m_aliveListSRV);
    SafeRelease(&m_drawArgsStaging);
    SafeRelease(&m_freeListBuffer);
    SafeRelease(&m_freeListUAV);
    SafeRelease(&m_freeListConsumeUAV);
}

void ParticleEffectRendererBase::Update(void)
{
#ifdef _DEBUG
    LoadShaders();
#endif // DEBUG

    // 旧SRVバインドを解除
    UnbindAllShaderSRVs();

    // 共通バッファをバインド
    BindCommonComputeResources();

    // 全UAVを初期化（Append構造体は必ずClear）
    ClearInternalUAVs();

    // パーティクル発生器更新
    UpdateEmitter();

    // パーティクル更新用CB更新
    UpdateParticleUpdateCB();

    // 子クラスのパーティクル更新
    DispatchComputeShader();

    // DrawIndirect 使用時のみ、描画引数を更新
    if (UseDrawIndirect())
        UpdateDrawArgsInstanceCount();

    // リソースの解除（次のパイプラインステージに備える）
    UnbindComputeUAVs();
}

void ParticleEffectRendererBase::Draw(const XMMATRIX& viewProj)
{
    XMMATRIX worldMatrix = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    BindParticleDrawResources(worldMatrix, viewProj);
    DrawParticles();
}

void ParticleEffectRendererBase::DrawIndirect(void)
{
    // DrawInstancedIndirectを使用する場合の描画関数
    if (!m_drawArgsBuffer) return;
    m_context->DrawInstancedIndirect(m_drawArgsBuffer, 0);
}

bool ParticleEffectRendererBase::CreateConstantBuffer(void)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC desc = {};
    // 更新用: acceleration, scale, deltaTime, totalTime, lifeMin/Max, spawnRateMin/Max, padding
    desc.ByteWidth = sizeof(CBParticleUpdate);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&desc, nullptr, &m_cbParticleUpdate);
    if (FAILED(hr))
        return false;

    desc.ByteWidth = sizeof(XMMATRIX) * 2; // 描画用 2つ行列: world, viewProj
    hr = m_device->CreateBuffer(&desc, nullptr, &m_cbParticleDraw);
    if (FAILED(hr))
        return false;

    return true;
}

bool ParticleEffectRendererBase::CreateDrawArgsBuffer(void)
{
    HRESULT hr = S_OK;

    DrawArgs args = {};
    args.VertexCountPerInstance = 4;  // 1つのインスタンスあたりの頂点数（四角形の4頂点）
    args.InstanceCount = 0;          // インスタンス数（初期値は0）
    args.StartVertexLocation = 0;
    args.StartInstanceLocation = 0;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = &args;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(DrawArgs);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = 0; // バインドフラグは設定しない、CopyStructureCount用
    desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS; // DrawInstancedIndirect用


    hr = m_device->CreateBuffer(&desc, &init, &m_drawArgsBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

bool ParticleEffectRendererBase::CreateAliveListBuffer(void)
{
    HRESULT hr = S_OK;

    // 構造体1個あたりのサイズ（UINT = 4バイト）
    const UINT elementSize = sizeof(UINT);

    // 最大粒子数分の要素数を確保（アクティブリストは最大 m_maxParticles 個）
    const UINT elementCount = m_maxParticles;

    // バッファの作成（構造化バッファ）
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = elementSize * elementCount;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = elementSize;

    hr = m_device->CreateBuffer(&desc, nullptr, &m_aliveListBuffer);
    if (FAILED(hr))
        return false;

    // UAV の作成（Compute Shader 側で index 書き込み用）
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = elementCount;
    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; // AppendStructuredBuffer 用のAppendフラグ

    hr = m_device->CreateUnorderedAccessView(m_aliveListBuffer, &uavDesc, &m_aliveListUAV);
    if (FAILED(hr))
        return false;

    // SRV の作成（VS 側で index 読み出し用）
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = elementCount;

    hr = m_device->CreateShaderResourceView(m_aliveListBuffer, &srvDesc, &m_aliveListSRV);
    if (FAILED(hr))
        return false;

    return true;
}

bool ParticleEffectRendererBase::CreateFreeListBuffer(void)
{
    HRESULT hr = S_OK;

    // 構造体1個あたりのサイズ（UINT = 4バイト）
    const UINT elementSize = sizeof(UINT);

    // 最大粒子数分の要素数を確保（死亡粒子のIDを最大 m_maxParticles 個保持可能）
    const UINT elementCount = m_maxParticles;

    // バッファの作成（構造化バッファ）
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = elementSize * elementCount;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = elementSize;

    hr = m_device->CreateBuffer(&desc, nullptr, &m_freeListBuffer);
    if (FAILED(hr))
        return false;

    // UAV の作成（AppendStructuredBuffer 用のAppendフラグを指定）
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = elementCount;
    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; // Appendバッファのフラグ

    hr = m_device->CreateUnorderedAccessView(m_freeListBuffer, &uavDesc, &m_freeListUAV);
    if (FAILED(hr))
        return false;

    // consumeUAV の作成（ConsumeStructuredBuffer として読み出し用）
    D3D11_UNORDERED_ACCESS_VIEW_DESC consumUavDesc = {};
    consumUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    consumUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    consumUavDesc.Buffer.FirstElement = 0;
    consumUavDesc.Buffer.NumElements = elementCount;
    consumUavDesc.Buffer.Flags = 0; // Consumeバッファのフラグ

    hr = m_device->CreateUnorderedAccessView(m_freeListBuffer, &consumUavDesc, &m_freeListConsumeUAV);
    if (FAILED(hr))
        return false;

    return true;
}

void ParticleEffectRendererBase::InitializeFreeList(void)
{
    SimpleArray<UINT> ids(m_maxParticles);
    for (UINT i = 0; i < m_maxParticles; ++i)
        ids[i] = i;

    m_context->UpdateSubresource(m_freeListBuffer, 0, nullptr, ids.data(), 0, 0);
}

bool ParticleEffectRendererBase::CreateParticleBuffer(void)
{
    const int groupIndex = static_cast<int>(m_shaderGroup);
    if (s_bufferFactoryTable[groupIndex])
    {
        s_bufferFactoryTable[groupIndex](this);
        return m_particleBuffer != nullptr;
    }
    return false;
}

void ParticleEffectRendererBase::DrawParticles(void)
{
    if (UseDrawIndirect())
    {
        DrawIndirect();
    }
    else
    {
        m_context->Draw(m_maxParticles, 0);
    }
}

void ParticleEffectRendererBase::DispatchEmitParticlesCS(void)
{
#ifdef _DEBUG

    ID3D11InfoQueue* infoQueue;
    if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue)))
    {
        D3D11_MESSAGE_ID denyIds[] =
        {
            D3D11_MESSAGE_ID_DEVICE_UNORDEREDACCESSVIEW_DIMENSION_MISMATCH
        };

        D3D11_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;

        infoQueue->PushStorageFilter(&filter);
    }
#endif // _DEBUG

    if (m_particlesToEmitThisFrame > 0 && m_emitParticlesCS.cs)
    {
        // Emit用CSの準備
        PrepareForEmitCS();

        // Emit用CSのバインド
        m_context->CSSetShader(m_emitParticlesCS.cs, nullptr, 0);

        UINT threadGroupCount = GET_THREAD_GROUP_COUNT(m_particlesToEmitThisFrame, 64); // スレッドグループ数計算
        m_context->Dispatch(threadGroupCount, 1, 1);
    }

#ifdef _DEBUG
    m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue);

    UINT64 messageCount = infoQueue->GetNumStoredMessages();
    for (UINT64 i = 0; i < messageCount; ++i)
    {
        SIZE_T messageLength = 0;
        infoQueue->GetMessage(i, nullptr, &messageLength);

        SimpleArray<char> messageData(static_cast<UINT>(messageLength));
        D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(messageData.data());

        if (SUCCEEDED(infoQueue->GetMessage(i, message, &messageLength)))
        {
            std::cout << "[D3D11 Warning] ID = " << message->ID << ", Description: " << message->pDescription << std::endl;
        }
    }

    if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue)))
    {
        infoQueue->PopStorageFilter();
    }
#endif
}

void ParticleEffectRendererBase::BindParticleDrawResources(const XMMATRIX& worldMatrix, const XMMATRIX& viewProj)
{
    // パーティクル描画用CB更新
    UpdateParticleDrawCB(worldMatrix, viewProj);

    m_shaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_EFFECT_DRAW, m_cbParticleDraw);
    m_shaderResourceBinder.BindConstantBuffer(ShaderStage::GS, SLOT_CB_EFFECT_DRAW, m_cbParticleDraw);

    m_shaderResourceBinder.BindShaderResource(ShaderStage::GS, SLOT_SRV_PARTICLE, m_particleSRV);

    if (UseDrawIndirect() && m_aliveListSRV)
    {
        m_shaderResourceBinder.BindShaderResource(ShaderStage::VS, SLOT_SRV_ALIVE_LIST, m_aliveListSRV);
    }

    m_renderer.BindViewBuffer(ShaderStage::GS);
}

void ParticleEffectRendererBase::BindCommonComputeResources(void)
{
    m_shaderResourceBinder.BindConstantBuffer(ShaderStage::CS, SLOT_CB_EFFECT_UPDATE, m_cbParticleUpdate);
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_PARTICLE, m_particleUAV);

    if (UseDrawIndirect())
    {
        // アクティブなパーティクルIDリスト（RWStructuredBuffer<uint> g_AliveList : register(SLOT_UAV_ALIVE_LIST)）
        if (m_aliveListUAV)
        {
            m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_ALIVE_LIST, m_aliveListUAV);
        }

        // フリーリスト（RWStructuredBuffer<uint> g_FreeList : register(SLOT_UAV_FREE_LIST)）
        if (m_freeListUAV)
		{
			m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST, m_freeListUAV);
		}
    }

}

void ParticleEffectRendererBase::UnbindComputeUAVs(void)
{
    ID3D11UnorderedAccessView* nullUAV = nullptr;

    // パーティクルのUAVは常に使用
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_PARTICLE, nullUAV);

    // AliveList/FreeList のUAVも常にクリア
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_ALIVE_LIST, nullUAV);
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST, nullUAV);
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST_CONSUME, nullUAV);
}

void ParticleEffectRendererBase::UnbindAllShaderSRVs(void)
{
    ID3D11ShaderResourceView* nullSRV = nullptr;

    // VSのリストSRVを解除
    m_shaderResourceBinder.BindShaderResource(ShaderStage::VS, SLOT_SRV_ALIVE_LIST, nullSRV);

    // GSのパーティクルSRVを解除
    m_shaderResourceBinder.BindShaderResource(ShaderStage::GS, SLOT_SRV_PARTICLE, nullSRV);

}

void ParticleEffectRendererBase::PrepareForEmitCS(void)
{
    // EmitCS 用のリソース準備
    // Append 用 UAV（freeListUAV）を アンバインド
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST, nullptr);

    // Consume 用 UAV を バインド（SRV ではなく UAV である点に注意）
    if (m_freeListConsumeUAV)
    {
        // ConsumeStructuredBuffer に対応そのため SRV ではなく UAV を通してバインドする必要があります
        m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST_CONSUME, m_freeListConsumeUAV);
    }
}

void ParticleEffectRendererBase::ClearInternalUAVs(void)
{
    UINT clear[4] = { 0 };

    // アクティブリストをクリア
    if (m_aliveListUAV)
        m_context->ClearUnorderedAccessViewUint(m_aliveListUAV, clear);

    // フリーリストをクリア
    if (m_freeListUAV)
        m_context->ClearUnorderedAccessViewUint(m_freeListUAV, clear);
}

void ParticleEffectRendererBase::ConfigureEffect(const ParticleEffectParams& params)
{
    m_scale = params.scale;
    m_position = params.position;
    m_acceleration = params.acceleration;
    m_maxParticles = params.numParticles;
    m_startColor = params.startColor;
    m_endColor = params.endColor;
    SetSpawnRateRange(params.spawnRateMin, params.spawnRateMax);
    SetLifeRange(params.lifeMin, params.lifeMax);
}

void ParticleEffectRendererBase::SetupPipeline(void)
{
    // すでに設定済みならスキップ
    if (s_pipelineBoundMap[TO_UINT64(m_shaderGroup)])
        return;

    m_context->IASetInputLayout(m_shaderSet.inputLayout);

    m_context->VSSetShader(m_shaderSet.vs, nullptr, 0);
    m_context->GSSetShader(m_shaderSet.gs, nullptr, 0);
    m_context->PSSetShader(m_shaderSet.ps, nullptr, 0);

    m_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    m_renderer.SetBlendState(m_blendMode);

    s_pipelineBoundMap[TO_UINT64(m_shaderGroup)] = true;
}

bool ParticleEffectRendererBase::LoadShaders(void)
{
    bool loadShaders = true;

    // UpdateCSチェック＆取得
    loadShaders &= ShaderManager::get_instance().HasComputerShader(m_computeGroup, ComputePassType::Update);
    if (loadShaders)
        m_updateParticlesCS = ShaderManager::get_instance().GetComputeShader(m_computeGroup, ComputePassType::Update);

    if (UseDrawIndirect())
	{
		// EmitCSチェック＆取得
		loadShaders &= ShaderManager::get_instance().HasComputerShader(m_computeGroup, ComputePassType::Emit);
		if (loadShaders)
			m_emitParticlesCS = ShaderManager::get_instance().GetComputeShader(m_computeGroup, ComputePassType::Emit);
	}

    // VS/GS/PSセットチェック＆取得
    loadShaders &= ShaderManager::get_instance().HasShaderSet(m_shaderGroup);
    if (loadShaders)
        m_shaderSet = ShaderManager::get_instance().GetShaderSet(m_shaderGroup);

    return loadShaders;
}

bool ParticleEffectRendererBase::CreateBillboardSimpleBuffer(ParticleEffectRendererBase* self)
{
    HRESULT hr = S_OK;

    SimpleArray<BillboardSimpleParticle> initData(self->m_maxParticles);
    for (UINT i = 0; i < self->m_maxParticles; ++i)
    {
        initData[i].position = self->m_position;
        initData[i].velocity = XMFLOAT3(0.0f, 0.3f + (i % 10) * 0.05f, 0.0f);
        initData[i].size = 0.5f * self->m_scale;
        initData[i].life = 0.0f; // 初期ライフは0
        initData[i].lifeRemaining = 0.0f; // 残りライフ
        initData[i].rotation = 0.0f; // 初期回転角度
        initData[i].color = self->m_startColor;
        initData[i].startColor = self->m_startColor;
        initData[i].endColor = self->m_endColor;
    }

    // バッファ作成
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(BillboardSimpleParticle) * self->m_maxParticles;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(BillboardSimpleParticle);

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = initData.data();

    hr = self->m_device->CreateBuffer(&bufferDesc, &subData, &self->m_particleBuffer);
    if (FAILED(hr))
        return false;

    // SRV作成
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = self->m_maxParticles;
    hr = self->m_device->CreateShaderResourceView(self->m_particleBuffer, &srvDesc, &self->m_particleSRV);
    if (FAILED(hr))
        return false;

    // UAV作成
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = self->m_maxParticles;
    hr = self->m_device->CreateUnorderedAccessView(self->m_particleBuffer, &uavDesc, &self->m_particleUAV);
    if (FAILED(hr))
        return false;

    return true;
}

bool ParticleEffectRendererBase::CreateBillboardFlipbookBuffer(ParticleEffectRendererBase* self)
{
    HRESULT hr = S_OK;

    // 初期パーティクル配列を生成（すべてゼロ寿命）
    SimpleArray<BillboardFlipbookParticle> initData(self->m_maxParticles);
    for (UINT i = 0; i < self->m_maxParticles; ++i)
    {
        initData[i].position = self->m_position;
        initData[i].velocity = XMFLOAT3(0, 0, 0);
        initData[i].life = 0.0f; // 初期ライフは0
        initData[i].lifeRemaining = 0.0f; // 残りライフ
        initData[i].rotation = 0.0f;
        initData[i].size = 1.0f;
        initData[i].color = self->m_startColor;
        initData[i].startColor = self->m_startColor;
        initData[i].endColor = self->m_endColor;
        initData[i].frameIndex = 0.0f;
        initData[i].frameSpeed = 0.0f;
    }

    // 構造化バッファの設定
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(BillboardFlipbookParticle) * self->m_maxParticles;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(BillboardFlipbookParticle);

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = initData.data();

    hr = self->m_device->CreateBuffer(&bufferDesc, &subData, &self->m_particleBuffer);
    if (FAILED(hr))
        return false;

    // SRV（Shader Resource View）の作成
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = self->m_maxParticles;

    hr = self->m_device->CreateShaderResourceView(self->m_particleBuffer, &srvDesc, &self->m_particleSRV);
    if (FAILED(hr)) return false;

    // UAV（Unordered Access View）の作成
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = self->m_maxParticles;

    hr = self->m_device->CreateUnorderedAccessView(self->m_particleBuffer, &uavDesc, &self->m_particleUAV);
    if (FAILED(hr))
        return false;

    return true;
}

void ParticleEffectRendererBase::RegisterarticleBufferFactories()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    s_bufferFactoryTable[static_cast<UINT>(ParticleShaderGroup::BillboardSimple)] = &CreateBillboardSimpleBuffer;
    s_bufferFactoryTable[static_cast<UINT>(ParticleShaderGroup::BillboardFlipbook)] = &CreateBillboardFlipbookBuffer;
}

void ParticleEffectRendererBase::UpdateParticleUpdateCB(void)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(m_context->Map(m_cbParticleUpdate, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CBParticleUpdate* cb = reinterpret_cast<CBParticleUpdate*>(mapped.pData);
        cb->deltaTime = m_timer.GetDeltaTime();
        cb->totalTime = m_timer.GetElapsedTime();
        cb->scale = m_scale;
        cb->acceleration = m_acceleration;
        cb->spawnRateMin = m_spawnRateMin;
        cb->spawnRateMax = m_spawnRateMax;
        cb->lifeMin = m_lifeMin;
        cb->lifeMax = m_lifeMax;
        cb->maxParticlesCount = m_maxParticles;
        cb->particlesToEmitThisFrame = m_particlesToEmitThisFrame;
        cb->startColor = m_startColor;
        cb->endColor = m_endColor;
        m_context->Unmap(m_cbParticleUpdate, 0);
    }
}

void ParticleEffectRendererBase::UpdateEmitter(void)
{
    float seed = GetRandFloat(0.0f, 1.0f);
    float spawnRate = Lerp(m_spawnRateMin, m_spawnRateMax, seed);

    m_accumulatedEmitCount += spawnRate * m_timer.GetDeltaTime();

    m_particlesToEmitThisFrame = static_cast<UINT>(m_accumulatedEmitCount);
    m_accumulatedEmitCount -= m_particlesToEmitThisFrame;
}

void ParticleEffectRendererBase::UpdateParticleDrawCB(const XMMATRIX& worldMatrix, const XMMATRIX& viewProj)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(m_context->Map(m_cbParticleDraw, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CBParticleDraw* cb = reinterpret_cast<CBParticleDraw*>(mapped.pData);
        cb->world = XMMatrixTranspose(worldMatrix);
        cb->viewProj = XMMatrixTranspose(viewProj);
        m_context->Unmap(m_cbParticleDraw, 0);
    }
}

void ParticleEffectRendererBase::UpdateDrawArgsInstanceCount(void)
{
    if (!m_drawArgsBuffer || !m_aliveListUAV)
        return; // セーフティチェック

    // DrawInstancedIndirect用のインスタンス数を更新
    m_context->CopyStructureCount(
        m_drawArgsBuffer, 
        offsetof(DrawArgs, InstanceCount), 
        m_aliveListUAV);


#ifdef _DEBUG
    m_context->CopyResource(m_drawArgsStaging, m_drawArgsBuffer);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    m_context->Map(m_drawArgsStaging, 0, D3D11_MAP_READ, 0, &mapped);

    DrawArgs* args = reinterpret_cast<DrawArgs*>(mapped.pData);
    m_debugProc.PrintDebugProc("InstanceCount = %u\n", args->InstanceCount);

    m_context->Unmap(m_drawArgsStaging, 0);
#endif // DEBUG


}

ParticleShaderGroup ParticleEffectRendererBase::GetShaderGroupForEffect(EffectType type)
{
    switch (type)
    {
    case EffectType::Smoke:
        return ParticleShaderGroup::BillboardSimple;

    case EffectType::FireBall:
        return ParticleShaderGroup::BillboardFlipbook;
    default:
        return ParticleShaderGroup::None;
    }
}

ParticleComputeGroup ParticleEffectRendererBase::GetComputeGroupForEffect(EffectType type)
{
    switch (type)
    {
    case EffectType::Smoke:
        return ParticleComputeGroup::BasicBillboard;

    case EffectType::FireBall:
        return ParticleComputeGroup::FlipbookAnimated;
    default:
        return ParticleComputeGroup::None;
    }
}