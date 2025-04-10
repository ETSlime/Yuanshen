//=============================================================================
//
// UIOverlayRenderer処理 [UIOverlayRenderer.cpp]
// Author : 
//
//=============================================================================
#include "UIOverlayRenderer.h"
#include "UISpriteRenderer.h"

void UIOverlayRenderer::Initialize(ID3D11Device* device)
{
    if (m_vertexBuffer == nullptr)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = sizeof(UIVertex) * 4;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device->CreateBuffer(&desc, nullptr, &m_vertexBuffer);
    }
}

void UIOverlayRenderer::Shutdown()
{
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = nullptr;
    }
}

void UIOverlayRenderer::Draw(ID3D11DeviceContext* ctx, float x, float y, float width, float height, XMFLOAT4 color)
{
    // アニメーション中であれば、アルファ値を補間する
    if (m_isFading)
    {
        color.w = ComputeAlpha();
    }

    UISpriteRenderer::DrawSpriteWithColor(m_vertexBuffer, x, y, width, height, 0, 0, 1, 1, color);
}

void UIOverlayRenderer::StartFadeIn(float duration, XMFLOAT4 color)
{
    m_fadeIn = true;
    m_fadeDuration = duration;
    m_fadeTime = 0.0f;
    m_isFading = true;
    m_targetColor = color;
}

void UIOverlayRenderer::StartFadeOut(float duration, XMFLOAT4 color)
{
    m_fadeIn = false;
    m_fadeDuration = duration;
    m_fadeTime = 0.0f;
    m_isFading = true;
    m_targetColor = color;
}

void UIOverlayRenderer::Update(float deltaTime)
{
    if (m_isFading)
    {
        m_fadeTime += deltaTime;
        if (m_fadeTime >= m_fadeDuration)
        {
            m_fadeTime = m_fadeDuration;
            m_isFading = false;
        }
    }
}

float UIOverlayRenderer::ComputeAlpha() const
{
    float t = m_fadeTime / m_fadeDuration;
    t = (t > 1.0f) ? 1.0f : t;
    return m_fadeIn ? (t * m_targetColor.w) : ((1.0f - t) * m_targetColor.w);
}