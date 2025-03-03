//=============================================================================
//
// カメラ処理 [camera.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "input.h"
#include "camera.h"
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
static CAMERA			g_Camera;		// カメラデータ

static int				g_ViewPortType = TYPE_FULL_SCREEN;
static BOOL isDragging = FALSE;
static long startX = 0, startY = 0;
static long currentX = 0, currentY = 0;
static long deltaX = 0, deltaY = 0;

static Renderer& renderer = Renderer::get_instance();
//=============================================================================
// 初期化処理
//=============================================================================
void InitCamera(void)
{
	g_Camera.pos = { POS_X_CAM, POS_Y_CAM, POS_Z_CAM };
	g_Camera.at  = { 0.0f, 0.0f, 0.0f };
	g_Camera.up  = { 0.0f, 1.0f, 0.0f };
	g_Camera.rot = { 0.0f, 0.0f, 0.0f };
	g_Camera.fov = VIEW_ANGLE;

	// 視点と注視点の距離を計算
	float vx, vz;
	vx = g_Camera.pos.x - g_Camera.at.x;
	vz = g_Camera.pos.z - g_Camera.at.z;
	g_Camera.len = sqrtf(vx * vx + vz * vz);
	
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.y = g_Camera.at.y + sinf(g_Camera.rot.x) * g_Camera.len;

	// ビューポートタイプの初期化
	SetViewPort(g_ViewPortType);
}


//=============================================================================
// カメラの終了処理
//=============================================================================
void UninitCamera(void)
{

}


//=============================================================================
// カメラの更新処理
//=============================================================================
void UpdateCamera(void)
{
	if (!GetWindowActive()) return; // 非アクティブならカメラ更新スキップ

	// パラメータ設定
	const float MOUSE_SENSITIVITY = 0.003f;
	const float ZOOM_SENSITIVITY = 0.15f;
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
	static float originalDistance = g_Camera.len;
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

		if (deltaX != 0 || deltaY != 0)
			int a = 1;
		// 前回のマウス位置を更新
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

		// 水平方向の回転処理 (マウス左右移動)
		g_Camera.rot.y += deltaX * MOUSE_SENSITIVITY;

		// 水平方向の回転角度を -π 〜 π に正規化
		if (g_Camera.rot.y > XM_PI)
			g_Camera.rot.y -= XM_PI * 2.0f;
		if (g_Camera.rot.y < -XM_PI)
			g_Camera.rot.y += XM_PI * 2.0f;

		// 垂直方向の回転処理 (マウス上下移動)
		float desiredRotX = g_Camera.rot.x + deltaY * MOUSE_SENSITIVITY;
		if (desiredRotX > MIN_VERTICAL_ANGLE && desiredRotX <= MAX_VERTICAL_ANGLE)
		{
			if (g_Camera.len < originalDistance && deltaY > 0)
			{
				g_Camera.len += CAMERA_RETREAT_SPEED;
				if (g_Camera.len > originalDistance) g_Camera.len = originalDistance;
			}
			else
				g_Camera.rot.x = desiredRotX;
		}
		else if (desiredRotX <= MIN_VERTICAL_ANGLE)
		{
			// 地面に近づきすぎた場合の処理
			if (deltaY < 0)
			{
				g_Camera.len -= CAMERA_APPROACH_SPEED;
				//if (g_Camera.len > MIN_CAMERA_DISTANCE)
				//	g_Camera.rot.x += deltaY* MOUSE_SENSITIVITY * 0.3f;
				if (g_Camera.len < MIN_CAMERA_DISTANCE)
					g_Camera.len = MIN_CAMERA_DISTANCE;
			}
			else if (deltaY > 0 && g_Camera.len < originalDistance)
			{

				g_Camera.len += CAMERA_RETREAT_SPEED;
				//if (g_Camera.len < originalDistance)
				//	g_Camera.rot.x -= deltaY * MOUSE_SENSITIVITY * 0.6f;
				if (g_Camera.len > originalDistance)
					g_Camera.len = originalDistance;
			}
		}
		else if (desiredRotX > MAX_VERTICAL_ANGLE)
		{
			// 最大仰角を超えないように制限
			g_Camera.rot.x = MAX_VERTICAL_ANGLE;
		}

		// マウスホイールによるズーム処理
		long wheelDelta = GetMouseZ();
		if (wheelDelta != 0)
		{
			g_Camera.len -= wheelDelta * ZOOM_SENSITIVITY;

			if (g_Camera.len < MIN_CAMERA_DISTANCE) g_Camera.len = MIN_CAMERA_DISTANCE;
			if (g_Camera.len > MAX_CAMERA_DISTANCE) g_Camera.len = MAX_CAMERA_DISTANCE;

			originalDistance = g_Camera.len;
		}


		// マウス位置を中央に戻す
		//if (rotated)
		SetMousePosCenter();
	}



	// カメラ位置の更新
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * cosf(g_Camera.rot.x) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * cosf(g_Camera.rot.x) * g_Camera.len;
	g_Camera.pos.y = g_Camera.at.y + sinf(g_Camera.rot.x) * g_Camera.len;

	//if (!IsMouseRecentered())
	//	SetMousePosCenter();

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_Z))
	{// 視点旋回「左」
		g_Camera.rot.y += VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_C))
	{// 視点旋回「右」
		g_Camera.rot.y -= VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_Y))
	{// 視点移動「上」
		g_Camera.pos.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_N))
	{// 視点移動「下」
		g_Camera.pos.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_Q))
	{// 注視点旋回「左」
		g_Camera.rot.y -= VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}

		g_Camera.at.x = g_Camera.pos.x + sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.at.z = g_Camera.pos.z + cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_E))
	{// 注視点旋回「右」
		g_Camera.rot.y += VALUE_ROTATE_CAMERA;
		if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.at.x = g_Camera.pos.x + sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.at.z = g_Camera.pos.z + cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_LALT))
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
			deltaX = 0.0f;
			deltaY = 0.0f;
		}

		g_Camera.rot.x -= deltaY * VALUE_ROTATE_CAMERA * 0.1f;
		g_Camera.rot.y -= deltaX * VALUE_ROTATE_CAMERA * 0.1f;
		if (g_Camera.rot.y < -XM_PI)
		{
			g_Camera.rot.y += XM_PI * 2.0f;
		}
		else if (g_Camera.rot.y > XM_PI)
		{
			g_Camera.rot.y -= XM_PI * 2.0f;
		}

		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.y = g_Camera.at.y + sinf(g_Camera.rot.x) * g_Camera.len;
	}
	

	if (GetKeyboardPress(DIK_T))
	{// 注視点移動「上」
		g_Camera.at.y += VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_B))
	{// 注視点移動「下」
		g_Camera.at.y -= VALUE_MOVE_CAMERA;
	}

	if (GetKeyboardPress(DIK_U))
	{// 近づく
		g_Camera.len -= VALUE_MOVE_CAMERA;
		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	if (GetKeyboardPress(DIK_M))
	{// 離れる
		g_Camera.len += VALUE_MOVE_CAMERA;
		g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
		g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;
	}

	// カメラを初期に戻す
	if (GetKeyboardPress(DIK_R))
	{
		UninitCamera();
		InitCamera();
	}

#endif



#ifdef _DEBUG	// デバッグ情報を表示する
	PrintDebugProc("Camera:ZC QE TB YN UM R\n");
#endif
}


//=============================================================================
// カメラの更新
//=============================================================================
void SetCamera(void) 
{
	// ビューマトリックス設定
	XMMATRIX mtxView;
	mtxView = XMMatrixLookAtLH(XMLoadFloat3(&g_Camera.pos), XMLoadFloat3(&g_Camera.at), XMLoadFloat3(&g_Camera.up));
	renderer.SetViewMatrix(&mtxView);
	XMStoreFloat4x4(&g_Camera.mtxView, mtxView);

	XMMATRIX mtxInvView;
	mtxInvView = XMMatrixInverse(nullptr, mtxView);
	XMStoreFloat4x4(&g_Camera.mtxInvView, mtxInvView);


	// プロジェクションマトリックス設定
	XMMATRIX mtxProjection;
	mtxProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

	renderer.SetProjectionMatrix(&mtxProjection);
	XMStoreFloat4x4(&g_Camera.mtxProjection, mtxProjection);

	renderer.SetShaderCamera(g_Camera.pos);
}


//=============================================================================
// カメラの取得
//=============================================================================
CAMERA *GetCamera(void) 
{
	return &g_Camera;
}

//=============================================================================
// ビューポートの設定
//=============================================================================
void SetViewPort(int type)
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


int GetViewPortType(void)
{
	return g_ViewPortType;
}



// カメラの視点と注視点をセット
void SetCameraAT(XMFLOAT3 pos)
{
	// カメラの注視点を引数の座標にしてみる
	g_Camera.at = pos;

	// カメラの視点をカメラのY軸回転に対応させている
	g_Camera.pos.x = g_Camera.at.x - sinf(g_Camera.rot.y) * g_Camera.len;
	g_Camera.pos.z = g_Camera.at.z - cosf(g_Camera.rot.y) * g_Camera.len;

}

