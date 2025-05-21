//=============================================================================
//
// ���ׂĂ�UI�����𓝊�����Ǘ����o���� [UIManager.h]
// Author : 
// UI�v�f���܂Ƃ߂ĊǗ��E�`��E�X�V���A���[�_���̐����I�[�o�[���C�`������Ă���闊���N���X�ł����I
// UI�E�̏������܂݂����ȑ��݂ŁA�݂�Ȃ��X���[�Y�ɓ�����悤�Ɏx���Ă���Ă��~
//
//=============================================================================
#pragma once
#include "Utility/SingletonBase.h"
#include "UIElement.h"
#include "UIOverlayRenderer.h"
#include "Utility/SimpleArray.h"

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