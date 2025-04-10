//=============================================================================
//
// レンダリング処理 [CascadedShadowMap.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
// シャドウマップ解像度（1カスケードごとのサイズ）
#define CSM_SHADOW_MAP_SIZE 2048
// カスケードの数（最大4）
#define CSM_CASCADE_COUNT 4

enum class ShadowRadiusStrategy
{
    FitBoundingSphere,
    FitBoundingBox,
    PaddingBuffer
};

//*********************************************************
// 構造体
//*********************************************************
struct CascadedShadowConfig
{
    int numCascades = CSM_CASCADE_COUNT;
    float manualSplits[CSM_CASCADE_COUNT] = { 0.05f, 0.15f, 0.3f, 1.0f };
    float lambda = 0.95f;

    // スプリットの深度を手動で指定するかどうか
    bool useManualSplits = false;
    // テクセルスナップを有効化するかどうか
    bool enableTexelSnap = true;
    // スナップの精度を指定する
    ShadowRadiusStrategy radiusStrategy = ShadowRadiusStrategy::FitBoundingSphere;
    // カスケードの間隔を調整するためのパディング
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

    // シャドウマップのテクセルグリッドにスナップする補正関数
    XMMATRIX SnapShadowMatrixToTexelGrid(const XMMATRIX& lightView, const XMMATRIX& lightProj, XMVECTOR center);

    // カスケードシャドウマップの半径を計算するユーティリティ関数
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
