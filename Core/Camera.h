//=============================================================================
//
// �r���[�^�v���W�F�N�V�����s�񂨂�ю��_���䃆�[�e�B���e�B [Camera.h]
// Author : 
// �V�[���^�X�J�C�{�b�N�X�̎��_�ؑցAZ�����W�EFOV�EViewport�Ǘ��A
// ���_�ړ�����эs��X�V�𓝈�I�ɐ��䂷��J�����Ǘ��N���X
//
//=============================================================================
#pragma once

//*****************************************************************************
// �C���N���[�h�t�@�C��
//*****************************************************************************
#include "Core/Graphics/Renderer.h"
#include "Utility/SingletonBase.h"
#include "Core/Timer.h"
#include "Utility/InputManager.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	VIEW_ANGLE				(XMConvertToRadians(45.0f))						// �r���[���ʂ̎���p
#define	VIEW_ASPECT				((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// �r���[���ʂ̃A�X�y�N�g��	
#define	VIEW_NEAR_Z				(10.0f)											// �r���[���ʂ�NearZ�l
#define	VIEW_FAR_Z_SKYBOX		(800000.0f)										// �r���[���ʂ�FarZ�l
#define	VIEW_FAR_Z_SCENE		(50000.0f)										// �r���[���ʂ�FarZ�l

enum 
{
	TYPE_FULL_SCREEN,
	TYPE_LEFT_HALF_SCREEN,
	TYPE_RIGHT_HALF_SCREEN,
	TYPE_UP_HALF_SCREEN,
	TYPE_DOWN_HALF_SCREEN,
	TYPE_NONE,
};

enum class CameraType
{
	SKYBOX,
	SCENE,
};


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

class Camera : public SingletonBase<Camera>
{
public:
	void Init(void);
	void Shutdown(void);
	void Update(void);
	void SetCamera(void);

	void SetViewPort(int type);
	int GetViewPortType(void);

	void SetCameraAT(XMFLOAT3 pos);

	XMFLOAT4X4 GetViewMatrix(void) { return m_mtxView; }
	XMFLOAT4X4 GetProjMatrix(void) { return m_mtxProjection; }
	XMFLOAT3 GetRotation(void) { return m_rot; }
	XMMATRIX GetViewProjMtx(void) { return XMLoadFloat4x4(&m_mtxView) * XMLoadFloat4x4(&m_mtxProjection); }

	void SetCameraType(CameraType type);
	float GetNearZ(void) { return m_nearZ; }
	float GetFarZ(void) { return m_farZ; }
	float GetFov(void) { return m_fov; }
	float GetAspectRatio(void) { return VIEW_ASPECT; }

private:

	XMFLOAT4X4			m_mtxView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4			m_mtxInvView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4			m_mtxProjection;	// �v���W�F�N�V�����}�g���b�N�X
	XMMATRIX			m_projScene;
	XMMATRIX			m_projSkybox;

	XMFLOAT3			m_pos;			// �J�����̎��_(�ʒu)
	XMFLOAT3			m_at;				// �J�����̒����_
	XMFLOAT3			m_up;				// �J�����̏�����x�N�g��
	XMFLOAT3			m_rot;			// �J�����̉�]

	float				m_len;			// �J�����̎��_�ƒ����_�̋���
	float				m_fov;
	float				m_nearZ;
	float				m_farZ;

	CameraType			m_CameraType = CameraType::SCENE;

	Timer& m_timer = Timer::get_instance();
	DebugProc& m_debugProc = DebugProc::get_instance();
	InputManager& m_inputManager = InputManager::get_instance();
};