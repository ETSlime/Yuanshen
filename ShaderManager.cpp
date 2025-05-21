//=============================================================================
//
// ShaderManager���� [ShaderManager.cpp]
// Author : 
//
//=============================================================================
#include "ShaderManager.h"
#include "Renderer.h"
#include "IEffectRenderer.h"

ShaderManager::~ShaderManager()
{
    for (auto& it : m_shaderSets)
    {
        SafeRelease(&it.value.vs);
        SafeRelease(&it.value.ps);
        SafeRelease(&it.value.gs);
    }

    for (auto& it : m_shadowShaderSets)
    {
        SafeRelease(&it.value.vs);
        SafeRelease(&it.value.ps);
        SafeRelease(&it.value.alphaPs);
    }

    for (auto& it : m_computeShaders)
    {
        SafeRelease(&it.value.cs);
    }

    for (auto& it : m_inputLayouts)
    {
        SafeRelease(&it.value);
    }

    SafeRelease(&m_sharedShadowPS);
}

void ShaderManager::Init(ID3D11Device* device)
{
    // �V�F�[�_�[�ƃV���h�E�V�F�[�_�[�̃\�[�X���܂Ƃ߂ēo�^����
    RegisterShaderSourcePaths();
    CreateInputLayouts(device);
	LoadAllShaders(device);
}

void ShaderManager::RegisterShaderSourcePaths(void)
{
    // ===== �ʏ�V�F�[�_�[�̓o�^ =====
    RegisterShaderSource(ShaderSetID::StaticModel, VertexLayoutID::Static,
        "Shaders/shader.hlsl", "VertexShaderPolygon",
        "Shaders/shader.hlsl", "PixelShaderPolygon");
    RegisterShaderSource(ShaderSetID::SkinnedModel, VertexLayoutID::Skinned,
        "Shaders/shader.hlsl", "SkinnedMeshVertexShaderPolygon",
        "Shaders/shader.hlsl", "SkinnedMeshPixelShader");
    RegisterShaderSource(ShaderSetID::Instanced_Grass, VertexLayoutID::Instanced,
        "Shaders/Grass.hlsl", "VS",
        "Shaders/Grass.hlsl", "PS", 
        nullptr, nullptr, false);
    RegisterShaderSource(ShaderSetID::Instanced_Tree, VertexLayoutID::Instanced,
        "Shaders/Tree.hlsl", "VS",
        "Shaders/Tree.hlsl", "PS", 
        nullptr, nullptr, false);
    RegisterShaderSource(ShaderSetID::Skybox, VertexLayoutID::Skybox,
        "Shaders/Skybox.hlsl", "VS",
        "Shaders/Skybox.hlsl", "PS");
    RegisterShaderSource(ShaderSetID::VFX, VertexLayoutID::VFX,
        "Shaders/VFX.hlsl", "VS",
        "Shaders/VFX.hlsl", "PS");
    RegisterShaderSource(ShaderSetID::FireEffect, VertexLayoutID::FireEffect,
        "Shaders/FireEffect.hlsl", "VS",
        "Shaders/FireEffect.hlsl", "PS",
        "Shaders/FireEffect.hlsl", "GS", false);
    RegisterShaderSource(ParticleShaderGroup::BillboardFlipbook, VertexLayoutID::ParticleFlipbook,
        "Shaders/ParticleBillboardFlipbook.hlsl", "VS",
        "Shaders/ParticleBillboardFlipbook.hlsl", "PS",
        "Shaders/ParticleBillboardFlipbook.hlsl", "GS", false);
    RegisterShaderSource(ParticleShaderGroup::BillboardSimple, VertexLayoutID::ParticleSimple,
        "Shaders/ParticleBillboardSimple.hlsl", "VS",
        "Shaders/ParticleBillboardSimple.hlsl", "PS",
        "Shaders/ParticleBillboardSimple.hlsl", "GS", false);
    RegisterShaderSource(ShaderSetID::UI, VertexLayoutID::UI,
        "Shaders/UI.hlsl", "VS",
        "Shaders/UI.hlsl", "PS");
    RegisterShaderSource(ShaderSetID::Debug, VertexLayoutID::Debug,
        "Shaders/DebugBox.hlsl", "VS",
        "Shaders/DebugBox.hlsl", "PS");

    // ===== �V���h�E�V�F�[�_�[�̓o�^ =====
    RegisterShadowShaderSource(ShaderSetID::StaticModel, VertexLayoutID::Static,
        "Shaders/DepthMap.hlsl", "VS");
    RegisterShadowShaderSource(ShaderSetID::SkinnedModel, VertexLayoutID::Skinned,
        "Shaders/DepthMap.hlsl", "SkinnedMeshVertexShaderPolygon");
    RegisterShadowShaderSource(ShaderSetID::Instanced_Tree, VertexLayoutID::Instanced,
        "Shaders/Tree.hlsl", "VSShadow", 
        nullptr, nullptr, 
        "Shaders/Tree.hlsl", "PSShadowAlphaTest", false);

    //RegisterComputeShaderSource(ShaderSetID::FireEffect, "Shaders/FireEffect.hlsl", "CS");
    RegisterComputeShaderSource(ParticleComputeGroup::BasicBillboard, ComputePassType::Update,
        "Shaders/ParticleBillboardSimple.hlsl", "UpdateCS");
    RegisterComputeShaderSource(ParticleComputeGroup::BasicBillboard, ComputePassType::Emit,
        "Shaders/ParticleBillboardSimple.hlsl", "EmitCS");
    RegisterComputeShaderSource(ParticleComputeGroup::FlipbookAnimated, ComputePassType::Update,
        "Shaders/ParticleBillboardFlipbook.hlsl", "UpdateCS");
    RegisterComputeShaderSource(ParticleComputeGroup::FlipbookAnimated, ComputePassType::Emit,
        "Shaders/ParticleBillboardFlipbook.hlsl", "EmitCS");
}

void ShaderManager::RegisterShaderReloadEntry(const char* path, const char* entry, const char* target, void** outputShaderPtr, 
    ShaderCreateFunc createFunc, ShaderGroup shaderGroup, uint64_t shaderSetID, ShaderType shaderType, ComputePassType pass)
{
    ShaderReloadEntry entryInfo;
    entryInfo.path = path;
    entryInfo.entry = entry;
    entryInfo.target = target;
    entryInfo.outputShaderPtr = outputShaderPtr;
    entryInfo.createShaderFunc = createFunc;
    entryInfo.shaderGroup = shaderGroup;
    entryInfo.shaderSetID = shaderSetID;
    entryInfo.shaderType = shaderType;
    entryInfo.computePass = pass;

    m_shaderReloadEntries.push_back(entryInfo);

    // �t�@�C�����Ď����X�g�ɒǉ�
    m_reloader.WatchFile(path);
}

void ShaderManager::SyncReloadedShaderToShaderSet(const ShaderReloadEntry& entry)
{
    switch (entry.shaderGroup)
    {
    case ShaderGroup::Default:
        switch (entry.shaderType)
        {
        case ShaderType::VS:
            m_shaderSets[TO_UINT64(entry.shaderSetID)].vs = *(ID3D11VertexShader**)entry.outputShaderPtr;
            break;
        case ShaderType::PS:
            m_shaderSets[TO_UINT64(entry.shaderSetID)].ps = *(ID3D11PixelShader**)entry.outputShaderPtr;
            break;
        case ShaderType::GS:
            m_shaderSets[TO_UINT64(entry.shaderSetID)].gs = *(ID3D11GeometryShader**)entry.outputShaderPtr;
            break;
        }
        break;

    case ShaderGroup::Particle:
        switch (entry.shaderType)
        {
        case ShaderType::VS:
            m_particleShaderSets[TO_UINT64(entry.shaderSetID)].vs = *(ID3D11VertexShader**)entry.outputShaderPtr;
            break;
        case ShaderType::PS:
            m_particleShaderSets[TO_UINT64(entry.shaderSetID)].ps = *(ID3D11PixelShader**)entry.outputShaderPtr;
            break;
        case ShaderType::GS:
            m_particleShaderSets[TO_UINT64(entry.shaderSetID)].gs = *(ID3D11GeometryShader**)entry.outputShaderPtr;
        }
        break;

    case ShaderGroup::Shadow:
        switch (entry.shaderType)
        {
        case ShaderType::VS:
            m_shadowShaderSets[TO_UINT64(entry.shaderSetID)].vs = *(ID3D11VertexShader**)entry.outputShaderPtr;
            break;
        case ShaderType::PS:
            m_shadowShaderSets[TO_UINT64(entry.shaderSetID)].ps = *(ID3D11PixelShader**)entry.outputShaderPtr;
            break;
        case ShaderType::PS_ALPHA:
            m_shadowShaderSets[TO_UINT64(entry.shaderSetID)].alphaPs = *(ID3D11PixelShader**)entry.outputShaderPtr;
        }
        break;

    case ShaderGroup::Compute:
        m_computeShaders[ComputeShaderSetKey({ entry.shaderSetID, entry.computePass })].cs = *(ID3D11ComputeShader**)entry.outputShaderPtr;
        break;

    case ShaderGroup::ParticleCompute:
        m_particleComputeShaders[ComputeShaderSetKey({ entry.shaderSetID, entry.computePass })].cs = *(ID3D11ComputeShader**)entry.outputShaderPtr;
        break;
    }
}


void ShaderManager::CreateInputLayouts(ID3D11Device* device)
{
    // Static ���f���p InputLayout
    //CreateStaticInputLayout(device);

    // Skinned ���f���p InputLayout
    //CreateSkinnedInputLayout(device);

    // Instanced ���f���p InputLayout
    CreateInstancedInputLayout(device);
}

void ShaderManager::LoadAllShaders(ID3D11Device* device)
{
    // �ʏ�V�F�[�_�[��S�ēǂݍ���
    for (const auto& it : m_shaderSourcePaths)
    {
        ShaderSetID id = static_cast<ShaderSetID>(it.key);
        const ShaderSourcePathSet& info = it.value;

        ShaderSet set;

        if (info.useReflection)
        {
            // ���˂��g�p���� InputLayout ����������
            ShaderLoader::LoadShaderSet(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                nullptr, 0, set);
        }
        else
        {
            // �O���ō쐬�ς݂� InputLayout ���g�p
            ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                set);

            set.inputLayout = m_inputLayouts[TO_UINT64(info.layoutID)];
        }

        set.layoutID = it.value.layoutID;
        m_shaderSets[TO_UINT64(id)] = set;

#ifdef _DEBUG
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Default, id, ShaderType::VS, ComputePassType::None, Vertex, info.vsPath, info.vsEntry, SHADER_MODEL_VS, &m_shaderSets[TO_UINT64(id)].vs);
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Default, id, ShaderType::PS, ComputePassType::None, Pixel, info.psPath, info.psEntry, SHADER_MODEL_PS, &m_shaderSets[TO_UINT64(id)].ps);
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Default, id, ShaderType::GS, ComputePassType::None, Geometry, info.gsPath, info.gsEntry, SHADER_MODEL_GS, &m_shaderSets[TO_UINT64(id)].gs);
#endif // DEBUG

    }

    // �V���h�E�}�b�v�p�V�F�[�_�[��S�ēǂݍ���
    for (const auto& it : m_shadowShaderSourcePaths)
    {
        ShaderSetID id = static_cast<ShaderSetID>(it.key);
        const ShadowShaderSourcePathSet& info = it.value;

        ShadowShaderSet shadowSet;

        if (info.useReflection)
        {
            // ���˂��g�p���� InputLayout ����������
            ShaderLoader::LoadShadowShaderSet(device,
                info.vsPath, info.psPath, info.psAlphaPath,
                info.vsEntry, info.psEntry, info.psAlphaEntry,
                nullptr, 0, shadowSet);
        }
        else
        {
            // �O���ō쐬�ς݂� InputLayout ���g�p
            ShaderLoader::LoadShadowShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.psPath, info.psAlphaPath,
                info.vsEntry, info.psEntry, info.psAlphaEntry,
                shadowSet);
            shadowSet.inputLayout = m_inputLayouts[TO_UINT64(info.layoutID)];
        }

        m_shadowShaderSets[TO_UINT64(id)] = shadowSet;

#ifdef _DEBUG
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Shadow, id, ShaderType::VS, ComputePassType::None, Vertex, info.vsPath, info.vsEntry, SHADER_MODEL_VS, &m_shadowShaderSets[TO_UINT64(id)].vs);
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Shadow, id, ShaderType::PS, ComputePassType::None, Pixel, info.psPath, info.psEntry, SHADER_MODEL_PS, &m_shadowShaderSets[TO_UINT64(id)].ps);
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Shadow, id, ShaderType::PS_ALPHA, ComputePassType::None, Pixel, info.psAlphaPath, info.psAlphaEntry, SHADER_MODEL_PS, &m_shadowShaderSets[TO_UINT64(id)].alphaPs);
#endif // DEBUG

    }

    for (auto& it : m_computeShaders)
    {
        ComputeShaderSet& shaderSet = it.value;

        ShaderLoader::LoadComputerShaderSet(device, shaderSet);

#ifdef _DEBUG
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Compute, it.value.shaderSetID, ShaderType::GS, it.value.pass, Compute, shaderSet.path, shaderSet.entry, SHADER_MODEL_CS, &shaderSet.cs);
#endif // DEBUG

    }

    for (auto& it : m_particleComputeShaders)
    {
        ComputeShaderSet& shaderSet = it.value;

        ShaderLoader::LoadComputerShaderSet(device, shaderSet);

#ifdef _DEBUG
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::ParticleCompute, it.value.shaderSetID, ShaderType::GS, it.value.pass, Compute, shaderSet.path, shaderSet.entry, SHADER_MODEL_CS, &shaderSet.cs);
#endif // DEBUG

    }


    for (const auto& it : m_particleShaderSourcePaths)
    {
        ParticleShaderGroup group = static_cast<ParticleShaderGroup>(it.key);
        const ShaderSourcePathSet& info = it.value;

        ShaderSet set;

        if (info.useReflection)
        {
            // ���˂��g�p���� InputLayout ����������
            ShaderLoader::LoadShaderSet(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                nullptr, 0, set);
        }
        else
        {
            // �O���ō쐬�ς݂� InputLayout ���g�p
            ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                set);

            set.inputLayout = m_inputLayouts[TO_UINT64(info.layoutID)];
        }

        set.layoutID = it.value.layoutID;
        m_particleShaderSets[TO_UINT64(group)] = set;

#ifdef _DEBUG
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Particle, group, ShaderType::VS, ComputePassType::None, Vertex, info.vsPath, info.vsEntry, SHADER_MODEL_VS, &m_particleShaderSets[TO_UINT64(group)].vs);
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Particle, group, ShaderType::PS, ComputePassType::None, Pixel, info.psPath, info.psEntry, SHADER_MODEL_PS, &m_particleShaderSets[TO_UINT64(group)].ps);
        REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Particle, group, ShaderType::GS, ComputePassType::None, Geometry, info.gsPath, info.gsEntry, SHADER_MODEL_GS, &m_particleShaderSets[TO_UINT64(group)].gs);
#endif // DEBUG

    }
}

void ShaderManager::LoadSingleShaderSet(ID3D11Device* device, ShaderSetID id,
    const char* vsPath, const char* vsEntry, 
    const char* psPath, const char* psEntry,
    const char* gsPath, const char* gsEntry,
    bool createInputLayout)
{
    ShaderSet set;
    set.ps = m_sharedShadowPS;
    if (createInputLayout)
    {
        ShaderLoader::LoadShaderSet(device, vsPath, vsEntry, psPath, psEntry, gsPath, gsEntry, nullptr, 0, set);
    }
    else
    {
        ShaderLoader::LoadShaderSetWithoutLayoutCreation(device, vsPath, vsEntry, psPath, psEntry, gsPath, gsEntry, set);
        set.inputLayout = m_inputLayouts[TO_UINT64(id)];
    }
    m_shaderSets[TO_UINT64(id)] = set;

#ifdef _DEBUG
    REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Default, id, ShaderType::VS, ComputePassType::None, Vertex, vsPath, vsEntry, SHADER_MODEL_VS, &set.vs);
    REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Default, id, ShaderType::PS, ComputePassType::None, Pixel, psPath, psEntry, SHADER_MODEL_PS, &set.ps);
    REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Default, id, ShaderType::GS, ComputePassType::None, Geometry, gsPath, gsEntry, SHADER_MODEL_GS, &set.gs);
#endif // DEBUG
}

void ShaderManager::LoadShadowShaderSet(ID3D11Device* device, ShaderSetID id,
    const char* vsPath, const char* vsEntry, 
    const char* psPath, const char* psEntry, 
    const char* psAlphaPath, const char* psAlphaEntry,
    bool createInputLayout)
{
    ShadowShaderSet set;
    set.ps = m_sharedShadowPS;
    if (createInputLayout)
    {
        ShaderLoader::LoadShadowShaderSet(device, vsPath, vsEntry, psPath, psEntry, psAlphaPath, psAlphaEntry, nullptr, 0, set);
    }
    else
    {
        ShaderLoader::LoadShadowShaderSetWithoutLayoutCreation(device, vsPath, vsEntry, psPath, psEntry, psAlphaPath, psAlphaEntry, set);
        set.inputLayout = m_inputLayouts[TO_UINT64(id)];
    }

    m_shadowShaderSets[TO_UINT64(id)] = set;

#ifdef _DEBUG
    REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Shadow, id, ShaderType::VS, ComputePassType::None, Vertex, vsPath, vsEntry, SHADER_MODEL_VS, &set.vs);
    REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Shadow, id, ShaderType::PS, ComputePassType::None, Pixel, psPath, psEntry, SHADER_MODEL_PS, &set.ps);
    REGISTER_SHADER_RELOAD_ENTRY((*this), ShaderGroup::Shadow, id, ShaderType::PS_ALPHA, ComputePassType::None, Pixel, psAlphaPath, psAlphaPath, SHADER_MODEL_PS, &set.alphaPs);
#endif // DEBUG
}

void ShaderManager::RegisterShaderSource(ShaderSetID id, VertexLayoutID layoutID, 
    const char* vsPath, const char* vsEntry, 
    const char* psPath, const char* psEntry, 
    const char* gsPath, const char* gsEntry,
    bool useReflection)
{
    m_shaderSourcePaths[TO_UINT64(id)] = ShaderSourcePathSet{
    vsPath, psPath, gsPath, vsEntry, psEntry, gsEntry, useReflection, layoutID
    };
}

void ShaderManager::RegisterShaderSource(ParticleShaderGroup group, VertexLayoutID layoutID, const char* vsPath, const char* vsEntry, const char* psPath, const char* psEntry, const char* gsPath, const char* gsEntry, bool useReflection)
{
    m_particleShaderSourcePaths[TO_UINT64(group)] = ShaderSourcePathSet{
    vsPath, psPath, gsPath, vsEntry, psEntry, gsEntry, useReflection, layoutID
    };
}

void ShaderManager::RegisterShadowShaderSource(ShaderSetID id, VertexLayoutID layoutID, 
    const char* vsPath, const char* vsEntry, 
    const char* psPath, const char* psEntry, 
    const char* psAlphaPath, const char* psAlphaEntry, 
    bool useReflection)
{
    m_shadowShaderSourcePaths[TO_UINT64(id)] = ShadowShaderSourcePathSet{
    vsPath, psPath, psAlphaPath, vsEntry, psEntry, psAlphaEntry, useReflection, layoutID
    };
}

void ShaderManager::RegisterComputeShaderSource(ShaderSetID id, ComputePassType pass, const char* path, const char* entry)
{
    ComputeShaderSetKey key{ TO_UINT64(id), pass };
    m_computeShaders[key] = ComputeShaderSet{ nullptr, path, entry, pass, TO_UINT64(id) };
}

void ShaderManager::RegisterComputeShaderSource(ParticleComputeGroup group, ComputePassType pass, const char* path, const char* entry)
{
    ComputeShaderSetKey key{ TO_UINT64(group), pass };
    m_particleComputeShaders[key] = ComputeShaderSet{ nullptr, path, entry, pass, TO_UINT64(group) };
}


void ShaderManager::SetSharedShadowPixelShader(ID3D11PixelShader* sharedPS)
{
    m_sharedShadowPS = sharedPS;
}

bool ShaderManager::HasShadowShaderSet(ShaderSetID id) const
{
    return m_shadowShaderSets.find(TO_UINT64(id)) != m_shadowShaderSets.end();
}

bool ShaderManager::HasShaderSet(ShaderSetID id) const
{
    return m_shaderSets.find(TO_UINT64(id)) != m_shaderSets.end();
}

bool ShaderManager::HasShaderSet(ParticleShaderGroup group) const
{
    return m_particleShaderSets.find(TO_UINT64(group)) != m_particleShaderSets.end();
}

bool ShaderManager::HasComputerShader(ShaderSetID id, ComputePassType pass) const
{
    ComputeShaderSetKey key{ TO_UINT64(id), pass };
    return m_computeShaders.find(key) != m_computeShaders.end();
}

bool ShaderManager::HasComputerShader(ParticleComputeGroup group, ComputePassType pass) const
{
    ComputeShaderSetKey key{ TO_UINT64(group), pass };
    return m_particleComputeShaders.find(key) != m_particleComputeShaders.end();
}

// �V�F�[�_�[�̃\�[�X�t�@�C���ɕύX������΃����[�h����
void ShaderManager::CollectAndReloadShaders(ID3D11Device* device)
{
    HashMap<const char*, bool, CharPtrHash, CharPtrEquals> changedPaths;

    // �ŏ��ɁA�e�t�@�C���̕ύX����x�����m�F�A���ʂ��L���b�V������
    for (const auto& reloadEntry : m_shaderReloadEntries)
    {
        const char* path = reloadEntry.path;

        // ���̃t���[���ł܂����� path ���`�F�b�N���Ă��Ȃ����
        if (changedPaths.count(path) == 0 && m_reloader.HasFileChanged(path))
        {
            // �ύX�������� path ���L�^���Ă���
            // ���� path ���g�� PS/VS/GS ���ׂĂɂ��̌��ʂ��g����I
            // ����ɂ��AVS �ōX�V�ς݂� timestamp �ɂ���� PS ���X�L�b�v�����o�O��h��
            changedPaths[path] = true;
        }
    }

    // ���ɁAblob���x���̕ύX�𔻒f
    for (const auto& reloadEntry : m_shaderReloadEntries)
    {
        // �t�@�C�����̂ɕύX�Ȃ� �� �X�L�b�v
        if (changedPaths.count(reloadEntry.path) == 0)
            continue;

        const char* path = reloadEntry.path;
        const char* entry = reloadEntry.entry;
        const char* target = reloadEntry.target;

        ID3DBlob* blob = nullptr;
        if (!ShaderLoader::CompileShaderFromFile(path, entry, target, &blob))
            continue;

        if (m_reloader.HasShaderContentChanged(path, entry, blob->GetBufferPointer(), blob->GetBufferSize()))
        {
            void* newShader = nullptr;
            if (reloadEntry.createShaderFunc(device, blob->GetBufferPointer(), blob->GetBufferSize(), &newShader) == S_OK)
            {
                // ���ʃ|�C���^����FID3D11DeviceChild �Ƃ��� release
                SafeRelease((ID3D11DeviceChild**)reloadEntry.outputShaderPtr);
                *(ID3D11DeviceChild**)reloadEntry.outputShaderPtr = static_cast<ID3D11DeviceChild*>(newShader);
            }

            SyncReloadedShaderToShaderSet(reloadEntry);
        }
        blob->Release();
    }
}

const ShaderSet& ShaderManager::GetShaderSet(ShaderSetID id) const
{
    return m_shaderSets.at(TO_UINT64(id));
}

const ShaderSet& ShaderManager::GetShaderSet(ParticleShaderGroup group) const
{
    return m_particleShaderSets.at(TO_UINT64(group));
}

const ShadowShaderSet& ShaderManager::GetShadowShaderSet(ShaderSetID id) const
{
    return m_shadowShaderSets.at(TO_UINT64(id));
}

const ComputeShaderSet& ShaderManager::GetComputeShader(ShaderSetID id, ComputePassType pass) const
{
    ComputeShaderSetKey key{ TO_UINT64(id), pass };
    return m_computeShaders.at(key);
}

const ComputeShaderSet& ShaderManager::GetComputeShader(ParticleComputeGroup group, ComputePassType pass) const
{
    ComputeShaderSetKey key{ TO_UINT64(group), pass };
    return m_particleComputeShaders.at(key);
}

ID3D11InputLayout* ShaderManager::GetInputLayout(VertexLayoutID id) const
{
    // �蓮�œo�^���ꂽ InputLayout ���n�b�V���e�[�u������T��
    auto it = m_inputLayouts.find(TO_UINT64(id));
    if (it != m_inputLayouts.end())
    {
        return it->value;
    }

    // ������Ȃ��ꍇ�A���ׂĂ� ShaderSet �𑖍����� layoutID ����v������̂�T��
    for (const auto& pair : m_shaderSets)
    {
        const ShaderSet& shaderSet = pair.value;
        if (shaderSet.layoutID == id && shaderSet.inputLayout)
        {
            return shaderSet.inputLayout;
        }
    }

    // �ǂ���ɂ����݂��Ȃ��ꍇ�� nullptr ��Ԃ�
    return nullptr;
}

void ShaderManager::CreateInstancedInputLayout(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Weight
    { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },

    // �C���X�^���X�f�[�^
    { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // OffsetPosition
    { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // Rotation (Quaternion)
    { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // initialBillboardRot (�����r���{�[�h��])
    { "TEXCOORD", 4, DXGI_FORMAT_R32_FLOAT,          1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // Scale
    { "TEXCOORD", 5, DXGI_FORMAT_R32_FLOAT,          1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 } // Type
    };

    ID3DBlob* vsBlob = nullptr;
    ShaderLoader::CompileShaderFromFile("Shaders/Tree.hlsl", "VS", SHADER_MODEL_VS, &vsBlob);
    ID3D11InputLayout* inputLayout = nullptr;
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    vsBlob->Release();

    m_inputLayouts[TO_UINT64(VertexLayoutID::Instanced)] = inputLayout;
}

void ShaderManager::CreateSkinnedInputLayout(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BONEINDEX",0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3DBlob* vsBlob = nullptr;
    ShaderLoader::CompileShaderFromFile("Shaders/shader.hlsl", "SkinnedMeshVertexShaderPolygon", SHADER_MODEL_VS, &vsBlob);
    ID3D11InputLayout* inputLayout = nullptr;
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    vsBlob->Release();

    m_inputLayouts[TO_UINT64(VertexLayoutID::Skinned)] = inputLayout;
}

void ShaderManager::CreateStaticInputLayout(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3DBlob* vsBlob = nullptr;
    ShaderLoader::CompileShaderFromFile("Shaders/shader.hlsl", "VertexShaderPolygon", SHADER_MODEL_VS, &vsBlob);
    ID3D11InputLayout* inputLayout = nullptr;
    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    vsBlob->Release();

    m_inputLayouts[TO_UINT64(VertexLayoutID::Static)] = inputLayout;
}

void ShaderManager::CreateEffectInputLayout(ID3D11Device* device, EffectType type)
{
    ID3D11InputLayout* inputLayout = nullptr;

    // ��̃��C�A�E�g�i���_�v�f�Ȃ��j
    D3D11_INPUT_ELEMENT_DESC* emptyLayoutDESC = nullptr;
    // �v�f����0
    UINT numElements = 0;
    ID3DBlob* vsBlob = nullptr;

    HRESULT hr = S_OK;

    switch (type)
    {
        case EffectType::Fire:
            ShaderLoader::CompileShaderFromFile("Shaders/FireEffect.hlsl", "VS", SHADER_MODEL_VS, &vsBlob);
            hr = device->CreateInputLayout(
                emptyLayoutDESC,
                numElements,
                vsBlob->GetBufferPointer(),
                vsBlob->GetBufferSize(),
                &inputLayout);
            if (FAILED(hr)) 
                return;
            m_inputLayouts[TO_UINT64(VertexLayoutID::FireEffect)] = inputLayout;
		break;

        case EffectType::Smoke:
            ShaderLoader::CompileShaderFromFile("Shaders/Smokeffect.hlsl", "VS", SHADER_MODEL_VS, &vsBlob);
            hr = device->CreateInputLayout(
                emptyLayoutDESC,
                numElements,
                vsBlob->GetBufferPointer(),
                vsBlob->GetBufferSize(),
                &inputLayout);
            if (FAILED(hr))
                return;
            m_inputLayouts[TO_UINT64(VertexLayoutID::SmokeEffect)] = inputLayout;
        break;

        case EffectType::SoftBody:
            ShaderLoader::CompileShaderFromFile("Shaders/SoftBody.hlsl", "VS", SHADER_MODEL_VS, &vsBlob);
            hr = device->CreateInputLayout(
                emptyLayoutDESC,
                numElements,
                vsBlob->GetBufferPointer(),
                vsBlob->GetBufferSize(),
                &inputLayout);
            if (FAILED(hr))
                return;
            m_inputLayouts[TO_UINT64(VertexLayoutID::SoftBody)] = inputLayout;
            break;

        default:
            return;
    }
}
