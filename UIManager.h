//=============================================================================
//
// UIManager���� [UIManager.h]
// Author : 
//
//=============================================================================
#pragma once
#include "SingletonBase.h"
#include "UIElement.h"
#include "UIOverlayRenderer.h"
#include "SimpleArray.h"

class UIManager : public SingletonBase<UIManager>
{
public:
    UIManager() : m_modal(nullptr) {}

    void Init(void);
    void Uninit(void) {Clear();}
    void Update(void);
    void Draw(void);
    void Clear(void);
    void AddElement(UIElement* element);

    template<typename T>
    void SetModalIfNotExist()
    {
        if (dynamic_cast<T*>(m_modal) == nullptr)
        {
            if (m_modal)
                delete m_modal;
            m_modal = new T();
        }
    }

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