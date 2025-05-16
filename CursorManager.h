//=============================================================================
//
// �J�[�\���̎�ށE�\����ԁE�ʒu�𐧌䂷����͕⏕�N���X [CursorManager.h]
// Author : 
// �Q�[�����̃J�[�\���\���ؑցA��ސݒ�A���W�L�^�E�����𓝊����A
// UI��p���[�h��ꎞ�\���ȂǑ��l�ȑ����ԂɑΉ�����
//
//=============================================================================
#pragma once
#include "main.h"
#include "SingletonBase.h"
#include "InputManager.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************

// �g�p�\�ȃJ�[�\���^�C�v
enum class CursorType
{
    Default,    // �W���̖��J�[�\��
    Custom,     // �����J�X�^���J�[�\��
};

// �J�[�\���̃��[�h
enum class CursorMode
{
    Hidden,         // �J�[�\����\��
    VisibleTemp,    // �ꎞ�I�ɃJ�[�\���\��
    UIExclusive,	// UI��p�J�[�\���\��
};

class CursorManager : public SingletonBase<CursorManager>

{
public:

    // �J�[�\���̓ǂݍ���
    void Init(void);

    // �J�[�\���̕\��/��\��
    void Show(void);
    void Hide(void);

    // �J�[�\����ݒ�iSetCursor�j
    void SetCursorType(CursorType type);

    // ���݂̃J�[�\�����擾�iWM_SETCURSOR �p�j
    HCURSOR GetCurrentCursor(void) const;

    // ���݂̃J�[�\���^�C�v���擾
    CursorType GetCurrentCursorType(void) const;

    // �Q�[���ĊJ�O�Ɍ��݂̃J�[�\���ʒu���L�^
    void RememberCursorPosition(void);

    // �Q�[���ꎞ��~���ɑO��ʒu�֖߂�
    void RestoreLastCursorPosition(void);

    // �ꎞ�I�Ƀ}�E�X�����R�ɑ���ł����Ԃ��ǂ����𔻒肷��
    bool IsMouseFreeMode() const;

    void RestoreCursorAndEnterUIExclusive(void);

    void EnterUIHidden(void);
    void EnterUIExclusive(void);

    CursorMode GetCursorMode() const;

    POINT GetLastCursorPos() const
	{
		return m_lastCursorPos;
	}

    void OnEnterVisibleTemp();
    void OnExitVisibleTemp();

private:

    HCURSOR m_defaultCursor = nullptr;
    HCURSOR m_customCursor = nullptr;

    HCURSOR m_currentCursor = nullptr;
    CursorType m_cursorType = CursorType::Default;

    // �J�[�\���̕\�����
    bool m_isCursorVisible = true;

    CursorMode m_mode = CursorMode::Hidden;

    // �}�E�X�̈ʒu
    POINT m_lastCursorPos;

	InputManager& m_inputManager = InputManager::get_instance();
};