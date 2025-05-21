//=============================================================================
//
// ���͏��� [InputManager.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "Utility/InputManager.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
// game pad�p�ݒ�l
#define DEADZONE		2500			// �e����25%�𖳌��]�[���Ƃ���
#define RANGE_MAX		1000			// �L���͈͂̍ő�l
#define RANGE_MIN		-1000			// �L���͈͂̍ŏ��l


//=============================================================================
// ���͏����̏�����
//=============================================================================
HRESULT InputManager::Init(HINSTANCE hInst, HWND hWnd)
{
	HRESULT hr;

	if(!m_pDInput)
	{
		// DirectInput�I�u�W�F�N�g�̍쐬
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
									IID_IDirectInput8, (void**)&m_pDInput, NULL);
	}

	// �L�[�{�[�h�̏�����
	InitKeyboard(hInst, hWnd);

 	// �}�E�X�̏�����
	InitializeMouse(hInst, hWnd);
	
	// �p�b�h�̏�����
	InitializePad();

	return S_OK;
}

//=============================================================================
// ���͏����̏I������
//=============================================================================
void InputManager::Shutdown(void)
{
	// �L�[�{�[�h�̏I������
	UninitKeyboard();

	// �}�E�X�̏I������
	UninitMouse();

	// �p�b�h�̏I������
	UninitPad();

	if(m_pDInput)
	{
		m_pDInput->Release();
		m_pDInput = NULL;
	}
}

//=============================================================================
// ���͏����̍X�V����
//=============================================================================
void InputManager::Update(void)
{
	if (!GetWindowActive()) return; // ��A�N�e�B�u�Ȃ���͏������X�L�b�v

	// �L�[�{�[�h�̍X�V
	UpdateKeyboard();
	
	// �}�E�X�̍X�V
	UpdateMouse();
	
	// �p�b�h�̍X�V
	UpdatePad();

}

//=============================================================================
// �L�[�{�[�h�̏�����
//=============================================================================
HRESULT InputManager::InitKeyboard(HINSTANCE hInst, HWND hWnd)
{
	HRESULT hr;

	// �f�o�C�X�I�u�W�F�N�g���쐬
	hr = m_pDInput->CreateDevice(GUID_SysKeyboard, &m_pDIDevKeyboard, NULL);
	if(FAILED(hr) || m_pDIDevKeyboard == NULL)
	{
		MessageBox(hWnd, "�L�[�{�[�h���˂��I", "�x���I", MB_ICONWARNING);
		return hr;
	}

	// �f�[�^�t�H�[�}�b�g��ݒ�
	hr = m_pDIDevKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(hr))
	{
		MessageBox(hWnd, "�L�[�{�[�h�̃f�[�^�t�H�[�}�b�g��ݒ�ł��܂���ł����B", "�x���I", MB_ICONWARNING);
		return hr;
	}

	// �������[�h��ݒ�i�t�H�A�O���E���h����r�����[�h�j
	hr = m_pDIDevKeyboard->SetCooperativeLevel(hWnd, (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	if(FAILED(hr))
	{
		MessageBox(hWnd, "�L�[�{�[�h�̋������[�h��ݒ�ł��܂���ł����B", "�x���I", MB_ICONWARNING);
		return hr;
	}

	// �L�[�{�[�h�ւ̃A�N�Z�X�����l��(���͐���J�n)
	m_pDIDevKeyboard->Acquire();

	return S_OK;
}

//=============================================================================
// �L�[�{�[�h�̏I������
//=============================================================================
void InputManager::UninitKeyboard(void)
{
	if(m_pDIDevKeyboard)
	{
		m_pDIDevKeyboard->Release();
		m_pDIDevKeyboard = NULL;
	}
}

//=============================================================================
// �L�[�{�[�h�̍X�V
//=============================================================================
HRESULT InputManager::UpdateKeyboard(void)
{
	HRESULT hr;
	BYTE keyStateOld[256];

	// �O��̃f�[�^��ۑ�
	memcpy(keyStateOld, m_keyState, NUM_KEY_MAX);

	// �f�o�C�X����f�[�^���擾
	hr = m_pDIDevKeyboard->GetDeviceState(sizeof(m_keyState), m_keyState);
	if(SUCCEEDED(hr))
	{
		for(int cnt = 0; cnt < NUM_KEY_MAX; cnt++)
		{
			m_keyStateTrigger[cnt] = (keyStateOld[cnt] ^ m_keyState[cnt]) & m_keyState[cnt];
			m_keyStateRelease[cnt] = (keyStateOld[cnt] ^ m_keyState[cnt]) & ~m_keyState[cnt];
			m_keyStateRepeat[cnt] = m_keyStateTrigger[cnt];

			if(m_keyState[cnt])
			{
				m_keyStateRepeatCnt[cnt]++;
				if(m_keyStateRepeatCnt[cnt] >= 20)
				{
					m_keyStateRepeat[cnt] = m_keyState[cnt];
				}
			}
			else
			{
				m_keyStateRepeatCnt[cnt] = 0;
				m_keyStateRepeat[cnt] = 0;
			}
		}
	}
	else
	{
		// �L�[�{�[�h�ւ̃A�N�Z�X�����擾
		m_pDIDevKeyboard->Acquire();
	}

	return S_OK;
}

//=============================================================================
// �L�[�{�[�h�̃v���X��Ԃ��擾
//=============================================================================
BOOL InputManager::GetKeyboardPress(int key)
{
	return (m_keyState[key] & 0x80) ? TRUE : FALSE;
}

//=============================================================================
// �L�[�{�[�h�̃g���K�[��Ԃ��擾
//=============================================================================
BOOL InputManager::GetKeyboardTrigger(int key)
{
	return (m_keyStateTrigger[key] & 0x80) ? TRUE : FALSE;
}

//=============================================================================
// �L�[�{�[�h�̃��s�[�g��Ԃ��擾
//=============================================================================
BOOL InputManager::GetKeyboardRepeat(int key)
{
	return (m_keyStateRepeat[key] & 0x80) ? TRUE : FALSE;
}

//=============================================================================
// �L�[�{�[�h�̃����|�X��Ԃ��擾
//=============================================================================
BOOL InputManager::GetKeyboardRelease(int key)
{
	return (m_keyStateRelease[key] & 0x80) ? TRUE : FALSE;
}


//=============================================================================
// �}�E�X�֌W�̏���
//=============================================================================
// �}�E�X�̏�����
HRESULT InputManager::InitializeMouse(HINSTANCE hInst,HWND hWindow)
{
	HRESULT result;
	// �f�o�C�X�쐬
	result = m_pDInput->CreateDevice(GUID_SysMouse,&m_pMouse,NULL);
	if(FAILED(result) || m_pMouse==NULL)
	{
		MessageBox(hWindow,"No mouse","Warning",MB_OK | MB_ICONWARNING);
		return result;
	}
	// �f�[�^�t�H�[�}�b�g�ݒ�
	result = m_pMouse->SetDataFormat(&c_dfDIMouse2);
	if(FAILED(result))
	{
		MessageBox(hWindow,"Can't setup mouse","Warning",MB_OK | MB_ICONWARNING);
		return result;
	}
	// ���̃A�v���Ƌ������[�h�ɐݒ�
	result = m_pMouse->SetCooperativeLevel(hWindow, (DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	if(FAILED(result))
	{
		MessageBox(hWindow,"Mouse mode error","Warning",MB_OK | MB_ICONWARNING);
		return result;
	}
	
	// �f�o�C�X�̐ݒ�
	DIPROPDWORD prop;
	
	prop.diph.dwSize = sizeof(prop);
	prop.diph.dwHeaderSize = sizeof(prop.diph);
	prop.diph.dwObj = 0;
	prop.diph.dwHow = DIPH_DEVICE;
	prop.dwData = DIPROPAXISMODE_REL;		// �}�E�X�̈ړ��l�@���Βl

	result = m_pMouse->SetProperty(DIPROP_AXISMODE,&prop.diph);
	if(FAILED(result))
	{
		MessageBox(hWindow,"Mouse property error","Warning",MB_OK | MB_ICONWARNING);
		return result;	
	}
	
	// �A�N�Z�X���𓾂�
	m_pMouse->Acquire();
	return result;
}
//---------------------------------------------------------
void InputManager::UninitMouse()
{
	if(m_pMouse)
	{
		m_pMouse->Unacquire();
		m_pMouse->Release();
		m_pMouse = NULL;
	}

}
//-----------------------------------------------------------
HRESULT InputManager::UpdateMouse()
{
	if (!GetWindowActive()) return S_OK; // ��A�N�e�B�u���͍X�V���Ȃ�

	HRESULT result;
	// �O��̒l�ۑ�
	DIMOUSESTATE2 lastm_mouseState = m_mouseState;
	// �f�[�^�擾
	result = m_pMouse->GetDeviceState(sizeof(m_mouseState),&m_mouseState);
	if(SUCCEEDED(result))
	{
		m_mouseTrigger.lX = m_mouseState.lX;
		m_mouseTrigger.lY = m_mouseState.lY;
		m_mouseTrigger.lZ = m_mouseState.lZ;
		// �}�E�X�̃{�^�����
		for(int i=0;i<8;i++)
		{
			m_mouseTrigger.rgbButtons[i] = ((lastm_mouseState.rgbButtons[i] ^
				m_mouseState.rgbButtons[i]) & m_mouseState.rgbButtons[i]);
		}
	}
	else	// �擾���s
	{
		// �A�N�Z�X���𓾂Ă݂�
		result = m_pMouse->Acquire();
	}
	return result;
	
}

//----------------------------------------------
BOOL InputManager::IsMouseLeftPressed(void)
{
	return (BOOL)(m_mouseState.rgbButtons[0] & 0x80);	// �����ꂽ�Ƃ��ɗ��r�b�g������
}
BOOL InputManager::IsMouseLeftTriggered(void)
{
	return (BOOL)(m_mouseTrigger.rgbButtons[0] & 0x80);
}
BOOL InputManager::IsMouseRightPressed(void)
{
	return (BOOL)(m_mouseState.rgbButtons[1] & 0x80);
}
BOOL InputManager::IsMouseRightTriggered(void)
{
	return (BOOL)(m_mouseTrigger.rgbButtons[1] & 0x80);
}
BOOL InputManager::IsMouseCenterPressed(void)
{
	return (BOOL)(m_mouseState.rgbButtons[2] & 0x80);
}
BOOL InputManager::IsMouseCenterTriggered(void)
{
	return (BOOL)(m_mouseTrigger.rgbButtons[2] & 0x80);
}
//------------------
long InputManager::GetMouseX(void)
{
	return m_mouseState.lX;
}
long InputManager::GetMouseY(void)
{
	return m_mouseState.lY;
}
long InputManager::GetMouseZ(void)
{
	return m_mouseState.lZ;
}
//================================================= game pad
//---------------------------------------- �R�[���o�b�N�֐�
BOOL CALLBACK InputManager::SearchGamePadCallback(const LPDIDEVICEINSTANCE lpddi, LPVOID pContext)
{
	InputManager* self = reinterpret_cast<InputManager*>(pContext);

	if (self->m_padCount >= 4) return DIENUM_STOP;

	HRESULT result = self->m_pDInput->CreateDevice(lpddi->guidInstance, &self->m_pGamePad[self->m_padCount++], NULL);
	return DIENUM_CONTINUE;	// ���̃f�o�C�X���

}

//---------------------------------------- ������
HRESULT InputManager::InitializePad(void)			// �p�b�h������
{
	HRESULT		result;
	int			i;

	m_padCount = 0;
	// �W���C�p�b�h��T��
	m_pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)SearchGamePadCallback, this, DIEDFL_ATTACHEDONLY);
	// �Z�b�g�����R�[���o�b�N�֐����A�p�b�h�𔭌������������Ă΂��B

	for ( i=0 ; i<m_padCount ; i++ ) {
		// �W���C�X�e�B�b�N�p�̃f�[�^�E�t�H�[�}�b�g��ݒ�
		result = m_pGamePad[i]->SetDataFormat(&c_dfDIJoystick);
		if ( FAILED(result) )
			return FALSE; // �f�[�^�t�H�[�}�b�g�̐ݒ�Ɏ��s

		// ���[�h��ݒ�i�t�H�A�O���E���h����r�����[�h�j
//		result = m_pGamePad[i]->SetCooperativeLevel(hWindow, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
//		if ( FAILED(result) )
//			return FALSE; // ���[�h�̐ݒ�Ɏ��s

		// ���̒l�͈̔͂�ݒ�
		// X���AY���̂��ꂼ��ɂ��āA�I�u�W�F�N�g���񍐉\�Ȓl�͈̔͂��Z�b�g����B
		// (max-min)�́A�ő�10,000(?)�B(max-min)/2�������l�ɂȂ�B
		// ����傫������΁A�A�i���O�l�ׂ̍��ȓ�����߂炦����B(�p�b�h�̐��\�ɂ��)
		DIPROPRANGE				diprg;
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize		= sizeof(diprg); 
		diprg.diph.dwHeaderSize	= sizeof(diprg.diph); 
		diprg.diph.dwHow		= DIPH_BYOFFSET; 
		diprg.lMin				= RANGE_MIN;
		diprg.lMax				= RANGE_MAX;
		// X���͈̔͂�ݒ�
		diprg.diph.dwObj		= DIJOFS_X; 
		m_pGamePad[i]->SetProperty(DIPROP_RANGE, &diprg.diph);
		// Y���͈̔͂�ݒ�
		diprg.diph.dwObj		= DIJOFS_Y;
		m_pGamePad[i]->SetProperty(DIPROP_RANGE, &diprg.diph);

		// �e�����ƂɁA�����̃]�[���l��ݒ肷��B
		// �����]�[���Ƃ́A��������̔����ȃW���C�X�e�B�b�N�̓����𖳎�����͈͂̂��ƁB
		// �w�肷��l�́A10000�ɑ΂��鑊�Βl(2000�Ȃ�20�p�[�Z���g)�B
		DIPROPDWORD				dipdw;
		dipdw.diph.dwSize		= sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize	= sizeof(dipdw.diph);
		dipdw.diph.dwHow		= DIPH_BYOFFSET;
		dipdw.dwData			= DEADZONE;
		//X���̖����]�[����ݒ�
		dipdw.diph.dwObj		= DIJOFS_X;
		m_pGamePad[i]->SetProperty( DIPROP_DEADZONE, &dipdw.diph);
		//Y���̖����]�[����ݒ�
		dipdw.diph.dwObj		= DIJOFS_Y;
		m_pGamePad[i]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
			
		//�W���C�X�e�B�b�N���͐���J�n
		m_pGamePad[i]->Acquire();
	}
		
	return TRUE;

}
//------------------------------------------- �I������
void InputManager::UninitPad(void)
{
	for (int i=0 ; i<GAMEPADMAX ; i++) {
		if ( m_pGamePad[i] )
		{
			m_pGamePad[i]->Unacquire();
			m_pGamePad[i]->Release();
		}
	}

}

//------------------------------------------ �X�V
void InputManager::UpdatePad(void)
{
	HRESULT			result;
	DIJOYSTATE2		dijs;
	int				i;

	for ( i=0 ; i<m_padCount ; i++ ) 
	{
		DWORD lastPadState;
		lastPadState = m_padState[i];
		m_padState[i] = 0x00000000l;	// ������

		result = m_pGamePad[i]->Poll();	// �W���C�X�e�B�b�N�Ƀ|�[����������
		if ( FAILED(result) ) {
			result = m_pGamePad[i]->Acquire();
			while ( result == DIERR_INPUTLOST )
				result = m_pGamePad[i]->Acquire();
		}

		result = m_pGamePad[i]->GetDeviceState(sizeof(DIJOYSTATE), &dijs);	// �f�o�C�X��Ԃ�ǂݎ��
		if ( result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED ) {
			result = m_pGamePad[i]->Acquire();
			while ( result == DIERR_INPUTLOST )
				result = m_pGamePad[i]->Acquire();
		}

		// �R�Q�̊e�r�b�g�ɈӖ����������A�{�^�������ɉ����ăr�b�g���I���ɂ���
		//* y-axis (forward)
		if ( dijs.lY < 0 )					m_padState[i] |= BUTTON_UP;
		//* y-axis (backward)
		if ( dijs.lY > 0 )					m_padState[i] |= BUTTON_DOWN;
		//* x-axis (left)
		if ( dijs.lX < 0 )					m_padState[i] |= BUTTON_LEFT;
		//* x-axis (right)
		if ( dijs.lX > 0 )					m_padState[i] |= BUTTON_RIGHT;
		//* �`�{�^��
		if ( dijs.rgbButtons[0] & 0x80 )	m_padState[i] |= BUTTON_A;
		//* �a�{�^��
		if ( dijs.rgbButtons[1] & 0x80 )	m_padState[i] |= BUTTON_B;
		//* �b�{�^��
		if ( dijs.rgbButtons[2] & 0x80 )	m_padState[i] |= BUTTON_C;
		//* �w�{�^��
		if ( dijs.rgbButtons[3] & 0x80 )	m_padState[i] |= BUTTON_X;
		//* �x�{�^��
		if ( dijs.rgbButtons[4] & 0x80 )	m_padState[i] |= BUTTON_Y;
		//* �y�{�^��
		if ( dijs.rgbButtons[5] & 0x80 )	m_padState[i] |= BUTTON_Z;
		//* �k�{�^��
		if ( dijs.rgbButtons[6] & 0x80 )	m_padState[i] |= BUTTON_L;
		//* �q�{�^��
		if ( dijs.rgbButtons[7] & 0x80 )	m_padState[i] |= BUTTON_R;
		//* �r�s�`�q�s�{�^��
		if ( dijs.rgbButtons[8] & 0x80 )	m_padState[i] |= BUTTON_START;
		//* �l�{�^��
		if ( dijs.rgbButtons[9] & 0x80 )	m_padState[i] |= BUTTON_M;

		// Trigger�ݒ�
		m_padTrigger[i] = ((lastPadState ^ m_padState[i])	// �O��ƈ���Ă���
						& m_padState[i]);					// ��������ON�̂��
		
	}

}
//----------------------------------------------- ����
BOOL InputManager::IsButtonPressed(int padNo,DWORD button)
{
	return (button & m_padState[padNo]);
}

BOOL InputManager::IsButtonTriggered(int padNo,DWORD button)
{
	return (button & m_padTrigger[padNo]);
}

void InputManager::SetMousePosCenter(void)
{
	HWND hwnd = GetForegroundWindow();

	RECT rect;
	GetClientRect(hwnd, &rect);  // �N���C�A���g�̈�擾

	POINT center;
	center.x = (rect.right - rect.left) / 2;
	center.y = (rect.bottom - rect.top) / 2;

	ClientToScreen(hwnd, &center); // �X�N���[�����W�ɕϊ�
	SetCursorPos(center.x, center.y); // �}�E�X�ʒu��ݒ�

	m_isMouseRecentered = true;            // �Z���^�����O�t���O��L����
}

void InputManager::SetMouseRecentered(BOOL recenter)
{
	m_isMouseRecentered = recenter;
}

BOOL InputManager::IsMouseRecentered(void)
{
	return m_isMouseRecentered;
}
	


