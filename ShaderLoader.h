//=============================================================================
//
// ShaderLoader処理 [ShaderLoader.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
// HLSL シェーダーモデルのバージョンを統一管理するマクロ
#define SHADER_MODEL_VS "vs_4_0"
#define SHADER_MODEL_PS "ps_4_0"
#define SHADER_MODEL_GS "gs_4_0"
#define SHADER_MODEL_CS "cs_5_0" // AppendStructuredBufferは5.0以上が必要

// 着色器 ID を一括管理する列挙型
enum class ShaderSetID : uint64_t
{
    None,
    StaticModel,
    SkinnedModel,
    Instanced_Grass,
    Instanced_Tree,
    Skybox,
    VFX,
    WATER,
    SOFT_BODY,
    FireEffect,
    FireBallEffect,
    SmokeEffect,
    UI,
    Debug,
};

enum class ParticleShaderGroup : uint64_t
{
    None,

    BillboardSimple,
    BillboardFlipbook,
    BillboardSoftParticle,
    BillboardFlipbookSoft,
};

// コンピュートパスの種類を管理する列挙型
enum class ComputePassType : uint64_t
{
    Update,
    Emit,
    Sort,
    Blur,
    PreIntegration,
};

// 頂点レイアウト ID（InputLayout を管理するため）
enum class VertexLayoutID : uint64_t
{
    Static,
    Skinned,
    Instanced,
    Skybox,
    VFX,
    UI,
    Debug,

    //VertexIDOnly
    ParticleBillboard,
    FireEffect,
    SmokeEffect,
    FireBallEffect,
    SoftBody,
};


//*********************************************************
// 構造体
//*********************************************************
struct ShaderSet
{
    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11GeometryShader* gs = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;

    VertexLayoutID layoutID = VertexLayoutID::Static;
};

struct ShadowShaderSet
{
    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11PixelShader* alphaPs = nullptr;
    ID3D11InputLayout* inputLayout = nullptr;

    VertexLayoutID layoutID = VertexLayoutID::Static;
};

struct ComputeShaderSet 
{
    ID3D11ComputeShader* cs = nullptr;
    const char* path = nullptr;
    const char* entry = nullptr;

    VertexLayoutID layoutID = VertexLayoutID::Static;
};


class ShaderLoader
{
public:
    // Shader（VS + PS + InputLayout）
    static bool LoadShaderSet(ID3D11Device* device,
        const char* vsPath, const char* vsEntry,
        const char* psPath, const char* psEntry,
        const char* gsPath, const char* gsEntry,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        UINT numElements,
        ShaderSet& outShaderSet);
    static bool LoadShaderSetWithoutLayoutCreation(ID3D11Device* device,
        const char* vsPath, const char* vsEntry,
        const char* psPath, const char* psEntry,
        const char* gsPath, const char* gsEntry,
        ShaderSet& outShaderSet);

    // ShadowShaderSet（Alpha テスト含む）
    static bool LoadShadowShaderSet(ID3D11Device* device,
        const char* vsPath, const char* psPath, const char* psAlphaPath,
        const char* vsEntry, const char* psEntry, const char* psAlphaEntry,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        UINT numElements,
        ShadowShaderSet& outShaderSet);
    static bool LoadShadowShaderSetWithoutLayoutCreation(ID3D11Device* device,
        const char* vsPath, const char* psPath, const char* psAlphaPath,
        const char* vsEntry, const char* psEntry, const char* psAlphaEntry,
        ShadowShaderSet& outShaderSet);

    static bool CompileShaderFromFile(const char* fileName, const char* entryPoint, const char* target, ID3DBlob** blobOut);

    // 空のピクセルシェーダーを読み込む関数
    static bool LoadEmptyPixelShader(ID3D11Device* device, ID3D11PixelShader** outPixelShader, const char* fileName, const char* entryPoint);

private:

    static bool CreateInputLayoutFromReflection(ID3D11Device* device, ID3DBlob* vsBlob, ID3D11InputLayout** outLayout);

    static bool CompileAndCreateVertexShader(ID3D11Device* device, const char* path, const char* entry, ID3D11VertexShader** outVS, ID3DBlob** outVSBlob);
    static bool CompileAndCreatePixelShader(ID3D11Device* device, const char* path, const char* entry, ID3D11PixelShader** outPS);
    static bool CompileAndCreateGeometryShader(ID3D11Device* device, const char* path, const char* entry, ID3D11GeometryShader** outGS);
};