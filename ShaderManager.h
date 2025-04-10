//=============================================================================
//
// ShaderManager処理 [ShaderManager.h]
// Author : 
//
//=============================================================================
#pragma once
#include "ShaderReloader.h"
#include "ShaderLoader.h"
#include "SingletonBase.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TO_UINT64(entry)			static_cast<uint64_t>(entry)

//*********************************************************
// 構造体
//*********************************************************
struct ShaderSourcePathSet
{
    const char* vsPath = nullptr;
    const char* psPath = nullptr;
    const char* vsEntry = "main";
    const char* psEntry = "main";
    bool useReflection = false;
    VertexLayoutID layoutID = VertexLayoutID::Static;
};

struct ShadowShaderSourcePathSet
{
    const char* vsPath = nullptr;
    const char* psPath = nullptr;
    const char* psAlphaPath = nullptr;
    const char* vsEntry = "main";
    const char* psEntry = "main";
    const char* psAlphaEntry = "main";
    bool useReflection = false;
    VertexLayoutID layoutID = VertexLayoutID::Static;
};

class ShaderManager : public SingletonBase<ShaderManager>
{
public:
    void Init(ID3D11Device* device);

    // シェーダーのソースファイルに変更があればリロードする
    void ReloadAllIfChanged(ID3D11Device* device);

    const ShaderSet& GetShaderSet(ShaderSetID id) const;
    const ShadowShaderSet& GetShadowShaderSet(ShaderSetID id) const;
    ID3D11InputLayout* GetInputLayout(VertexLayoutID id) const;
    void SetSharedShadowPixelShader(ID3D11PixelShader* sharedPS);

    bool HasShadowShaderSet(ShaderSetID id) const;

private:
    HashMap<uint64_t, ShaderSet, HashUInt64, EqualUInt64> m_shaderSets;
    HashMap<uint64_t, ShadowShaderSet, HashUInt64, EqualUInt64> m_shadowShaderSets;
    HashMap<uint64_t, ShaderSourcePathSet, HashUInt64, EqualUInt64> m_shaderSourcePaths;
    HashMap<uint64_t, ShadowShaderSourcePathSet, HashUInt64, EqualUInt64> m_shadowShaderSourcePaths;
    HashMap<uint64_t, ID3D11InputLayout*, HashUInt64, EqualUInt64>m_inputLayouts;
    ID3D11PixelShader* m_sharedShadowPS = nullptr;
    ShaderReloader m_reloader;

    void RegisterShaderSourcePaths(void);

    void LoadAllShaders(ID3D11Device* device);
    void LoadSingleShaderSet(ID3D11Device* device, ShaderSetID id,
        const char* vsPath, const char* vsEntry, 
        const char* psPath, const char* psEntry,
        bool createInputLayout = true);
    void LoadShadowShaderSet(ID3D11Device* device, ShaderSetID id,
        const char* vsPath, const char* vsEntry, 
        const char* psPath = nullptr, const char* psEntry = nullptr,
        const char* psAlphaPath = nullptr, const char* psAlphaEntry = nullptr,
        bool createInputLayout = true);
    void RegisterShaderSource(ShaderSetID id, VertexLayoutID layoutID,
        const char* vsPath, const char* vsEntry,
        const char* psPath, const char* psEntry, 
        bool useReflection = true);
    void RegisterShadowShaderSource(ShaderSetID id, VertexLayoutID layoutID,
        const char* vsPath, const char* vsEntry, 
        const char* psPath = nullptr, const char* psEntry = nullptr,
        const char* psAlphaPath = nullptr, const char* psAlphaEntry = nullptr,
        bool useReflection = true);
    void CreateInputLayouts(ID3D11Device* device);
};