//=============================================================================
//
// �ꎞ��~�̖��@�������郂�[�_������� [PauseModal.h]
// Author : 
// �|�[�Y���ɉ�ʑS�̂��ӂ���I�[�o�[���C�ŕ��ł�������UI�N���X�ł���
// �w�i���₳�����Ղ��āA�܂�Ŏ��Ԃ��~�܂����݂����ȉ��o�����Ă�����
// 
//=============================================================================
#pragma once
#include "UIElement.h"
#include "UI/UIManager.h"

class PauseModal : public UIElement
{
public:
    PauseModal() 
    {
        m_position = { 640, 360 };
        m_size = { 400, 200 };
        //m_customOverlay = true; // �I�[�o�[���C���g�p����
    }

    void Update(void) override {} // ������
    void Draw(void) override {}
    bool NeedsOverlay(void) const override { return true; } // �I�[�o�[���C���K�v
    void DrawCustomOverlay(void) const override
    {
        UIManager::get_instance().DrawOverlay(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, { 1.0f, 0.7f, 0.9f, 0.4f });
    }
};

