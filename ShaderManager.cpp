//=============================================================================
//
// ShaderManager処理 [ShaderManager.cpp]
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
    // シェーダーとシャドウシェーダーのソースをまとめて登録する
    RegisterShaderSourcePaths();
    CreateInputLayouts(device);
	LoadAllShaders(device);
}

void ShaderManager::RegisterShaderSourcePaths(void)
{
    // ===== 通常シェーダーの登録 =====
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
    RegisterShaderSource(ParticleShaderGroup::BillboardFlipbook, VertexLayoutID::SmokeEffect,
        "Shaders/SmokeEffect.hlsl", "VS",
        "Shaders/SmokeEffect.hlsl", "PS",
        "Shaders/SmokeEffect.hlsl", "GS", false);
    RegisterShaderSource(ParticleShaderGroup::BillboardSimple, VertexLayoutID::FireBallEffect,
        "Shaders/FireBallEffect.hlsl", "VS",
        "Shaders/FireBallEffect.hlsl", "PS",
        "Shaders/FireBallEffect.hlsl", "GS", false);
    RegisterShaderSource(ShaderSetID::UI, VertexLayoutID::UI,
        "Shaders/UI.hlsl", "VS",
        "Shaders/UI.hlsl", "PS");
    RegisterShaderSource(ShaderSetID::Debug, VertexLayoutID::Debug,
        "Shaders/DebugBox.hlsl", "VS",
        "Shaders/DebugBox.hlsl", "PS");

    // ===== シャドウシェーダーの登録 =====
    RegisterShadowShaderSource(ShaderSetID::StaticModel, VertexLayoutID::Static,
        "Shaders/DepthMap.hlsl", "VS");
    RegisterShadowShaderSource(ShaderSetID::SkinnedModel, VertexLayoutID::Skinned,
        "Shaders/DepthMap.hlsl", "SkinnedMeshVertexShaderPolygon");
    RegisterShadowShaderSource(ShaderSetID::Instanced_Tree, VertexLayoutID::Instanced,
        "Shaders/Tree.hlsl", "VSShadow", 
        nullptr, nullptr, 
        nullptr, nullptr, false);

    //RegisterComputeShaderSource(ShaderSetID::FireEffect, "Shaders/FireEffect.hlsl", "CS");
    RegisterComputeShaderSource(ShaderSetID::SmokeEffect, ComputePassType::Update,
        "Shaders/SmokeEffect.hlsl", "UpdateCS");
    RegisterComputeShaderSource(ShaderSetID::SmokeEffect, ComputePassType::Emit,
        "Shaders/SmokeEffect.hlsl", "EmitCS");
}


void ShaderManager::CreateInputLayouts(ID3D11Device* device)
{
    // Static モデル用 InputLayout
    //CreateStaticInputLayout(device);

    // Skinned モデル用 InputLayout
    //CreateSkinnedInputLayout(device);

    // Instanced モデル用 InputLayout
    CreateInstancedInputLayout(device);
}

void ShaderManager::LoadAllShaders(ID3D11Device* device)
{
    // 通常シェーダーを全て読み込む
    for (const auto& it : m_shaderSourcePaths)
    {
        ShaderSetID id = static_cast<ShaderSetID>(it.key);
        const ShaderSourcePathSet& info = it.value;

        ShaderSet set;

        if (info.useReflection)
        {
            // 反射を使用して InputLayout を自動生成
            ShaderLoader::LoadShaderSet(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                nullptr, 0, set);
        }
        else
        {
            // 外部で作成済みの InputLayout を使用
            ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                set);

            set.inputLayout = m_inputLayouts[TO_UINT64(info.layoutID)];
        }

        set.layoutID = it.value.layoutID;
        m_shaderSets[TO_UINT64(id)] = set;
    }

    // シャドウマップ用シェーダーを全て読み込む
    for (const auto& it : m_shadowShaderSourcePaths)
    {
        ShaderSetID id = static_cast<ShaderSetID>(it.key);
        const ShadowShaderSourcePathSet& info = it.value;

        ShadowShaderSet shadowSet;

        if (info.useReflection)
        {
            // 反射を使用して InputLayout を自動生成
            ShaderLoader::LoadShadowShaderSet(device,
                info.vsPath, info.psPath, info.psAlphaPath,
                info.vsEntry, info.psEntry, info.psAlphaEntry,
                nullptr, 0, shadowSet);
        }
        else
        {
            // 外部で作成済みの InputLayout を使用
            ShaderLoader::LoadShadowShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.psPath, info.psAlphaPath,
                info.vsEntry, info.psEntry, info.psAlphaEntry,
                shadowSet);
            shadowSet.inputLayout = m_inputLayouts[TO_UINT64(info.layoutID)];
        }

        m_shadowShaderSets[TO_UINT64(id)] = shadowSet;
    }

    for (auto& it : m_computeShaders)
    {
        const ComputeShaderSet& info = it.value;

        if (info.path && info.entry)
        {
            ID3DBlob* blob = nullptr;
            if (ShaderLoader::CompileShaderFromFile(info.path, info.entry, SHADER_MODEL_CS, &blob))
            {
                device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &it.value.cs);
                blob->Release();
            }
        }
    }


    for (const auto& it : m_particleShaderSourcePaths)
    {
        ParticleShaderGroup group = static_cast<ParticleShaderGroup>(it.key);
        const ShaderSourcePathSet& info = it.value;

        ShaderSet set;

        if (info.useReflection)
        {
            // 反射を使用して InputLayout を自動生成
            ShaderLoader::LoadShaderSet(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                nullptr, 0, set);
        }
        else
        {
            // 外部で作成済みの InputLayout を使用
            ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.vsEntry,
                info.psPath, info.psEntry,
                info.gsPath, info.gsEntry,
                set);

            set.inputLayout = m_inputLayouts[TO_UINT64(info.layoutID)];
        }

        set.layoutID = it.value.layoutID;
        m_particleShaderSets[TO_UINT64(group)] = set;
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

    m_reloader.WatchFile(vsPath);
    m_reloader.WatchFile(psPath);
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

    m_reloader.WatchFile(vsPath);
    m_reloader.WatchFile(psPath);
    m_reloader.WatchFile(psAlphaPath);
}

void ShaderManager::ReloadAllIfChanged(ID3D11Device* device)
{
    // 通常シェーダーのリロード
    for (const auto& it : m_shaderSourcePaths)
    {
        if (m_reloader.HasFileChanged(it.value.vsPath) || m_reloader.HasFileChanged(it.value.psPath))
        {
            ShaderSet set;
            if (it.value.useReflection)
            {
                ShaderLoader::LoadShaderSet(device,
                    it.value.vsPath, it.value.vsEntry,
                    it.value.psPath, it.value.psEntry,
                    it.value.gsPath, it.value.gsEntry,
                    nullptr, 0, set);
            }
            else
            {
                ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                    it.value.vsPath, it.value.vsEntry,
                    it.value.psPath, it.value.psEntry,
                    it.value.gsPath, it.value.gsEntry,
                    set);
                set.inputLayout = m_inputLayouts[it.key];
            }

            m_shaderSets[it.key] = set;
        }
    }

    // シャドウシェーダーのリロード
    for (const auto& it : m_shadowShaderSourcePaths)
    {
        if (m_reloader.HasFileChanged(it.value.vsPath) ||
            m_reloader.HasFileChanged(it.value.psPath) ||
            m_reloader.HasFileChanged(it.value.psAlphaPath))
        {
            ShadowShaderSet set;
            if (it.value.useReflection)
            {
                ShaderLoader::LoadShadowShaderSet(device,
                    it.value.vsPath, it.value.psPath, it.value.psAlphaPath,
                    it.value.vsEntry, it.value.psEntry, it.value.psAlphaEntry,
                    nullptr, 0, set);
            }
            else
            {
                ShaderLoader::LoadShadowShaderSetWithoutLayoutCreation(device,
                    it.value.vsPath, it.value.psPath, it.value.psAlphaPath,
                    it.value.vsEntry, it.value.psEntry, it.value.psAlphaEntry,
                    set);
            }
            m_shadowShaderSets[it.key] = set;
        }
    }
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
    ComputeShaderSetKey key{ id, pass };
    m_computeShaders[key] = ComputeShaderSet{ nullptr, path, entry };
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
    ComputeShaderSetKey key{ id, pass };
    return m_computeShaders.find(key) != m_computeShaders.end();
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
    ComputeShaderSetKey key{ id, pass };
    return m_computeShaders.at(key);
}

ID3D11InputLayout* ShaderManager::GetInputLayout(VertexLayoutID id) const
{
    // 手動で登録された InputLayout をハッシュテーブルから探す
    auto it = m_inputLayouts.find(TO_UINT64(id));
    if (it != m_inputLayouts.end())
    {
        return it->value;
    }

    // 見つからない場合、すべての ShaderSet を走査して layoutID が一致するものを探す
    for (const auto& pair : m_shaderSets)
    {
        const ShaderSet& shaderSet = pair.value;
        if (shaderSet.layoutID == id && shaderSet.inputLayout)
        {
            return shaderSet.inputLayout;
        }
    }

    // どちらにも存在しない場合は nullptr を返す
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

    // インスタンスデータ
    { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // OffsetPosition
    { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // Rotation (Quaternion)
    { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // initialBillboardRot (初期ビルボード回転)
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

    // 空のレイアウト（頂点要素なし）
    D3D11_INPUT_ELEMENT_DESC* emptyLayoutDESC = nullptr;
    // 要素数は0
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
