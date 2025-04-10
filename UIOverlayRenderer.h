//=============================================================================
//
// UIOverlayRenderer処理 [UIOverlayRenderer.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

// UIフェード・オーバーレイ描画クラス
class UIOverlayRenderer
{
public:
    void Initialize(ID3D11Device* device);
    void Shutdown();

    // 瞬間表示（アルファ指定）
    void Draw(ID3D11DeviceContext* ctx, float x, float y, float width, float height, XMFLOAT4 color);

    // フェード効果（Alpha値0.0f~1.0f）
    void StartFadeIn(float duration, XMFLOAT4 color);
    void StartFadeOut(float duration, XMFLOAT4 color);

    void Update(float deltaTime);
    bool IsFading() const { return m_isFading; }

private:
    ID3D11Buffer* m_vertexBuffer = nullptr;

    float m_fadeTime = 0.0f;
    float m_fadeDuration = 0.0f;
    bool m_fadeIn = true;
    bool m_isFading = false;
    XMFLOAT4 m_targetColor = { 0, 0, 0, 0 };

    // 現在のアルファ値を取得
    float ComputeAlpha() const;
};
