//=============================================================================
//
// カメラ処理 [camera.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "renderer.h"
#include "SingletonBase.h"
#include "Timer.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************


enum 
{
	TYPE_FULL_SCREEN,
	TYPE_LEFT_HALF_SCREEN,
	TYPE_RIGHT_HALF_SCREEN,
	TYPE_UP_HALF_SCREEN,
	TYPE_DOWN_HALF_SCREEN,
	TYPE_NONE,
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

	XMFLOAT4X4 GeViewMtx(void) { return mtxView; }
	XMFLOAT3 GetRotation(void) { return rot; }

private:

	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス

	XMFLOAT3			pos;			// カメラの視点(位置)
	XMFLOAT3			at;				// カメラの注視点
	XMFLOAT3			up;				// カメラの上方向ベクトル
	XMFLOAT3			rot;			// カメラの回転

	float				len;			// カメラの視点と注視点の距離
	float				fov;

	Timer& timer = Timer::get_instance();
};