//=============================================================================
//
// カメラ処理 [Camera.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "Camera.h"
#include "debugproc.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	POS_X_CAM			(0.0f)			// カメラの初期位置(X座標)
#define	POS_Y_CAM			(50.0f)			// カメラの初期位置(Y座標)
#define	POS_Z_CAM			(-480.0f)		// カメラの初期位置(Z座標)


#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))						// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(800000.0f)										// ビュー平面のFarZ値

#define	VALUE_MOVE_CAMERA	(2.0f)										// カメラの移動量
#define	VALUE_ROTATE_CAMERA	(XM_PI * 0.01f)								// カメラの回転量

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static int				g_ViewPortType = TYPE_FULL_SCREEN;
static BOOL isDragging = FALSE;
static long startX = 0, startY = 0;
static long currentX = 0, currentY = 0;
static long deltaX = 0, deltaY = 0;

static Renderer& renderer = Renderer::get_instance();
//=============================================================================
// 初期化処理
//=============================================================================
void Camera::Init(void)
{
	pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	at  = { 0.0f, 0.0f, 0.0f };
	up  = { 0.0f, 1.0f, 0.0f };
	rot = { 0.0f, 0.0f, 0.0f };
	fov = VIEW_ANGLE;

	// 視点と注視点の距離を計算
	float vx, vz;
	vx = pos.x - at.x;
	vz = pos.z - at.z;
	len = sqrtf(vx * vx + vz * vz);
	
	pos.x = at.x - sinf(rot.y) * len;
	pos.z = at.z - cosf(rot.y) * len;
	pos.y = at.y + sinf(rot.x) * len;

	nearZ = VIEW_NEAR_Z;
	farZ = VIEW_FAR_Z;

	// ビューポートタイプの初期化
	SetViewPort(g_ViewPortType);
}


//=============================================================================
// カメラの終了処理
//=============================================================================
void Camera::Uninit(void)
{

}


//=============================================================================
// カメラの更新処理
//=============================================================================
void Camera::Update(void)
{
	if (!GetWindowActive()) return; // 非アクティブならカメラ更新スキップ

	// パラメータ設定
	const float MOUSE_SENSITIVITY = 0.003f;
	const float ZOOM_SENSITIVITY = 0.25f;
	const float MIN_CAMERA_DISTANCE = 320.0f;
	const float MAX_CAMERA_DISTANCE = 860.0f;
	const float CAMERA_APPROACH_SPEED = 3.0f;
	const float CAMERA_RETREAT_SPEED = 5.0f;
	const float GROUND_LIMIT_ANGLE = 0.3f;

	// 垂直回転角度の制限 (正値は上方向、負値は下方向)
	const float MAX_VERTICAL_ANGLE = XM_PIDIV2 - 0.1f;
	const float MIN_VERTICAL_ANGLE = -0.1f;

	// マウスの前回位置を保持する静的変数
	static long prevMouseX = 0;
	static long prevMouseY = 0;
	// 元のカメラ距離を保存
	static float originalDistance = len;
	static bool rotated = false;


	if (IsMouseRecentered())
	{
		prevMouseX = GetMousePosX();
		prevMouseY = GetMousePosY();
		SetMouseRecentered(false);  // 次のフレームから有効
	}
	else
	{
		long currentMouseX = GetMousePosX();
		long currentMouseY = GetMousePosY();

		// マウスの移動量（フレーム間の差分）を計算
		float deltaX = static_cast<float>(currentMouseX - prevMouseX);
		float deltaY = static_cast<float>(currentMouseY - prevMouseY);

		// 前回のマウス位置を更新
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

		// 水平方向の回転処理 (マウス左右移動)
		rot.y += deltaX * MOUSE_SENSITIVITY * timer.GetScaledDeltaTime();

		// 水平方向の回転角度を -π 〜 π に正規化
		if (rot.y > XM_PI)
			rot.y -= XM_2PI;
		if (rot.y < -XM_PI)
			rot.y += XM_2PI;

		// 垂直方向の回転処理 (マウス上下移動)
		float desiredRotX = rot.x + deltaY * MOUSE_SENSITIVITY * timer.GetScaledDeltaTime();
		if (desiredRotX > MIN_VERTICAL_ANGLE && desiredRotX <= MAX_VERTICAL_ANGLE)
		{
			if (len < originalDistance && deltaY > 0)
			{
				len += CAMERA_RETREAT_SPEED * timer.GetScaledDeltaTime();
				if (len > originalDistance) len = originalDistance;
			}
			else
				rot.x = desiredRotX;
		}
		else if (desiredRotX <= MIN_VERTICAL_ANGLE)
		{
			// 地面に近づきすぎた場合の処理
			if (deltaY < 0)
			{
				len -= CAMERA_APPROACH_SPEED * timer.GetScaledDeltaTime();
				//if (len > MIN_CAMERA_DISTANCE)
				//	rot.x += deltaY* MOUSE_SENSITIVITY * 0.3f;
				if (len < MIN_CAMERA_DISTANCE)
					len = MIN_CAMERA_DISTANCE;
			}
			else if (deltaY > 0 && len < originalDistance)
			{

				len += CAMERA_RETREAT_SPEED * timer.GetScaledDeltaTime();
				//if (len < originalDistance)
				//	rot.x -= deltaY * MOUSE_SENSITIVITY * 0.6f;
				if (len > originalDistance)
					len = originalDistance;
			}
		}
		else if (desiredRotX > MAX_VERTICAL_ANGLE)
		{
			// 最大仰角を超えないように制限
			rot.x = MAX_VERTICAL_ANGLE;
		}

		// マウスホイールによるズーム処理
		long wheelDelta = GetMouseZ();
		if (wheelDelta != 0)
		{
			len -= wheelDelta * ZOOM_SENSITIVITY * timer.GetScaledDeltaTime();

			if (len < MIN_CAMERA_DISTANCE) len = MIN_CAMERA_DISTANCE;
			if (len > MAX_CAMERA_DISTANCE) len = MAX_CAMERA_DISTANCE;

			originalDistance = len;
		}


		// マウス位置を中央に戻す
		//if (rotated)
		SetMousePosCenter();
	}



	// カメラ位置の更新
	pos.x = at.x - sinf(rot.y) * cosf(rot.x) * len;
	pos.z = at.z - cosf(rot.y) * cosf(rot.x) * len;
	pos.y = at.y + sinf(rot.x) * len;

	//if (!IsMouseRecentered())
	//	SetMousePosCenter();

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_Z))
	{// 視点旋回「左」
		rot.y += VALUE_ROTATE_CAMERA;
		if (rot.y > XM_PI)
		{
			rot.y -= XM_2PI;
		}

		pos.x = at.x - sinf(rot.y) * len;
		pos.z = at.z - cosf(rot.y) * len;
	}

	if (GetKeyboardPress(DIK_C))
	{// 視点旋回「右」
		rot.y -= VALUE_ROTATE_CAMERA;
		if (rot.y < -XM_PI)
		{
			rot.y += XM_2PI;
		}

		pos.x = at.x - sinf(rot.y) * len;
		pos.z = at.z - cosf(rot.y) * len;
	}

	if (GetKeyboardPress(DIK_Y))
	{// 視点移動「上」
		pos.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_N))
	{// 視点移動「下」
		pos.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_Q))
	{// 注視点旋回「左」
		rot.y -= VALUE_ROTATE_CAMERA;
		if (rot.y < -XM_PI)
		{
			rot.y += XM_2PI;
		}

		at.x = pos.x + sinf(rot.y) * len;
		at.z = pos.z + cosf(rot.y) * len;
	}

	if (GetKeyboardPress(DIK_E))
	{// 注視点旋回「右」
		rot.y += VALUE_ROTATE_CAMERA;
		if (rot.y > XM_PI)
		{
			rot.y -= XM_2PI;
		}

		at.x = pos.x + sinf(rot.y) * len;
		at.z = pos.z + cosf(rot.y) * len;
	}

	if (GetKeyboardPress(DIK_LCONTROL))
	{
		if (IsMouseLeftTriggered())
		{
			isDragging = TRUE;
			startX = GetMousePosX();
			startY = GetMousePosY();
			currentX = startX;
			currentY = startY;
			deltaX = 0;
			deltaY = 0;
		}
		else if (IsMouseLeftPressed() && isDragging == TRUE)
		{
			long newX = GetMousePosX();
			long newY = GetMousePosY();
			deltaX = newX - currentX;
			deltaY = newY - currentY;
			currentX = newX;
			currentY = newY;

		}
		else if (!IsMouseLeftPressed())
		{
			isDragging = FALSE;
			deltaX = 0L;
			deltaY = 0L;
		}

		rot.x -= deltaY * VALUE_ROTATE_CAMERA * 0.1f * timer.GetScaledDeltaTime();
		rot.y -= deltaX * VALUE_ROTATE_CAMERA * 0.1f * timer.GetScaledDeltaTime();
		if (rot.y < -XM_PI)
		{
			rot.y += XM_2PI;
		}
		else if (rot.y > XM_PI)
		{
			rot.y -= XM_2PI;
		}

		pos.x = at.x - sinf(rot.y) * len;
		pos.z = at.z - cosf(rot.y) * len;
		pos.y = at.y + sinf(rot.x) * len;
	}
	

	if (GetKeyboardPress(DIK_T))
	{// 注視点移動「上」
		at.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_B))
	{// 注視点移動「下」
		at.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_U))
	{// 近づく
		len -= VALUE_MOVE_CAMERA;
		pos.x = at.x - sinf(rot.y) * len;
		pos.z = at.z - cosf(rot.y) * len;
	}

	if (GetKeyboardPress(DIK_M))
	{// 離れる
		len += VALUE_MOVE_CAMERA;
		pos.x = at.x - sinf(rot.y) * len;
		pos.z = at.z - cosf(rot.y) * len;
	}

	// カメラを初期に戻す
	if (GetKeyboardPress(DIK_R))
	{
		Uninit();
		Init();
	}

#endif



#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Camera:ZC QE TB YN UM R\n");
#endif
}


//=============================================================================
// カメラの更新
//=============================================================================
void Camera::SetCamera(void)
{
	// ビューマトリックス設定
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&at), XMLoadFloat3(&up));
	renderer.SetViewMatrix(&mtxView);
	XMStoreFloat4x4(&this->mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&this->mtxInvView, mtxInvView);


	// プロジェクションマトリックス設定
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, nearZ, farZ);

	renderer.SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&this->mtxProjection, mtxProjection);

	renderer.SetShaderCamera(pos);
}

//=============================================================================
// ビューポートの設定
//=============================================================================
void Camera::SetViewPort(int type)
{
	ID3D11DeviceContext *g_ImmediateContext = renderer.GetDeviceContext();
	D3D11_VIEWPORT vp;

	g_ViewPortType = type;

	// ビューポート設定
	switch (g_ViewPortType)
	{
	case TYPE_FULL_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_LEFT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_RIGHT_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH / 2;
		vp.Height = (FLOAT)SCREEN_HEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = (FLOAT)SCREEN_WIDTH / 2;
		vp.TopLeftY = 0;
		break;

	case TYPE_UP_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		break;

	case TYPE_DOWN_HALF_SCREEN:
		vp.Width = (FLOAT)SCREEN_WIDTH;
		vp.Height = (FLOAT)SCREEN_HEIGHT / 2;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = (FLOAT)SCREEN_HEIGHT / 2;
		break;


	}
	g_ImmediateContext->RSSetViewports(1, &vp);

}


int Camera::GetViewPortType(void)
{
	return g_ViewPortType;
}



// カメラの視点と注視点をセット
void Camera::SetCameraAT(XMFLOAT3 pos)
{
	// カメラの注視点を引数の座標にしてみる
	at = pos;

	// カメラの視点をカメラのY軸回転に対応させている
	pos.x = at.x - sinf(rot.y) * len;
	pos.z = at.z - cosf(rot.y) * len;

}

