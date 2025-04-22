//=============================================================================
//
// �J�������� [Camera.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// �C���N���[�h�t�@�C��
//*****************************************************************************
#include "Renderer.h"
#include "SingletonBase.h"
#include "Timer.h"
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
	void Uninit(void);
	void Update(void);
	void SetCamera(void);

	void SetViewPort(int type);
	int GetViewPortType(void);

	void SetCameraAT(XMFLOAT3 pos);

	XMFLOAT4X4 GetViewMatrix(void) { return mtxView; }
	XMFLOAT4X4 GetProjMatrix(void) { return mtxProjection; }
	XMFLOAT3 GetRotation(void) { return rot; }
	XMMATRIX GetViewProjMtx(void) { return XMLoadFloat4x4(&mtxView) * XMLoadFloat4x4(&mtxProjection); }

	void SetCameraType(CameraType type);
	float GetNearZ(void) { return nearZ; }
	float GetFarZ(void) { return farZ; }
	float GetFov(void) { return fov; }
	float GetAspectRatio(void) { return VIEW_ASPECT; }

private:

	XMFLOAT4X4			mtxView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4			mtxInvView;		// �r���[�}�g���b�N�X
	XMFLOAT4X4			mtxProjection;	// �v���W�F�N�V�����}�g���b�N�X
	XMMATRIX			m_projScene;
	XMMATRIX			m_projSkybox;

	XMFLOAT3			pos;			// �J�����̎��_(�ʒu)
	XMFLOAT3			at;				// �J�����̒����_
	XMFLOAT3			up;				// �J�����̏�����x�N�g��
	XMFLOAT3			rot;			// �J�����̉�]

	float				len;			// �J�����̎��_�ƒ����_�̋���
	float				fov;
	float				nearZ;
	float				farZ;

	CameraType			m_CameraType = CameraType::SCENE;

	Timer& timer = Timer::get_instance();
	DebugProc& debugProc = DebugProc::get_instance();
};