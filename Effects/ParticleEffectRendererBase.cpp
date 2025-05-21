//=============================================================================
//
// ParticleEffectRendererBase���� [ParticleEffectRendererBase.cpp]
// Author : 
//
//=============================================================================
#include "ParticleEffectRendererBase.h"
#include "Effects/EffectSystem.h"
#include "ParticleStructs.h"
//*****************************************************************************
// �O���[�o���ϐ�
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

    // �p�[�e�B�N���o�b�t�@�쐬
    if (!CreateParticleBuffer())
        return false;

    // �萔�o�b�t�@�쐬
    if (!CreateConstantBuffer())
        return false;

    if (UseDrawIndirect())
    {
        // �����o�b�t�@�쐬
        if (!CreateDrawArgsBuffer())
            return false;

        // �A�N�e�B�u�ȃp�[�e�B�N�����X�g�o�b�t�@�쐬
        if (!CreateAliveListBuffer())
			return false;

        // �t���[���X�g�o�b�t�@�쐬
		if (!CreateFreeListBuffer())
			return false;

        // �t���[���X�g��������
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

    // ��SRV�o�C���h������
    UnbindAllShaderSRVs();

    // ���ʃo�b�t�@���o�C���h
    BindCommonComputeResources();

    // �SUAV���������iAppend�\���͕̂K��Clear�j
    ClearInternalUAVs();

    // �p�[�e�B�N��������X�V
    UpdateEmitter();

    // �p�[�e�B�N���X�V�pCB�X�V
    UpdateParticleUpdateCB();

    // �q�N���X�̃p�[�e�B�N���X�V
    DispatchComputeShader();

    // DrawIndirect �g�p���̂݁A�`��������X�V
    if (UseDrawIndirect())
        UpdateDrawArgsInstanceCount();

    // ���\�[�X�̉����i���̃p�C�v���C���X�e�[�W�ɔ�����j
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
    // DrawInstancedIndirect���g�p����ꍇ�̕`��֐�
    if (!m_drawArgsBuffer) return;
    m_context->DrawInstancedIndirect(m_drawArgsBuffer, 0);
}

bool ParticleEffectRendererBase::CreateConstantBuffer(void)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC desc = {};
    // �X�V�p: acceleration, scale, deltaTime, totalTime, lifeMin/Max, spawnRateMin/Max, padding
    desc.ByteWidth = sizeof(CBParticleUpdate);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&desc, nullptr, &m_cbParticleUpdate);
    if (FAILED(hr))
        return false;

    desc.ByteWidth = sizeof(XMMATRIX) * 2; // �`��p 2�s��: world, viewProj
    hr = m_device->CreateBuffer(&desc, nullptr, &m_cbParticleDraw);
    if (FAILED(hr))
        return false;

    return true;
}

bool ParticleEffectRendererBase::CreateDrawArgsBuffer(void)
{
    HRESULT hr = S_OK;

    DrawArgs args = {};
    args.VertexCountPerInstance = 4;  // 1�̃C���X�^���X������̒��_���i�l�p�`��4���_�j
    args.InstanceCount = 0;          // �C���X�^���X���i�����l��0�j
    args.StartVertexLocation = 0;
    args.StartInstanceLocation = 0;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = &args;

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(DrawArgs);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = 0; // �o�C���h�t���O�͐ݒ肵�Ȃ��ACopyStructureCount�p
    desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS; // DrawInstancedIndirect�p


    hr = m_device->CreateBuffer(&desc, &init, &m_drawArgsBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

bool ParticleEffectRendererBase::CreateAliveListBuffer(void)
{
    HRESULT hr = S_OK;

    // �\����1������̃T�C�Y�iUINT = 4�o�C�g�j
    const UINT elementSize = sizeof(UINT);

    // �ő嗱�q�����̗v�f�����m�ہi�A�N�e�B�u���X�g�͍ő� m_maxParticles �j
    const UINT elementCount = m_maxParticles;

    // �o�b�t�@�̍쐬�i�\�����o�b�t�@�j
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = elementSize * elementCount;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = elementSize;

    hr = m_device->CreateBuffer(&desc, nullptr, &m_aliveListBuffer);
    if (FAILED(hr))
        return false;

    // UAV �̍쐬�iCompute Shader ���� index �������ݗp�j
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = elementCount;
    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; // AppendStructuredBuffer �p��Append�t���O

    hr = m_device->CreateUnorderedAccessView(m_aliveListBuffer, &uavDesc, &m_aliveListUAV);
    if (FAILED(hr))
        return false;

    // SRV �̍쐬�iVS ���� index �ǂݏo���p�j
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

    // �\����1������̃T�C�Y�iUINT = 4�o�C�g�j
    const UINT elementSize = sizeof(UINT);

    // �ő嗱�q�����̗v�f�����m�ہi���S���q��ID���ő� m_maxParticles �ێ��\�j
    const UINT elementCount = m_maxParticles;

    // �o�b�t�@�̍쐬�i�\�����o�b�t�@�j
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = elementSize * elementCount;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = elementSize;

    hr = m_device->CreateBuffer(&desc, nullptr, &m_freeListBuffer);
    if (FAILED(hr))
        return false;

    // UAV �̍쐬�iAppendStructuredBuffer �p��Append�t���O���w��j
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = elementCount;
    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; // Append�o�b�t�@�̃t���O

    hr = m_device->CreateUnorderedAccessView(m_freeListBuffer, &uavDesc, &m_freeListUAV);
    if (FAILED(hr))
        return false;

    // consumeUAV �̍쐬�iConsumeStructuredBuffer �Ƃ��ēǂݏo���p�j
    D3D11_UNORDERED_ACCESS_VIEW_DESC consumUavDesc = {};
    consumUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    consumUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    consumUavDesc.Buffer.FirstElement = 0;
    consumUavDesc.Buffer.NumElements = elementCount;
    consumUavDesc.Buffer.Flags = 0; // Consume�o�b�t�@�̃t���O

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
        // Emit�pCS�̏���
        PrepareForEmitCS();

        // Emit�pCS�̃o�C���h
        m_context->CSSetShader(m_emitParticlesCS.cs, nullptr, 0);

        UINT threadGroupCount = GET_THREAD_GROUP_COUNT(m_particlesToEmitThisFrame, 64); // �X���b�h�O���[�v���v�Z
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
    // �p�[�e�B�N���`��pCB�X�V
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
        // �A�N�e�B�u�ȃp�[�e�B�N��ID���X�g�iRWStructuredBuffer<uint> g_AliveList : register(SLOT_UAV_ALIVE_LIST)�j
        if (m_aliveListUAV)
        {
            m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_ALIVE_LIST, m_aliveListUAV);
        }

        // �t���[���X�g�iRWStructuredBuffer<uint> g_FreeList : register(SLOT_UAV_FREE_LIST)�j
        if (m_freeListUAV)
		{
			m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST, m_freeListUAV);
		}
    }

}

void ParticleEffectRendererBase::UnbindComputeUAVs(void)
{
    ID3D11UnorderedAccessView* nullUAV = nullptr;

    // �p�[�e�B�N����UAV�͏�Ɏg�p
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_PARTICLE, nullUAV);

    // AliveList/FreeList ��UAV����ɃN���A
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_ALIVE_LIST, nullUAV);
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST, nullUAV);
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST_CONSUME, nullUAV);
}

void ParticleEffectRendererBase::UnbindAllShaderSRVs(void)
{
    ID3D11ShaderResourceView* nullSRV = nullptr;

    // VS�̃��X�gSRV������
    m_shaderResourceBinder.BindShaderResource(ShaderStage::VS, SLOT_SRV_ALIVE_LIST, nullSRV);

    // GS�̃p�[�e�B�N��SRV������
    m_shaderResourceBinder.BindShaderResource(ShaderStage::GS, SLOT_SRV_PARTICLE, nullSRV);

}

void ParticleEffectRendererBase::PrepareForEmitCS(void)
{
    // EmitCS �p�̃��\�[�X����
    // Append �p UAV�ifreeListUAV�j�� �A���o�C���h
    m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST, nullptr);

    // Consume �p UAV �� �o�C���h�iSRV �ł͂Ȃ� UAV �ł���_�ɒ��Ӂj
    if (m_freeListConsumeUAV)
    {
        // ConsumeStructuredBuffer �ɑΉ����̂��� SRV �ł͂Ȃ� UAV ��ʂ��ăo�C���h����K�v������܂�
        m_shaderResourceBinder.BindUnorderedAccessView(SLOT_UAV_FREE_LIST_CONSUME, m_freeListConsumeUAV);
    }
}

void ParticleEffectRendererBase::ClearInternalUAVs(void)
{
    UINT clear[4] = { 0 };

    // �A�N�e�B�u���X�g���N���A
    if (m_aliveListUAV)
        m_context->ClearUnorderedAccessViewUint(m_aliveListUAV, clear);

    // �t���[���X�g���N���A
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
    // ���łɐݒ�ς݂Ȃ�X�L�b�v
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

    // UpdateCS�`�F�b�N���擾
    loadShaders &= ShaderManager::get_instance().HasComputerShader(m_computeGroup, ComputePassType::Update);
    if (loadShaders)
        m_updateParticlesCS = ShaderManager::get_instance().GetComputeShader(m_computeGroup, ComputePassType::Update);

    if (UseDrawIndirect())
	{
		// EmitCS�`�F�b�N���擾
		loadShaders &= ShaderManager::get_instance().HasComputerShader(m_computeGroup, ComputePassType::Emit);
		if (loadShaders)
			m_emitParticlesCS = ShaderManager::get_instance().GetComputeShader(m_computeGroup, ComputePassType::Emit);
	}

    // VS/GS/PS�Z�b�g�`�F�b�N���擾
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
        initData[i].life = 0.0f; // �������C�t��0
        initData[i].lifeRemaining = 0.0f; // �c�胉�C�t
        initData[i].rotation = 0.0f; // ������]�p�x
        initData[i].color = self->m_startColor;
        initData[i].startColor = self->m_startColor;
        initData[i].endColor = self->m_endColor;
    }

    // �o�b�t�@�쐬
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

    // SRV�쐬
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = self->m_maxParticles;
    hr = self->m_device->CreateShaderResourceView(self->m_particleBuffer, &srvDesc, &self->m_particleSRV);
    if (FAILED(hr))
        return false;

    // UAV�쐬
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

    // �����p�[�e�B�N���z��𐶐��i���ׂă[�������j
    SimpleArray<BillboardFlipbookParticle> initData(self->m_maxParticles);
    for (UINT i = 0; i < self->m_maxParticles; ++i)
    {
        initData[i].position = self->m_position;
        initData[i].velocity = XMFLOAT3(0, 0, 0);
        initData[i].life = 0.0f; // �������C�t��0
        initData[i].lifeRemaining = 0.0f; // �c�胉�C�t
        initData[i].rotation = 0.0f;
        initData[i].size = 1.0f;
        initData[i].color = self->m_startColor;
        initData[i].startColor = self->m_startColor;
        initData[i].endColor = self->m_endColor;
        initData[i].frameIndex = 0.0f;
        initData[i].frameSpeed = 0.0f;
    }

    // �\�����o�b�t�@�̐ݒ�
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

    // SRV�iShader Resource View�j�̍쐬
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = self->m_maxParticles;

    hr = self->m_device->CreateShaderResourceView(self->m_particleBuffer, &srvDesc, &self->m_particleSRV);
    if (FAILED(hr)) return false;

    // UAV�iUnordered Access View�j�̍쐬
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
        return; // �Z�[�t�e�B�`�F�b�N

    // DrawInstancedIndirect�p�̃C���X�^���X�����X�V
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