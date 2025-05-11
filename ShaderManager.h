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
    const char* gsPath = nullptr;
    const char* vsEntry = "main";
    const char* psEntry = "main";
    const char* gsEntry = "main";
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

struct ComputeShaderSetKey
{
    ShaderSetID id;
    ComputePassType pass;
};

struct ShaderSetKeyHash 
{
    size_t operator()(const ComputeShaderSetKey& k) const
    {
        return (static_cast<size_t>(k.id) << 8) ^ static_cast<size_t>(k.pass);
    }
};

struct EqualShaderSetKey
{
    bool operator()(const ComputeShaderSetKey& a, const ComputeShaderSetKey& b) const
    {
        return a.id == b.id && a.pass == b.pass;
    }
};

// 前方宣言
enum class EffectType;

class ShaderManager : public SingletonBase<ShaderManager>
{
public:
    ~ShaderManager();

    void Init(ID3D11Device* device);

    // シェーダーのソースファイルに変更があればリロードする
    void ReloadAllIfChanged(ID3D11Device* device);

    const ShaderSet& GetShaderSet(ShaderSetID id) const;
    const ShaderSet& GetShaderSet(ParticleShaderGroup group) const;
    const ShadowShaderSet& GetShadowShaderSet(ShaderSetID id) const;
    const ComputeShaderSet& GetComputeShader(ShaderSetID id, ComputePassType pass) const;
    ID3D11InputLayout* GetInputLayout(VertexLayoutID id) const;
    void SetSharedShadowPixelShader(ID3D11PixelShader* sharedPS);

    bool HasShadowShaderSet(ShaderSetID id) const;
    bool HasShaderSet(ShaderSetID id) const;
    bool HasShaderSet(ParticleShaderGroup group) const;
    bool HasComputerShader(ShaderSetID id, ComputePassType pass) const;

private:
    
    // すべてのセットを保持する
    HashMap<uint64_t, ShaderSet, HashUInt64, EqualUInt64> m_shaderSets;
    HashMap<uint64_t, ShaderSet, HashUInt64, EqualUInt64> m_particleShaderSets;
    HashMap<uint64_t, ShadowShaderSet, HashUInt64, EqualUInt64> m_shadowShaderSets;
    HashMap<ComputeShaderSetKey, ComputeShaderSet, ShaderSetKeyHash, EqualShaderSetKey> m_computeShaders;

    // シェーダーのソースファイルのパスを保持する
    HashMap<uint64_t, ShaderSourcePathSet, HashUInt64, EqualUInt64> m_shaderSourcePaths;
    HashMap<uint64_t, ShaderSourcePathSet, HashUInt64, EqualUInt64> m_particleShaderSourcePaths;
    HashMap<uint64_t, ShadowShaderSourcePathSet, HashUInt64, EqualUInt64> m_shadowShaderSourcePaths;

    // InputLayoutを保持する
    HashMap<uint64_t, ID3D11InputLayout*, HashUInt64, EqualUInt64>m_inputLayouts;

    ID3D11PixelShader* m_sharedShadowPS = nullptr;
    ShaderReloader m_reloader;

    void RegisterShaderSourcePaths(void);

    void LoadAllShaders(ID3D11Device* device);

    void LoadSingleShaderSet(ID3D11Device* device, ShaderSetID id,
        const char* vsPath, const char* vsEntry, 
        const char* psPath, const char* psEntry,
        const char* gsPath, const char* gsEntry,
        bool createInputLayout = true);

    void LoadShadowShaderSet(ID3D11Device* device, ShaderSetID id,
        const char* vsPath, const char* vsEntry, 
        const char* psPath = nullptr, const char* psEntry = nullptr,
        const char* psAlphaPath = nullptr, const char* psAlphaEntry = nullptr,
        bool createInputLayout = true);

    void RegisterShaderSource(ShaderSetID id, VertexLayoutID layoutID,
        const char* vsPath, const char* vsEntry,
        const char* psPath, const char* psEntry, 
        const char* gsPath = nullptr, const char* gsEntry = nullptr,
        bool useReflection = true);

    void RegisterShaderSource(ParticleShaderGroup group, VertexLayoutID layoutID,
        const char* vsPath, const char* vsEntry,
        const char* psPath, const char* psEntry,
        const char* gsPath = nullptr, const char* gsEntry = nullptr,
        bool useReflection = true);

    void RegisterShadowShaderSource(ShaderSetID id, VertexLayoutID layoutID,
        const char* vsPath, const char* vsEntry, 
        const char* psPath = nullptr, const char* psEntry = nullptr,
        const char* psAlphaPath = nullptr, const char* psAlphaEntry = nullptr,
        bool useReflection = true);
    
    void RegisterComputeShaderSource(ShaderSetID id, ComputePassType pass, 
        const char* path, const char* entry);

    void CreateInputLayouts(ID3D11Device* device);

    // Instanced モデル用 InputLayout
    void CreateInstancedInputLayout(ID3D11Device* device);

    // Skinned モデル用 InputLayout
    void CreateSkinnedInputLayout(ID3D11Device* device);

    // Static モデル用 InputLayout
    void CreateStaticInputLayout(ID3D11Device* device);

    // エフェクト用 InputLayout
    void CreateEffectInputLayout(ID3D11Device* device, EffectType type);
};