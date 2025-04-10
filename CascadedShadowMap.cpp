//=============================================================================
//
// レンダリング処理 [CascadedShadowMap.cpp]
// Author : 
//
//=============================================================================
#include "CascadedShadowMap.h"

CascadedShadowMap::CascadedShadowMap() {}
CascadedShadowMap::~CascadedShadowMap() { Shutdown(); }

void CascadedShadowMap::Initialize(int shadowMapSize, int numCascades) 
{
    m_device = Renderer::get_instance().GetDevice();
    m_context = Renderer::get_instance().GetDeviceContext();

    assert(numCascades <= MaxCascades);

    m_shadowMapSize = shadowMapSize;
    m_numCascades = max(1, numCascades); // 1以上に制限

    m_shadowViewport = {};
    m_shadowViewport.TopLeftX = 0.0f;
    m_shadowViewport.TopLeftY = 0.0f;
    m_shadowViewport.Width = static_cast<float>(shadowMapSize);
    m_shadowViewport.Height = static_cast<float>(shadowMapSize);
    m_shadowViewport.MinDepth = 0.0f;
    m_shadowViewport.MaxDepth = 1.0f;

    for (int i = 0; i < numCascades; ++i) 
    {
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = shadowMapSize;
        texDesc.Height = shadowMapSize;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        m_device->CreateTexture2D(&texDesc, nullptr, &m_shadowMaps[i]);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        m_device->CreateDepthStencilView(m_shadowMaps[i], &dsvDesc, &m_dsvs[i]);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(m_shadowMaps[i], &srvDesc, &m_srvs[i]);
    }
}

void CascadedShadowMap::Shutdown() 
{
    for (int i = 0; i < MaxCascades; ++i) 
    {
        SafeRelease(&m_shadowMaps[i]);
        SafeRelease(&m_dsvs[i]);
        SafeRelease(&m_srvs[i]);
    }
}

void CascadedShadowMap::SetViewport(void) 
{
    m_context->RSSetViewports(1, &m_shadowViewport);
}

void CascadedShadowMap::BeginCascadeRender(int cascadeIndex) 
{
    m_context->OMGetRenderTargets(1, &m_prevRTV, &m_prevDSV);
    m_context->OMSetRenderTargets(0, nullptr, m_dsvs[cascadeIndex]);
    m_currentCascadeIndex = cascadeIndex;

}

void CascadedShadowMap::EndCascadeRender(void) 
{
    m_context->OMSetRenderTargets(1, &m_prevRTV, m_prevDSV);
    if (m_prevRTV) m_prevRTV->Release();
    if (m_prevDSV) m_prevDSV->Release();
    m_prevRTV = nullptr;
    m_prevDSV = nullptr;
}

void CascadedShadowMap::UpdateCascades(const XMMATRIX& camView, const XMMATRIX& camProj, const XMVECTOR& lightDir, float nearZ, float farZ) 
{
    // 各カスケードの分割割合（0.0～1.0）を格納する配列
    float cascadeSplits[MaxCascades] = {};

    // 分割のタイプを制御する係数（0.0 = 線形、1.0 = 対数、0.95ならほぼ対数）
    float lambda = 0.95f;
    float range = farZ - nearZ; // 視錐台の奥行きの範囲
    float ratio = farZ / nearZ; // 対数スケールの計算に使用

    if (m_config.useManualSplits)
    {
        for (int i = 0; i < m_numCascades; ++i)
            cascadeSplits[i] = m_config.manualSplits[i];
    }
    else
    {
        // カスケードごとにスプリット位置を計算
        for (int i = 0; i < m_numCascades; ++i)
        {
            float p = (i + 1) / static_cast<float>(m_numCascades);          // 分割の位置（正規化）
            float logSplit = nearZ * powf(ratio, p);                        // 対数スプリット
            float uniSplit = nearZ + range * p;                             // 線形スプリット
            float split = lambda * logSplit + (1.0f - lambda) * uniSplit;   // ハイブリッド分割
            cascadeSplits[i] = (split - nearZ) / range;                     // 0～1 に正規化して保存
        }
    }

    float lastSplitDist = 0.0f;

    // 各カスケードに対応するフラスタム範囲を決定
    for (int i = 0; i < m_numCascades; ++i) 
    {
        float splitDist = cascadeSplits[i];                     // 現在のスプリットの割合（0～1）
        float splitNear = nearZ + lastSplitDist * range;        // このカスケードの近クリップ
        float splitFar = nearZ + splitDist * range;             // このカスケードの遠クリップ

        // 現在のカスケードに対応するシャドウ行列を計算
        ComputeCascadeMatrices(i, nearZ + lastSplitDist * range, splitFar, camView, camProj, lightDir);
        // シャドウ比較用のスプリット深度
        m_cascadeData[i].splitDepth = nearZ + splitDist * range;
        // 次の nearZ 用に更新
        lastSplitDist = splitDist;
    }
}

void CascadedShadowMap::ComputeCascadeMatrices(int index, float splitNear, float splitFar,
                                               const XMMATRIX& camView, const XMMATRIX& camProj,
                                               const XMVECTOR& lightDir) 
{
    // カメラのビュー投影行列の逆行列を計算（NDC -> ワールド座標変換用）
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, camView * camProj);

    // 視錐台全体(NDC空間の8つのコーナー(0~1の深度)を定義
    XMVECTOR ndcCorners[8] =
    {
        XMVectorSet(-1,  1, 0, 1),  // near 左上
        XMVectorSet( 1,  1, 0, 1),  // near 右上
        XMVectorSet( 1, -1, 0, 1),  // near 右下
        XMVectorSet(-1, -1, 0, 1),  // near 左下
        XMVectorSet(-1,  1, 1, 1),  // far 左上
        XMVectorSet( 1,  1, 1, 1),  // far 右上
        XMVectorSet( 1, -1, 1, 1),  // far 右下
        XMVectorSet(-1, -1, 1, 1),  // far 左下
    };

    // ワールド空間に変換
    XMVECTOR frustumCornersWS[8];
    for (int i = 0; i < 8; ++i) 
    {
        frustumCornersWS[i] = XMVector3TransformCoord(ndcCorners[i], invViewProj);
    }


    // 分割された視錐台のスライスを構築(splitNear/splitFar の範囲)
    XMVECTOR sliceCorners[8];
    for (int i = 0; i < 4; ++i) 
    {
        // near 面(0~3) splitNear 平面
        sliceCorners[i] = XMVectorLerp(frustumCornersWS[i], frustumCornersWS[i + 4], (splitNear - 0.0f) / (1.0f - 0.0f));
        // far 面(4~7) splitFar 平面
        sliceCorners[i + 4] = XMVectorLerp(frustumCornersWS[i], frustumCornersWS[i + 4], (splitFar - 0.0f) / (1.0f - 0.0f));
    }

    // 視錐スライスの中心を計算
    XMVECTOR frustumCenter = XMVectorZero();
    for (int i = 0; i < 8; ++i) 
    {
        frustumCenter += sliceCorners[i];
    }
    frustumCenter /= 8.0f;


    // カスケードスライスの中心点とコーナー配列が計算済みであることを前提とする
    float radius = ComputeCascadeRadius(sliceCorners, frustumCenter, m_config.radiusStrategy, m_config.padding);

    // ライトビュー行列(光源方向からスライス中心を見下ろす)
    XMVECTOR lightPos = frustumCenter - lightDir * radius * 2.0f;
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, frustumCenter, XMVectorSet(0, 1, 0, 0));

    // 正射影行列(スライス全体をカバー)
    XMMATRIX lightProj = XMMatrixOrthographicLH(radius * 2.0f, radius * 2.0f, 0.0f, radius * 4.0f);

    // 最終ビュー行列を保存する前に、必要に応じてスナップ
    m_lastCascadeRadius = radius;
    m_lastLightDir = lightDir;

    // 最終シャドウ行列を保存
    if (m_config.enableTexelSnap)
    {
        m_cascadeData[index].lightViewProj = SnapShadowMatrixToTexelGrid(lightView, lightProj, frustumCenter);
    }
    else
    {
        m_cascadeData[index].lightViewProj = lightView * lightProj;
    }
    m_cascadeData[index].lightViewProj = lightView * lightProj;
}

XMMATRIX CascadedShadowMap::SnapShadowMatrixToTexelGrid(const XMMATRIX& lightView, const XMMATRIX& lightProj, XMVECTOR center)
{
    // ビュー射影行列の合成
    XMMATRIX lightViewProj = lightView * lightProj;

    // 中心点をライトビュー空間に変換
    XMVECTOR lightSpaceCenter = XMVector3TransformCoord(center, lightView);

    // シャドウマップのサイズを取得
    float shadowMapSize = static_cast<float>(m_shadowMapSize);

    // テクセルサイズを計算
    float texelSize = (2.0f * m_lastCascadeRadius) / shadowMapSize;

    // テクセル単位にスナップ（格子に合わせる）
    XMVECTOR snapped = XMVectorFloor(lightSpaceCenter / texelSize) * texelSize;

    // スナップ後の中心点をワールド空間に戻す
    XMMATRIX invLightView = XMMatrixInverse(nullptr, lightView);
    XMVECTOR newCenter = XMVector3TransformCoord(snapped, invLightView);

    // 新しいビュー行列を再計算
    XMVECTOR lightPos = newCenter - m_lastLightDir * m_lastCascadeRadius * 2.0f;
    return XMMatrixLookAtLH(lightPos, newCenter, XMVectorSet(0, 1, 0, 0)) * lightProj;
}

float CascadedShadowMap::ComputeCascadeRadius(XMVECTOR* corners, XMVECTOR center, ShadowRadiusStrategy strategy, float padding)
{
    switch (strategy)
    {
    case ShadowRadiusStrategy::FitBoundingSphere:
    {
        // すべてのコーナー点と中心点の距離の最大値を求める（球で包む）
        float r = 0.0f;
        // 最大半径を求めてシャドウ範囲を確保
        for (int i = 0; i < 8; ++i)
        {
            float d = XMVectorGetX(XMVector3Length(corners[i] - center));
            r = max(r, d);
        }
        // 安定化のためにパディングとスナップ単位で切り上げ
        return ceilf((r + padding) * 16.0f) / 16.0f;
    }

    case ShadowRadiusStrategy::FitBoundingBox:
    {
        // AABB（軸に沿ったバウンディングボックス）を作成
        XMVECTOR minPt = corners[0];
        XMVECTOR maxPt = corners[0];
        for (int i = 1; i < 8; ++i)
        {
            minPt = XMVectorMin(minPt, corners[i]);
            maxPt = XMVectorMax(maxPt, corners[i]);
        }

        XMVECTOR extent = (maxPt - minPt) * 0.5f;
        float r = max(max(XMVectorGetX(extent), XMVectorGetY(extent)), XMVectorGetZ(extent));
        return ceilf((r + padding) * 16.0f) / 16.0f;
    }

    case ShadowRadiusStrategy::PaddingBuffer:
        // 固定のバッファを使う（シンプルだが非効率）
        return padding;

    default:
        return 1.0f;
    }
}
