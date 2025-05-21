//=============================================================================
//
// �v���C���[�̑�������[��Ԏ󂯎��i�ߓ������ [InputManager.h]
// Author : 
// �L�[�{�[�h�E�}�E�X�E�Q�[���p�b�h�̓��͏�Ԃ��܂Ƃ߂ĊǗ�������͎�t�N���X�ł���
// �����ꂽ�u�Ԃ≟�����ςȂ��A�}�E�X�ړ��E�z�C�[������܂ŁA�Q�[���Ƃ̋��n�����Ȃ�
// 
//=============================================================================
#pragma once
#include "Utility/SingletonBase.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************

// �v���O������������Ƃ��Ɏg��
#define	USE_KEYBOARD										// �錾����ƃL�[�{�[�h�ő���\�ɂȂ�
#define	USE_MOUSE											// �錾����ƃ}�E�X�ő���\�ɂȂ�
#define	USE_PAD												// �錾����ƃp�b�h�ő���\�ɂȂ�


/* game pad��� */
#define BUTTON_UP		0x00000001l	// �����L�[��(.IY<0)
#define BUTTON_DOWN		0x00000002l	// �����L�[��(.IY>0)
#define BUTTON_LEFT		0x00000004l	// �����L�[��(.IX<0)
#define BUTTON_RIGHT	0x00000008l	// �����L�[�E(.IX>0)
#define BUTTON_A		0x00000010l	// �`�{�^��(.rgbButtons[0]&0x80)
#define BUTTON_B		0x00000020l	// �a�{�^��(.rgbButtons[1]&0x80)
#define BUTTON_C		0x00000040l	// �b�{�^��(.rgbButtons[2]&0x80)
#define BUTTON_X		0x00000080l	// �w�{�^��(.rgbButtons[3]&0x80)
#define BUTTON_Y		0x00000100l	// �x�{�^��(.rgbButtons[4]&0x80)
#define BUTTON_Z		0x00000200l	// �y�{�^��(.rgbButtons[5]&0x80)
#define BUTTON_L		0x00000400l	// �k�{�^��(.rgbButtons[6]&0x80)
#define BUTTON_R		0x00000800l	// �q�{�^��(.rgbButtons[7]&0x80)
#define BUTTON_START	0x00001000l	// �r�s�`�q�s�{�^��(.rgbButtons[8]&0x80)
#define BUTTON_M		0x00002000l	// �l�{�^��(.rgbButtons[9]&0x80)
#define GAMEPADMAX		4			// �����ɐڑ�����W���C�p�b�h�̍ő吔���Z�b�g

//===============================
// ���̓L�[��`�i�Œ�o�C���f�B���O�j
//===============================
#define	NUM_KEY_MAX			(256)

// �ꎞ��~�i�|�[�Y�j�L�[
#define KEY_PAUSE       DIK_P

// �v���C���[�ړ��L�[
#define KEY_MOVE_FORWARD   DIK_W
#define KEY_MOVE_BACKWARD  DIK_S
#define KEY_MOVE_LEFT      DIK_A
#define KEY_MOVE_RIGHT     DIK_D

// �_�b�V���i����j�L�[
#define KEY_RUN         DIK_LSHIFT

// �}�b�v�^�N�G�X�g�L�[
#define KEY_MAP         DIK_M
#define KEY_QUEST       DIK_Q

// ���j���[����
#define KEY_CONFIRM     DIK_RETURN
#define KEY_CANCEL      DIK_ESCAPE

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

class InputManager : public SingletonBase<InputManager>
{
public:
	HRESULT Init(HINSTANCE hInst, HWND hWnd);
	void Shutdown(void);
	void Update(void);

	//---------------------------- keyboard
	BOOL GetKeyboardPress(int nKey);
	BOOL GetKeyboardTrigger(int nKey);
	BOOL GetKeyboardRepeat(int nKey);
	BOOL GetKeyboardRelease(int nKey);

	//---------------------------- mouse
	BOOL IsMouseLeftPressed(void);      // ���N���b�N�������
	BOOL IsMouseLeftTriggered(void);    // ���N���b�N�����u��
	BOOL IsMouseRightPressed(void);     // �E�N���b�N�������
	BOOL IsMouseRightTriggered(void);   // �E�N���b�N�����u��
	BOOL IsMouseCenterPressed(void);    // ���N���b�N�������
	BOOL IsMouseCenterTriggered(void);  // ���N���b�N�����u��
	long GetMouseX(void);               // �}�E�X��X�����ɓ��������Βl
	long GetMouseY(void);               // �}�E�X��Y�����ɓ��������Βl
	long GetMouseZ(void);               // �}�E�X�z�C�[�������������Βl

	//---------------------------- game pad
	BOOL IsButtonPressed(int padNo, DWORD button);
	BOOL IsButtonTriggered(int padNo, DWORD button);

	void SetMousePosCenter(void);
	void SetMouseRecentered(BOOL recenter);
	BOOL IsMouseRecentered(void);

private:
	HRESULT InitKeyboard(HINSTANCE hInst, HWND hWnd);
	void UninitKeyboard(void);
	HRESULT UpdateKeyboard(void);

	HRESULT InitializeMouse(HINSTANCE hInst, HWND hWindow); // �}�E�X�̏�����
	void UninitMouse();						// �}�E�X�̏I������
	HRESULT UpdateMouse();					// �}�E�X�̍X�V����

	static BOOL SearchGamePadCallback(const LPDIDEVICEINSTANCE lpddi, LPVOID pContext);

	HRESULT InitializePad(void);			// �p�b�h������
	//BOOL CALLBACK SearchPadCallback(LPDIDEVICEINSTANCE lpddi, LPVOID);	// �p�b�h�����R�[���o�b�N
	void UpdatePad(void);
	void UninitPad(void);

	//------------------------------- keyboard
	LPDIRECTINPUT8			m_pDInput = NULL;					// IDirectInput8�C���^�[�t�F�[�X�ւ̃|�C���^
	LPDIRECTINPUTDEVICE8	m_pDIDevKeyboard = NULL;			// IDirectInputDevice8�C���^�[�t�F�[�X�ւ̃|�C���^(�L�[�{�[�h)
	BYTE					m_keyState[NUM_KEY_MAX];			// �L�[�{�[�h�̏�Ԃ��󂯎�郏�[�N
	BYTE					m_keyStateTrigger[NUM_KEY_MAX];		// �L�[�{�[�h�̏�Ԃ��󂯎�郏�[�N
	BYTE					m_keyStateRepeat[NUM_KEY_MAX];		// �L�[�{�[�h�̏�Ԃ��󂯎�郏�[�N
	BYTE					m_keyStateRelease[NUM_KEY_MAX];		// �L�[�{�[�h�̏�Ԃ��󂯎�郏�[�N
	int						m_keyStateRepeatCnt[NUM_KEY_MAX];	// �L�[�{�[�h�̃��s�[�g�J�E���^

	//--------------------------------- mouse
	LPDIRECTINPUTDEVICE8 m_pMouse = NULL; // mouse

	DIMOUSESTATE2   m_mouseState;		// �}�E�X�̃_�C���N�g�ȏ��
	DIMOUSESTATE2   m_mouseTrigger;	// �����ꂽ�u�Ԃ���ON
	BOOL			m_isMouseRecentered = FALSE;
	//--------------------------------- game pad

	LPDIRECTINPUTDEVICE8	m_pGamePad[GAMEPADMAX] = { NULL,NULL,NULL,NULL };// �p�b�h�f�o�C�X

	DWORD	m_padState[GAMEPADMAX];	// �p�b�h���i�����Ή��j
	DWORD	m_padTrigger[GAMEPADMAX];
	int		m_padCount = 0;			// ���o�����p�b�h�̐�
};

