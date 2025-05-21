//=============================================================================
//
// �V�F�[�_�[�t�@�C���̕ύX���m�ƃz�b�g�����[�h�@�\ [ShaderReloader.h]
// Author : 
// �t�@�C���X�V�����E�o�C�i���n�b�V�����r���A�G���g���P�ʂł�
// �������o�Ɠ��I�Đ����i�z�b�g�����[�h�j���T�|�[�g���郆�[�e�B���e�B
// 
//=============================================================================
#pragma once
#include "Utility/HashMap.h"
#include "Core/Shader/ShaderLoader.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************

//==============================================================
// REGISTER_SHADER_RELOAD_ENTRY �}�N����`
// - shaderType �� D3D11 �V�F�[�_�[�N���X���iVertex, Pixel, Compute, Geometry�j
// - ptr �� ID3D11XXXShader* �^�̕ϐ�
// - target �� "vs_5_0" �Ȃ�
//==============================================================
#define REGISTER_SHADER_RELOAD_ENTRY(manager, shaderGroup, shaderSetID, shaderType, computePass, shaderTypeName, path, entry, target, ptr)  \
    manager.RegisterShaderReloadEntry(                                                                  \
        path,                                                                                           \
        entry,                                                                                          \
        target,                                                                                         \
        reinterpret_cast<void**>(ptr),                                                                  \
        [](ID3D11Device* dev, const void* data, SIZE_T size, void** out) -> HRESULT {                   \
            return dev->Create##shaderTypeName##Shader(data, size, nullptr,                             \
                reinterpret_cast<ID3D11##shaderTypeName##Shader**>(out));                               \
        },                                                                                              \
        shaderGroup,                                                                                    \
        TO_UINT64(shaderSetID),                                                                         \
        shaderType,                                                                                     \
        computePass)

enum class ShaderGroup : uint8_t
{
    Default,
    Particle,
    Shadow,
    Compute,
    ParticleCompute
};

enum class ShaderType : uint8_t
{
    VS, GS, PS, PS_ALPHA, CS
};

//*********************************************************
// �\����
//*********************************************************

// �V�F�[�_�[�Đ����֐��|�C���^�^
typedef HRESULT(*ShaderCreateFunc)(ID3D11Device* device, const void* bytecode, SIZE_T size, void** outShader);

struct ShaderReloadEntry
{
    const char* path;                   // �V�F�[�_�[�t�@�C��
    const char* entry;                  // �G���g���[�|�C���g
    const char* target;                 // �R���p�C���^�[�Q�b�g (vs_5_0 / ps_5_0)
    uint64_t shaderSetID;
    ShaderGroup shaderGroup;
    ShaderType  shaderType;
    ComputePassType computePass;
    void** outputShaderPtr;             // �o�͐�̃V�F�[�_�[�|�C���^ (void**)
    ShaderCreateFunc createShaderFunc;  // �V�F�[�_�[�����֐�
};

struct ShaderSourceKey
{
    // path + entry ���L�[��
    const char* path;
    const char* entry;

    bool operator==(const ShaderSourceKey& other) const
    {
        return strcmp(path, other.path) == 0 && strcmp(entry, other.entry) == 0;
    }
};

struct ShaderSourceKeyHash
{
    unsigned int operator()(const ShaderSourceKey& key) const
    {
        // DJB2 �n�b�V���֐����g���� path �� entry �������n�b�V��
        unsigned long hash = 5381;

        const char* p = key.path;
        while (p && *p)
        {
            hash = ((hash << 5) + hash) + *p++;
        }

        const char* e = key.entry;
        while (e && *e)
        {
            hash = ((hash << 5) + hash) + *e++;
        }

        return static_cast<unsigned int>(hash);
    }
};

// ������r���Z�q
struct ShaderSourceKeyEquals
{
    bool operator()(const ShaderSourceKey& a, const ShaderSourceKey& b) const
    {
        return strcmp(a.path, b.path) == 0 && strcmp(a.entry, b.entry) == 0;
    }
};


struct FileState
{
    FILETIME lastWriteTime;
    uint64_t contentHash;
};

class ShaderReloader
{
public:
    // �t�@�C�����Ď��Ώۂɒǉ�����
    void WatchFile(const char* path);

    // �t�@�C�����ύX���ꂽ�����`�F�b�N����
    bool HasFileChanged(const char* path);

    //void AdvanceFrame(void);

    bool HasShaderContentChanged(const char* path, const char* entry, const void* blob, size_t size);

private:

    //bool GetFileHashAndTime(const char* path, FILETIME& outTime, uint64_t& outHash);
    uint64_t CalculateBlobHash(const void* data, size_t size);
    uint64_t CalculateFileHash(const char* path);

    // �t�@�C���p�X�ƍŏI�X�V�����̃}�b�v
    HashMap<const char*, FileState, CharPtrHash, CharPtrEquals> m_fileStateMap;
    // entry���x����blob��r
    HashMap<ShaderSourceKey, uint64_t, ShaderSourceKeyHash, ShaderSourceKeyEquals> m_blobHashMap;
    // �t���[�����Ƃ� fallback �`�F�b�N�p
    int m_frameCount = 0;
    static constexpr int kHashFallbackInterval = 30; // 30�t���[������  fallback hash �`�F�b�N
};
