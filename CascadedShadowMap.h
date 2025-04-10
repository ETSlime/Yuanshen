//=============================================================================
//
// �����_�����O���� [CascadedShadowMap.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
// �V���h�E�}�b�v�𑜓x�i1�J�X�P�[�h���Ƃ̃T�C�Y�j
#define CSM_SHADOW_MAP_SIZE 2048
// �J�X�P�[�h�̐��i�ő�4�j
#define CSM_CASCADE_COUNT 4

enum class ShadowRadiusStrategy
{
    FitBoundingSphere,
    FitBoundingBox,
    PaddingBuffer
};

//*********************************************************
// �\����
//*********************************************************
struct CascadedShadowConfig
{
    int numCascades = CSM_CASCADE_COUNT;
    float manualSplits[CSM_CASCADE_COUNT] = { 0.05f, 0.15f, 0.3f, 1.0f };
    float lambda = 0.95f;

    // �X�v���b�g�̐[�x���蓮�Ŏw�肷�邩�ǂ���
    bool useManualSplits = false;
    // �e�N�Z���X�i�b�v��L�������邩�ǂ���
    bool enableTexelSnap = true;
    // �X�i�b�v�̐��x���w�肷��
    ShadowRadiusStrategy radiusStrategy = ShadowRadiusStrategy::FitBoundingSphere;
    // �J�X�P�[�h�̊Ԋu�𒲐����邽�߂̃p�f�B���O
    float padding = 0.0f;
};

class CascadedShadowMap 
{
public:
    static const int MaxCascades = 4;

    struct CascadeData 
    {
        XMMATRIX lightViewProj;
        float splitDepth;
    };

    CascadedShadowMap();
    ~CascadedShadowMap();

    void Initialize(int shadowMapSize, int numCascades = 4);
    void Shutdown(void);

    void UpdateCascades(const XMMATRIX& cameraView, const XMMATRIX& cameraProj,
        const XMVECTOR& lightDir, float nearZ, float farZ);

    void BeginCascadeRender(int cascadeIndex);
    void EndCascadeRender(void);
    void SetViewport(void);

    ID3D11ShaderResourceView* GetShadowSRV(int index) const { return m_srvs[index]; }
    ID3D11DepthStencilView* GetDSV(int index) const { return m_dsvs[index]; }
    const CascadeData& GetCascadeData(int index) const { return m_cascadeData[index]; }
    const CascadeData& GetCurrentCascadeData(void) { return m_cascadeData[m_currentCascadeIndex]; }

    int GetCascadeCount() const { return m_numCascades; }

private:
    void ComputeCascadeMatrices(int index, float splitNear, float splitFar,
        const XMMATRIX& camView, const XMMATRIX& camProj, const XMVECTOR& lightDir);

    // �V���h�E�}�b�v�̃e�N�Z���O���b�h�ɃX�i�b�v����␳�֐�
    XMMATRIX SnapShadowMatrixToTexelGrid(const XMMATRIX& lightView, const XMMATRIX& lightProj, XMVECTOR center);

    // �J�X�P�[�h�V���h�E�}�b�v�̔��a���v�Z���郆�[�e�B���e�B�֐�
    float ComputeCascadeRadius(XMVECTOR* corners, XMVECTOR center, ShadowRadiusStrategy strategy, float padding);

    int m_shadowMapSize = 1024;
    int m_numCascades = 4;
    int m_currentCascadeIndex = 0;

    float m_lastCascadeRadius;
    XMVECTOR m_lastLightDir;

    CascadedShadowConfig m_config;

    ID3D11Texture2D* m_shadowMaps[MaxCascades] = {};
    ID3D11DepthStencilView* m_dsvs[MaxCascades] = {};
    ID3D11ShaderResourceView* m_srvs[MaxCascades] = {};

    D3D11_VIEWPORT m_shadowViewport = {};

    CascadeData m_cascadeData[MaxCascades];

    // Store previous render targets
    ID3D11RenderTargetView* m_prevRTV = nullptr;
    ID3D11DepthStencilView* m_prevDSV = nullptr;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
};
