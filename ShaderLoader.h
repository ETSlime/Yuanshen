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

// ���F�� ID ���ꊇ�Ǘ�����񋓌^
enum class ShaderSetID : uint64_t
{
    StaticModel,
    SkinnedModel,
    Instanced_Grass,
    Instanced_Tree,
    Skybox,
    VFX,
    WATER,
    SOFT_BODY,
    EXPLOSION,
    UI,
    Debug,
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
};


//*********************************************************
// �\����
//*********************************************************
struct ShaderSet
{
    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
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
class ShaderLoader
{
public:
    // Shader�iVS + PS + InputLayout�j
    static bool LoadShaderSet(ID3D11Device* device,
        const char* vsPath, const char* psPath,
        const char* vsEntry, const char* psEntry,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        UINT numElements,
        ShaderSet& outShaderSet);
    static bool LoadShaderSetWithoutLayoutCreation(ID3D11Device* device,
        const char* vsPath, const char* psPath,
        const char* vsEntry, const char* psEntry,
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

    static bool CompileShaderFromFile(const char* fileName, const char* entryPoint, const char* target, ID3DBlob** blobOut);

    // ��̃s�N�Z���V�F�[�_�[��ǂݍ��ފ֐�
    static bool LoadEmptyPixelShader(ID3D11Device* device, ID3D11PixelShader** outPixelShader, const char* fileName, const char* entryPoint);

private:

    static bool CreateInputLayoutFromReflection(ID3D11Device* device, ID3DBlob* vsBlob, ID3D11InputLayout** outLayout);

    static bool CompileAndCreateVertexShader(ID3D11Device* device, const char* path, const char* entry, ID3D11VertexShader** outVS, ID3DBlob** outVSBlob);
    static bool ShaderLoader::CompileAndCreatePixelShader(ID3D11Device* device, const char* path, const char* entry, ID3D11PixelShader** outPS);
};