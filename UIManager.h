//=============================================================================
//
// UIManager処理 [UIManager.h]
// Author : 
//
//=============================================================================
#pragma once
#include "SimpleArray.h"
#include "SingletonBase.h"
#include "UIElement.h"
#include "UIOverlayRenderer.h"

class UIManager : public SingletonBase<UIManager>
{
public:
    UIManager() : m_modal(nullptr) {}
    ~UIManager() { Clear(); }

    void Init(void);
    void Update(void);
    void Draw(void);
    void Clear(void);
    void AddElement(UIElement* element);

    void SetModal(UIElement* modal);
    bool IsModalActive() const;
    void ClearModal(void);

    void DrawOverlay(float x, float y, float w, float h, XMFLOAT4 color);
private:
    SimpleArray<UIElement*> m_elements; // UI要素の配列
    UIElement* m_modal; // モーダルUIの保持
    UIOverlayRenderer m_overlay; // UIオーバーレイ描画クラス
    Renderer& m_renderer = Renderer::get_instance();
};