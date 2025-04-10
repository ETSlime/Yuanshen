//=============================================================================
//
// ShaderManager処理 [ShaderManager.cpp]
// Author : 
//
//=============================================================================
#include "ShaderManager.h"
#include "ShaderLoader.h"
#include "Renderer.h"
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
        "shader.hlsl", "VertexShaderPolygon",
        "shader.hlsl", "PixelShaderPolygon");
    RegisterShaderSource(ShaderSetID::SkinnedModel, VertexLayoutID::Skinned,
        "shader.hlsl", "SkinnedMeshVertexShaderPolygon",
        "shader.hlsl", "SkinnedMeshPixelShader");
    RegisterShaderSource(ShaderSetID::Instanced_Grass, VertexLayoutID::Instanced,
        "Grass.hlsl", "VS",
        "Grass.hlsl", "PS", false);
    RegisterShaderSource(ShaderSetID::Instanced_Tree, VertexLayoutID::Instanced,
        "Tree.hlsl", "VS",
        "Tree.hlsl", "PS", false);
    RegisterShaderSource(ShaderSetID::Skybox, VertexLayoutID::Skybox,
        "Skybox.hlsl", "VS",
        "Skybox.hlsl", "PS");
    RegisterShaderSource(ShaderSetID::VFX, VertexLayoutID::VFX,
        "VFX.hlsl", "VS",
        "VFX.hlsl", "PS");
    RegisterShaderSource(ShaderSetID::UI, VertexLayoutID::UI,
        "UI.hlsl", "VS",
        "UI.hlsl", "PS");

    // ===== シャドウシェーダーの登録 =====
    RegisterShadowShaderSource(ShaderSetID::StaticModel, VertexLayoutID::Static,
        "DepthMap.hlsl", "VS");
    RegisterShadowShaderSource(ShaderSetID::SkinnedModel, VertexLayoutID::Skinned,
        "DepthMap.hlsl", "SkinnedMeshVertexShaderPolygon");
    RegisterShadowShaderSource(ShaderSetID::Instanced_Tree, VertexLayoutID::Instanced,
        "Tree.hlsl", "VSShadow", false);
}


void ShaderManager::CreateInputLayouts(ID3D11Device* device)
{
    //// Static モデル用 InputLayout
    //{
    //    D3D11_INPUT_ELEMENT_DESC layout[] = {
    //        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //    };

    //    ID3DBlob* vsBlob = nullptr;
    //    ShaderLoader::CompileShaderFromFile("Static_VS.hlsl", "main", "vs_4_0", &vsBlob);
    //    ID3D11InputLayout* inputLayout = nullptr;
    //    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    //    vsBlob->Release();

    //    m_inputLayouts[TO_UINT64(VertexLayoutID::Static)] = inputLayout;
    //}

    //// Skinned モデル用 InputLayout
    //{
    //    D3D11_INPUT_ELEMENT_DESC layout[] = {
    //        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //        { "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //        { "BONEINDEX",0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //    };

    //    ID3DBlob* vsBlob = nullptr;
    //    ShaderLoader::CompileShaderFromFile("Skinned_VS.hlsl", "main", "vs_4_0", &vsBlob);
    //    ID3D11InputLayout* inputLayout = nullptr;
    //    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    //    vsBlob->Release();

    //    m_inputLayouts[TO_UINT64(VertexLayoutID::Skinned)] = inputLayout;
    //}

    // Instanced モデル用 InputLayout
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
        ShaderLoader::CompileShaderFromFile("Tree.hlsl", "VS", "vs_4_0", &vsBlob);
        ID3D11InputLayout* inputLayout = nullptr;
        device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
        vsBlob->Release();

        m_inputLayouts[TO_UINT64(VertexLayoutID::Instanced)] = inputLayout;
    }
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
                info.vsPath, info.psPath,
                info.vsEntry, info.psEntry,
                nullptr, 0, set);
        }
        else
        {
            // 外部で作成済みの InputLayout を使用
            ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                info.vsPath, info.psPath,
                info.vsEntry, info.psEntry,
                set);
            set.inputLayout = m_inputLayouts[TO_UINT64(id)];
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
            shadowSet.inputLayout = m_inputLayouts[TO_UINT64(id)];
        }

        m_shadowShaderSets[TO_UINT64(id)] = shadowSet;
    }
}

void ShaderManager::LoadSingleShaderSet(ID3D11Device* device, ShaderSetID id,
    const char* vsPath, const char* vsEntry, 
    const char* psPath, const char* psEntry,
    bool createInputLayout)
{
    ShaderSet set;
    set.ps = m_sharedShadowPS;
    if (createInputLayout)
    {
        ShaderLoader::LoadShaderSet(device, vsPath, vsEntry, psPath, psEntry, nullptr, 0, set);
    }
    else
    {
        ShaderLoader::LoadShaderSetWithoutLayoutCreation(device, vsPath, vsEntry, psPath, psEntry, set);
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
                    it.value.vsPath, it.value.psPath,
                    it.value.vsEntry, it.value.psEntry,
                    nullptr, 0, set);
            }
            else
            {
                ShaderLoader::LoadShaderSetWithoutLayoutCreation(device,
                    it.value.vsPath, it.value.psPath,
                    it.value.vsEntry, it.value.psEntry,
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

void ShaderManager::RegisterShaderSource(ShaderSetID id, VertexLayoutID layoutID, const char* vsPath, const char* vsEntry, const char* psPath, const char* psEntry, bool useReflection)
{
    m_shaderSourcePaths[TO_UINT64(id)] = ShaderSourcePathSet{
    vsPath, psPath, vsEntry, psEntry, useReflection, layoutID
    };
}

void ShaderManager::RegisterShadowShaderSource(ShaderSetID id, VertexLayoutID layoutID, const char* vsPath, const char* vsEntry, const char* psPath, const char* psEntry, const char* psAlphaPath, const char* psAlphaEntry, bool useReflection)
{
    m_shadowShaderSourcePaths[TO_UINT64(id)] = ShadowShaderSourcePathSet{
    vsPath, psPath, psAlphaPath, vsEntry, psEntry, psAlphaEntry, useReflection, layoutID
    };
}

void ShaderManager::SetSharedShadowPixelShader(ID3D11PixelShader* sharedPS)
{
    m_sharedShadowPS = sharedPS;
}

bool ShaderManager::HasShadowShaderSet(ShaderSetID id) const
{
    return m_shadowShaderSets.find(TO_UINT64(id)) != m_shadowShaderSets.end();
}

const ShaderSet& ShaderManager::GetShaderSet(ShaderSetID id) const
{
    return m_shaderSets.at(TO_UINT64(id));
}

const ShadowShaderSet& ShaderManager::GetShadowShaderSet(ShaderSetID id) const
{
    return m_shadowShaderSets.at(TO_UINT64(id));
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

