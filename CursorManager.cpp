//=============================================================================
//
// CursorManager���� [CursorManager.h]
// Author : 
//
//=============================================================================
#include "CursorManager.h"
#include "input.h"

void CursorManager::Init(void)
{
    // �f�t�H���g�J�[�\���ǂݍ��݁i�V�X�e���j
    m_defaultCursor = LoadCursor(nullptr, IDC_ARROW);

    // �I���W�i���J�[�\���ǂݍ��݁i.cur �t�@�C���j
    m_customCursor = LoadCursorFromFileA("data/TEXTURE/cursor.cur");

    m_currentCursor = m_customCursor;
    m_cursorType = CursorType::Custom;

    SetMousePosCenter(); // �J�����p�F�}�E�X�ʒu����ʂ̒��S�ɐݒ�
    GetCursorPos(&m_lastCursorPos); // �J�[�\���ʒu��ۑ�
    ShowCursor(FALSE);  // �J�[�\����\��
}

void CursorManager::Show(void)
{
    // �J�[�\����\������
    while (ShowCursor(TRUE) < 0);
    m_isCursorVisible = true;
}

void CursorManager::Hide(void)
{
    // �J�[�\�����\���ɂ���
	while (ShowCursor(FALSE) >= 0);
	m_isCursorVisible = false;
}

void CursorManager::SetCursorType(CursorType type)
{
    m_cursorType = type;

    switch (type)
    {
    case CursorType::Default:
        m_currentCursor = m_defaultCursor;
        break;
    case CursorType::Custom:
        m_currentCursor = m_customCursor;
        break;
    }

    SetCursor(m_currentCursor); // �����ɔ��f�i������ WM_SETCURSOR ���v�Ή��j
}

HCURSOR CursorManager::GetCurrentCursor(void) const
{
    return m_currentCursor;
}

CursorType CursorManager::GetCurrentCursorType(void) const
{
    return m_cursorType;
}

void CursorManager::RememberCursorPosition(void)
{
    GetCursorPos(&m_lastCursorPos);
}

void CursorManager::RestoreLastCursorPosition(void)
{
    // �}�E�X�����R�ɑ���ł����ԁiUI���쒆�Ȃǁj�̏ꍇ�́A
    // �����Ƀ}�E�X�ʒu�𕜌����Ȃ��i�W�����v��h���j
    if (IsMouseFreeMode())
        return;

    // �J�[�\���ʒu�𕜌�
    SetCursorPos(m_lastCursorPos.x, m_lastCursorPos.y);
}

bool CursorManager::IsMouseFreeMode() const
{
    // �J�[�\�����[�h�� UIExclusive �܂��� VisibleTemp �̏ꍇ
    return m_mode == CursorMode::VisibleTemp || m_mode == CursorMode::UIExclusive;
}

void CursorManager::RestoreCursorAndEnterUIExclusive(void)
{
    RestoreLastCursorPosition(); // �J�[�\���ʒu�𕜌�
    EnterUIExclusive(); // UI��p���[�h�ɓ���
}

void CursorManager::EnterUIExclusive(void)
{
    if (m_mode == CursorMode::UIExclusive) return;

    m_mode = CursorMode::UIExclusive; // �J�[�\�����[�h�� UIExclusive �ɕύX
    Show(); // �J�[�\����\��
}

void CursorManager::EnterUIHidden(void)
{
    if (m_mode == CursorMode::Hidden) return;

	m_mode = CursorMode::Hidden; // �J�[�\�����[�h�� Hidden �ɕύX
	Hide(); // �J�[�\�����\��
}

CursorMode CursorManager::GetCursorMode(void) const
{
    return m_mode;
}

void CursorManager::OnEnterVisibleTemp(void)
{
    if (m_mode == CursorMode::VisibleTemp) return;

    
    RestoreLastCursorPosition(); // �J�[�\���ʒu�𕜌�
    Show(); // �J�[�\����\��
    m_mode = CursorMode::VisibleTemp;
}

void CursorManager::OnExitVisibleTemp(void)
{
	if (m_mode != CursorMode::VisibleTemp) return;

    RememberCursorPosition(); // �J�[�\���ʒu��ۑ�
	Hide(); // �J�[�\�����\��
    SetMouseRecentered(true); // �J�����p�F���̃t���[���Ń}�E�X�ʒu���Z�b�g��v��
	m_mode = CursorMode::Hidden;
}

