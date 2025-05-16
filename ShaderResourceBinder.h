//=============================================================================
//
// �萔�o�b�t�@�ESRV�EUAV �̍œK�o�C���h�Ǘ� [ShaderResourceBinder.h]
// Author : 
// �e�V�F�[�_�[�X�e�[�W�ɂ����郊�\�[�X�̃o�C���h��Ԃ�ǐՂ��A
// �璷�ȃo�C���h��������ăp�t�H�[�}���X���œK�����郆�[�e�B���e�B
// 
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_CB_SLOT_NUM             16
#define MAX_SRV_SLOT_NUM            128
#define MAX_SAMPLER_SLOT_NUM        16
#define MAX_UAV_SLOT_NUM            8

// ===== �X���b�g��` =====
#define SLOT_CB_WORLD_MATRIX        0
#define SLOT_CB_VIEW_MATRIX         1
#define SLOT_CB_PROJECTION_MATRIX   2
#define SLOT_CB_MATERIAL            3
#define SLOT_CB_LIGHT               4
#define SLOT_CB_FOG                 5    
#define SLOT_CB_EFFECT_UPDATE       5 // ���q�X�V�p
#define SLOT_CB_EFFECT_DRAW         6 // ���q�`��p
#define SLOT_CB_FUCHI               6   
#define SLOT_CB_CASCADE_DATA_ARRAY  6
#define SLOT_CB_CAMERA_POS          7
#define SLOT_CB_EFFECT_PARTICLE     7
#define SLOT_CB_CASCADE_DATA        8
#define SLOT_CB_SKYBOX              9  
#define SLOT_CB_DEBUG_BOUNDING_BOX  9  
#define SLOT_CB_LIGHT_MODE          10 
#define SLOT_CB_BONE_MATRIX_ARRAY   11 
#define SLOT_CB_INSTANCED_DATA      12
#define SLOT_CB_RENDER_PROGRESS     13

// ===== �T���v���[�X���b�g��` =====
#define SLOT_SAMPLER_DEFAULT        0
#define SLOT_SAMPLER_SHADOW         1
#define SLOT_SAMPLER_OPACITY        2

// ===== �e�N�X�`���X���b�g��` =====
#define SLOT_TEX_DIFFUSE            0
#define SLOT_TEX_CSM                1
#define SLOT_TEX_NOISE              7
#define SLOT_TEX_LIGHT              8
#define SLOT_TEX_NORMAL             9
#define SLOT_TEX_BUMP               10
#define SLOT_TEX_OPACITY            11
#define SLOT_TEX_REFLECT            12
#define SLOT_TEX_TRANSLUCENCY       13
#define SLOT_TEX_SKYBOX_DAY         15
#define SLOT_TEX_SKYBOX_NIGHT       21

// ===== �X�g���N�`���[�h�o�b�t�@��` =====
#define SLOT_SRV_PARTICLE           6   // �p�[�e�B�N���ǂݎ��o�b�t�@
#define SLOT_SRV_ALIVE_LIST         7   // �p�[�e�B�N���������X�g
// ===== RW�X�g���N�`���[�h�o�b�t�@��` =====
#define SLOT_UAV_PARTICLE           0   // �p�[�e�B�N���X�V�o�b�t�@
#define SLOT_UAV_ALIVE_LIST         1   // �p�[�e�B�N���������X�g
#define SLOT_UAV_FREE_LIST          2   // �p�[�e�B�N���t���[���X�g
#define SLOT_UAV_FREE_LIST_CONSUME  3
// ===== �V�F�[�_�[�X�e�[�W��` =====
enum class ShaderStage
{
    VS,
    PS,
    GS,
    CS,
};

class ShaderResourceBinder : public SingletonBase<ShaderResourceBinder>
{
public:

    void Initialize(ID3D11DeviceContext* context);
    void Reset();

    // �萔�o�b�t�@�o�C���h
    void BindConstantBuffer(ShaderStage stage, UINT slot, ID3D11Buffer* buffer);

    // SRV�o�C���h
    void BindShaderResource(ShaderStage stage, UINT slot, ID3D11ShaderResourceView* srv);

    // �T���v���[�o�C���h
    void BindSampler(ShaderStage stage, UINT slot, ID3D11SamplerState* sampler);

    // ===== �o�b�`�o�C���h�i���������j=====
    void BindConstantBuffers(ShaderStage stage, UINT startSlot, UINT numBuffers, ID3D11Buffer* const* buffers);
    void BindShaderResources(ShaderStage stage, UINT startSlot, UINT numSRVs, ID3D11ShaderResourceView* const* srvs);
    void BindSamplers(ShaderStage stage, UINT startSlot, UINT numSamplers, ID3D11SamplerState* const* samplers);

    // ===== UAV�o�C���h�iCompute Shader��p�j=====
    void BindUnorderedAccessView(UINT slot, ID3D11UnorderedAccessView* uav);
    void BindUnorderedAccessViews(UINT startSlot, UINT numUAVs, ID3D11UnorderedAccessView* const* uavs);
    void BindCounterUnorderedAccessView(UINT slot, ID3D11UnorderedAccessView* uav, UINT initialCount = 0);

private:

    ID3D11DeviceContext* m_context = nullptr;

    // ���݂̃o�C���h���
    ID3D11Buffer* m_currentCB_VS[MAX_CB_SLOT_NUM]{};
    ID3D11Buffer* m_currentCB_PS[MAX_CB_SLOT_NUM]{};
    ID3D11Buffer* m_currentCB_GS[MAX_CB_SLOT_NUM]{};
    ID3D11Buffer* m_currentCB_CS[MAX_CB_SLOT_NUM]{};

    ID3D11ShaderResourceView* m_currentSRV_VS[MAX_SRV_SLOT_NUM]{};
    ID3D11ShaderResourceView* m_currentSRV_PS[MAX_SRV_SLOT_NUM]{};
    ID3D11ShaderResourceView* m_currentSRV_GS[MAX_SRV_SLOT_NUM]{};
    ID3D11ShaderResourceView* m_currentSRV_CS[MAX_SRV_SLOT_NUM]{};

    ID3D11SamplerState* m_currentSampler_VS[MAX_SAMPLER_SLOT_NUM]{};
    ID3D11SamplerState* m_currentSampler_PS[MAX_SAMPLER_SLOT_NUM]{};
    ID3D11SamplerState* m_currentSampler_GS[MAX_SAMPLER_SLOT_NUM]{};
    ID3D11SamplerState* m_currentSampler_CS[MAX_SAMPLER_SLOT_NUM]{};

    ID3D11UnorderedAccessView* m_currentUAV_CS[MAX_UAV_SLOT_NUM]{}; // CS��pUAV�o�C���h�ǐ�

    // �_�[�e�B�t���O
    bool m_dirtySRV_VS[MAX_SRV_SLOT_NUM]{};
    bool m_dirtySRV_PS[MAX_SRV_SLOT_NUM]{};
    bool m_dirtySRV_GS[MAX_SRV_SLOT_NUM]{};
    bool m_dirtySRV_CS[MAX_SRV_SLOT_NUM]{};
    bool m_dirtyUAV_CS[MAX_UAV_SLOT_NUM]{};

    // �����w���p
    ID3D11Buffer** GetCBSlotArray(ShaderStage stage);
    ID3D11ShaderResourceView** GetSRVSlotArray(ShaderStage stage);
    ID3D11SamplerState** GetSamplerSlotArray(ShaderStage stage);
    bool* GetSRVDirtyArray(ShaderStage stage);
};