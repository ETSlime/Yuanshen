//=============================================================================
//
// UIManager���� [UIManager.h]
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
    SimpleArray<UIElement*> m_elements; // UI�v�f�̔z��
    UIElement* m_modal; // ���[�_��UI�̕ێ�
    UIOverlayRenderer m_overlay; // UI�I�[�o�[���C�`��N���X
    Renderer& m_renderer = Renderer::get_instance();
};