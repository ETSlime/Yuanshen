//=============================================================================
//
// �V�F�[�_�[�Z�b�g�ƃ��C�A�E�g�̓����Ǘ��E�z�b�g�����[�h�Ή� [ShaderManager.h]
// Author : 
// �S�`��E�e�E�v�Z�p�V�F�[�_�[�Z�b�g���W���Ǘ����A���C�A�E�g�����E
// �\�[�X�p�X�o�^�E�ύX���m�ɂ��z�b�g�����[�h�@�\��񋟂���
// 
//=============================================================================
#pragma once
#include "ShaderReloader.h"
#include "SingletonBase.h"
#include "SimpleArray.h"
#include "ShaderLoader.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TO_UINT64(entry)			static_cast<uint64_t>(entry)

//*********************************************************
// �\����
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

// �O���錾
enum class EffectType;

class ShaderManager : public SingletonBase<ShaderManager>
{
public:
    ~ShaderManager();

    void Init(ID3D11Device* device);

    const ShaderSet& GetShaderSet(ShaderSetID id) const;
    const ShaderSet& GetShaderSet(ParticleShaderGroup group) const;
    const ShadowShaderSet& GetShadowShaderSet(ShaderSetID id) const;
    const ComputeShaderSet& GetComputeShader(ShaderSetID id, ComputePassType pass) const;
    const ComputeShaderSet& GetComputeShader(ParticleComputeGroup group, ComputePassType pass) const;
    ID3D11InputLayout* GetInputLayout(VertexLayoutID id) const;
    void SetSharedShadowPixelShader(ID3D11PixelShader* sharedPS);

    bool HasShadowShaderSet(ShaderSetID id) const;
    bool HasShaderSet(ShaderSetID id) const;
    bool HasShaderSet(ParticleShaderGroup group) const;
    bool HasComputerShader(ShaderSetID id, ComputePassType pass) const;
    bool HasComputerShader(ParticleComputeGroup group, ComputePassType pass) const;

    // ���t���[���Ăяo���A�ύX���ꂽ�V�F�[�_�[�����o���ă����[�h�L���[�ɒǉ�����
    void CollectAndReloadShaders(ID3D11Device* device);

private:
    
    // ���ׂẴZ�b�g��ێ�����
    HashMap<uint64_t, ShaderSet, HashUInt64, EqualUInt64> m_shaderSets;
    HashMap<uint64_t, ShaderSet, HashUInt64, EqualUInt64> m_particleShaderSets;
    HashMap<uint64_t, ShadowShaderSet, HashUInt64, EqualUInt64> m_shadowShaderSets;
    HashMap<ComputeShaderSetKey, ComputeShaderSet, ShaderSetKeyHash, EqualShaderSetKey> m_computeShaders;
    HashMap<ComputeShaderSetKey, ComputeShaderSet, ShaderSetKeyHash, EqualShaderSetKey> m_particleComputeShaders;

    // �V�F�[�_�[�̃\�[�X�t�@�C���̃p�X��ێ�����
    HashMap<uint64_t, ShaderSourcePathSet, HashUInt64, EqualUInt64> m_shaderSourcePaths;
    HashMap<uint64_t, ShaderSourcePathSet, HashUInt64, EqualUInt64> m_particleShaderSourcePaths;
    HashMap<uint64_t, ShadowShaderSourcePathSet, HashUInt64, EqualUInt64> m_shadowShaderSourcePaths;

    // InputLayout��ێ�����
    HashMap<uint64_t, ID3D11InputLayout*, HashUInt64, EqualUInt64>m_inputLayouts;

    SimpleArray<ShaderReloadEntry> m_shaderReloadEntries;

    ID3D11PixelShader* m_sharedShadowPS = nullptr;
    ShaderReloader m_reloader;

    void RegisterShaderSourcePaths(void);
    void RegisterShaderReloadEntry(const char* path, const char* entry, const char* target, void** out, 
        ShaderCreateFunc func, ShaderGroup shaderGroup, uint64_t shaderSetID, ShaderType shaderType, ComputePassType pass);
    void SyncReloadedShaderToShaderSet(const ShaderReloadEntry& entry);
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

    void RegisterComputeShaderSource(ParticleComputeGroup group, ComputePassType pass,
        const char* path, const char* entry);

    void CreateInputLayouts(ID3D11Device* device);

    // Instanced ���f���p InputLayout
    void CreateInstancedInputLayout(ID3D11Device* device);

    // Skinned ���f���p InputLayout
    void CreateSkinnedInputLayout(ID3D11Device* device);

    // Static ���f���p InputLayout
    void CreateStaticInputLayout(ID3D11Device* device);

    // �G�t�F�N�g�p InputLayout
    void CreateEffectInputLayout(ID3D11Device* device, EffectType type);
};