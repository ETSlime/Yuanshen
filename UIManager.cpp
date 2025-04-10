//=============================================================================
//
// UIElement処理 [UIElement.cpp]
// Author : 
//
//=============================================================================
#include "UIManager.h"
#include "UISpriteRenderer.h"

void UIManager::Init(void)
{
    m_overlay.Initialize(m_renderer.GetDevice());
}

void UIManager::Update(void)
{
    if (m_modal)
    {
        m_modal->Update(); // モーダルだけ更新
    }
    else
    {
        for (auto& e : m_elements)
            e->Update();
    }
}

void UIManager::Draw(void)
{
    // Z順でソート（必要なら）
    for (size_t i = 1; i < m_elements.getSize(); ++i)
    {
        auto key = m_elements[i];
        int j = static_cast<int>(i) - 1;

        while (j >= 0 && m_elements[j]->GetZOrder() > key->GetZOrder())
        {
            m_elements[j + 1] = m_elements[j];
            --j;
        }

        m_elements[j + 1] = key;
    }

    // UI要素を描画
    for (auto& e : m_elements)
        e->Draw();

    if (m_modal)
    {
        if (m_modal->HasCustomOverlay())
        {
            // カスタムオーバーレイを描画
            m_modal->DrawCustomOverlay();
        }
        else if (m_modal->NeedsOverlay())
        {
            // 半透明の黒い背景
            DrawOverlay(0, 0, 1280, 720, { 0, 0, 0, 0.5f });
        }


        m_modal->Draw();
    }
}

void UIManager::AddElement(UIElement* element)
{
    m_elements.push_back(element);
}

void UIManager::SetModal(UIElement* modal)
{
    m_modal = modal;
}

bool UIManager::IsModalActive() const
{
    return m_modal != nullptr;
}

void UIManager::ClearModal(void)
{
    m_modal = nullptr;
}

void UIManager::DrawOverlay(float x, float y, float w, float h, XMFLOAT4 color)
{
    m_overlay.Draw(m_renderer.GetDeviceContext(), x, y, w, h, color);
}

void UIManager::Clear()
{
    m_elements.clear();
}
