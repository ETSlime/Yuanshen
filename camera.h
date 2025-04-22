//=============================================================================
//
// カメラ処理 [Camera.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Renderer.h"
#include "SingletonBase.h"
#include "Timer.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VIEW_ANGLE				(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT				((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z				(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z_SKYBOX		(800000.0f)										// ビュー平面のFarZ値
#define	VIEW_FAR_Z_SCENE		(50000.0f)										// ビュー平面のFarZ値

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
// プロトタイプ宣言
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

	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス
	XMMATRIX			m_projScene;
	XMMATRIX			m_projSkybox;

	XMFLOAT3			pos;			// カメラの視点(位置)
	XMFLOAT3			at;				// カメラの注視点
	XMFLOAT3			up;				// カメラの上方向ベクトル
	XMFLOAT3			rot;			// カメラの回転

	float				len;			// カメラの視点と注視点の距離
	float				fov;
	float				nearZ;
	float				farZ;

	CameraType			m_CameraType = CameraType::SCENE;

	Timer& timer = Timer::get_instance();
	DebugProc& debugProc = DebugProc::get_instance();
};