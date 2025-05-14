//=============================================================================
//
// ShaderLoader���� [ShaderLoader.h]
// Author : 
//
//=============================================================================
#pragma once
#include "main.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
// HLSL �V�F�[�_�[���f���̃o�[�W�����𓝈�Ǘ�����}�N��
#define SHADER_MODEL_VS "vs_4_0"
#define SHADER_MODEL_PS "ps_4_0"
#define SHADER_MODEL_GS "gs_4_0"
#define SHADER_MODEL_CS "cs_5_0" // AppendStructuredBuffer��5.0�ȏオ�K�v

// ���F�� ID ���ꊇ�Ǘ�����񋓌^
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
    Count,
};

enum class ParticleComputeGroup : uint64_t
{
    None,
    BasicBillboard,          // BillboardSimple
    FlipbookAnimated,        // BillboardFlipbook
    FlipbookWithSoft,        // BillboardFlipbookSoft
};

// �R���s���[�g�p�X�̎�ނ��Ǘ�����񋓌^
enum class ComputePassType : uint64_t
{
    None,
    Update,
    Emit,
    Sort,
    Blur,
    PreIntegration,
};

// ���_���C�A�E�g ID�iInputLayout ���Ǘ����邽�߁j
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
    ParticleSimple,
    ParticleFlipbook,
    FireEffect,
    SmokeEffect,
    FireBallEffect,
    SoftBody,
};


//*********************************************************
// �\����
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

    ComputePassType pass = ComputePassType::None;
    uint64_t shaderSetID = 0;
};

struct ComputeShaderSetKey
{
    uint64_t shaderID;
    ComputePassType pass;
};

struct ShaderSetKeyHash
{
    size_t operator()(const ComputeShaderSetKey& k) const
    {
        return (static_cast<size_t>(k.shaderID) << 8) ^ static_cast<size_t>(k.pass);
    }
};

struct EqualShaderSetKey
{
    bool operator()(const ComputeShaderSetKey& a, const ComputeShaderSetKey& b) const
    {
        return a.shaderID == b.shaderID && a.pass == b.pass;
    }
};


class ShaderLoader
{
public:
    // Shader�iVS + PS + InputLayout�j
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

    // ShadowShaderSet�iAlpha �e�X�g�܂ށj
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

    // ComputerShaderSet
    static bool LoadComputerShaderSet(ID3D11Device* device, ComputeShaderSet& outShaderSet);

    // D3DX11CompileFromFile �����̂܂܎g���i�񐄏��E�L���b�V������j
    static bool CompileShaderFromFileLegacy(const char* fileName, const char* entryPoint, const char* target, ID3DBlob** blobOut);
    // ������ fread �� D3DCompile�i�ŐV�ŁE�L���b�V���΍�ς݁j
    static bool CompileShaderFromFile(const char* fileName, const char* entryPoint, const char* target, ID3DBlob** blobOut, ID3DBlob** errorBlob = nullptr);

    // ��̃s�N�Z���V�F�[�_�[��ǂݍ��ފ֐�
    static bool LoadEmptyPixelShader(ID3D11Device* device, ID3D11PixelShader** outPixelShader, const char* fileName, const char* entryPoint);

private:

    static bool CreateInputLayoutFromReflection(ID3D11Device* device, ID3DBlob* vsBlob, ID3D11InputLayout** outLayout);

    static bool CompileAndCreateVertexShader(ID3D11Device* device, const char* path, const char* entry, ID3D11VertexShader** outVS, ID3DBlob** outVSBlob);
    static bool CompileAndCreatePixelShader(ID3D11Device* device, const char* path, const char* entry, ID3D11PixelShader** outPS);
    static bool CompileAndCreateGeometryShader(ID3D11Device* device, const char* path, const char* entry, ID3D11GeometryShader** outGS);
    static bool CompileAndCreateComputeShader(ID3D11Device* device, const char* path, const char* entry, ID3D11ComputeShader** outCS);
};