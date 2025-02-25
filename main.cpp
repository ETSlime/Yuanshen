//=============================================================================
//
// メイン処理 [main.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "field.h"
#include "model.h"
#include "enemy.h"
#include "light.h"
#include "Ground.h"
#include "sprite.h"
#include "score.h"
#include "offScreenRender.h"
#include "MapEditor.h"
#include "FBXLoader.h"
#include "TextureMgr.h"
#include "SkinnedMeshModel.h"
#include "Player.h"
#include "CollisionManager.h"
#include "Skybox.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define CLASS_NAME		"AppClass"			// ウインドウのクラス名
#define WINDOW_NAME		"メッシュ表示"		// ウインドウのキャプション名

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);

//*****************************************************************************
// グローバル変数:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;

bool g_IsWindowActive = true; // デフォルトでアクティブ

MapEditor& mapEditor = MapEditor::get_instance();
EnemyManager& enemyManager = EnemyManager::get_instance();
CollisionManager& collisionManager = CollisionManager::get_instance();
Renderer& renderer = Renderer::get_instance();
Ground* ground = nullptr;
Player* player = nullptr;
Skybox* skybox = nullptr;
CAMERA* camera = GetCamera();
//TextureMgr& mTexMgr = TextureMgr::get_instance();
//FBXLoader& fbxLoader = FBXLoader::get_instance();

#ifdef _DEBUG
int		g_CountFPS;							// FPSカウンタ
char	g_DebugStr[2048] = WINDOW_NAME;		// デバッグ文字表示用

#endif


//=============================================================================
// メイン関数
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// 無くても良いけど、警告が出る（未使用宣言）
	UNREFERENCED_PARAMETER(lpCmdLine);		// 無くても良いけど、警告が出る（未使用宣言）

	// 時間計測用
	DWORD dwExecLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;

	WNDCLASSEX	wcex = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		CLASS_NAME,
		NULL
	};
	HWND		hWnd;
	MSG			msg;
	
	// ウィンドウクラスの登録
	RegisterClassEx(&wcex);

	// ウィンドウの作成
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,																		// ウィンドウの左座標
		CW_USEDEFAULT,																		// ウィンドウの上座標
		SCREEN_WIDTH + GetSystemMetrics(SM_CXDLGFRAME) * 2,									// ウィンドウ横幅
		SCREEN_HEIGHT + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),	// ウィンドウ縦幅
		NULL,
		NULL,
		hInstance,
		NULL);

	// 初期化処理(ウィンドウを作成してから行う)
	if(FAILED(Init(hInstance, hWnd, TRUE)))
	{
		return -1;
	}

	// フレームカウント初期化
	timeBeginPeriod(1);	// 分解能を設定
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// システム時刻をミリ秒単位で取得
	dwCurrentTime = dwFrameCount = 0;

	// ウインドウの表示(初期化処理の後に呼ばないと駄目)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	// メッセージループ
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()が呼ばれたらループ終了
				break;
			}
			else
			{
				// メッセージの翻訳と送出
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1秒ごとに実行
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPSを測定した時刻を保存
				dwFrameCount = 0;							// カウントをクリア
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))	// 1/60秒ごとに実行
			{
				dwExecLastTime = dwCurrentTime;	// 処理した時刻を保存

#ifdef _DEBUG	// デバッグ版の時だけFPSを表示する
				wsprintf(g_DebugStr, WINDOW_NAME);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " FPS:%d", g_CountFPS);
#endif

				Update();			// 更新処理
				Draw();				// 描画処理

#ifdef _DEBUG	// デバッグ版の時だけ表示する
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// 分解能を戻す

	// ウィンドウクラスの登録を解除
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// 終了処理
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// プロシージャ
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;

	case WM_MOUSEMOVE:
		g_MouseX = LOWORD(lParam);
		g_MouseY = HIWORD(lParam);
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE)
		{
			// ウィンドウがアクティブになった
			g_IsWindowActive = true;
			ShowCursor(FALSE); // 必要に応じてカーソルを非表示
		}
		else
		{
			// ウィンドウが非アクティブになった
			g_IsWindowActive = false;
			ShowCursor(TRUE);  // カーソル表示
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// レンダラーの初期化
	renderer.Init(hInstance, hWnd, bWindow);

	// カメラの初期化
	InitCamera();

	// 入力処理の初期化
	InitInput(hInstance, hWnd);

	// プレイヤーの初期化
	player = new Player();

	// ライトの初期化
	InitLight(player);

	// エネミーの初期化
	enemyManager.Init();

	ground = new Ground();

	InitOffScreenRender();

	InitScore();

	mapEditor.Init();

	// skyboxの初期化
	skybox = new Skybox(renderer.GetDevice(), renderer.GetDeviceContext());

	// ライトを有効化
	renderer.SetLightEnable(TRUE);

	// 背面ポリゴンをカリング
	renderer.SetCullingMode(CULL_MODE_BACK);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Uninit(void)
{
	delete ground;

	delete player;

	// フィールドの終了処理
	UninitField();

	// カメラの終了処理
	UninitCamera();

	//入力の終了処理
	UninitInput();

	UninitScore();

	// レンダラーの終了処理
	Renderer::get_instance().Uninit();

	UninitOffScreenRender();

	mapEditor.Uninit();
}

//=============================================================================
// 更新処理
//=============================================================================
void Update(void)
{
	// 入力の更新処理
	UpdateInput();

	// カメラ更新
	UpdateCamera();

	UpdateLight();

	skybox->Update();

	// プレイヤーの更新処理
	player->Update();

	// エネミーの更新処理
	enemyManager.Update();

	ground->Update();

	mapEditor.Update();

	collisionManager.Update();
}

//=============================================================================
// 描画処理
//=============================================================================
void Draw(void)
{
	// バックバッファクリア
	renderer.Clear();


	skybox->Draw(XMLoadFloat4x4(&camera->mtxView), XMLoadFloat4x4(&camera->mtxProjection));

	for (int lightIdx = 2; lightIdx >= 0; lightIdx--)
	{
		LIGHT* light = GetLightData(lightIdx);
		if (light->Enable == FALSE) continue;

		renderer.ClearShadowDSV(lightIdx);
	}

	renderer.SetModelInputLayout();
	for (int i = 2; i >= 0; i--)
	{
		LIGHT* light = GetLightData(i);
		if (light->Enable == FALSE) continue;

		renderer.SetRenderShadowMap(i);

		DrawScene();
	}

	renderer.SetSkinnedMeshInputLayout();

	for (int i = 2; i >= 0; i--)
	{
		LIGHT* light = GetLightData(i);
		if (light->Enable == FALSE) continue;

		renderer.SetRenderSkinnedMeshShadowMap(i);
		player->Draw();
		ground->Draw();
	}

	renderer.SetModelInputLayout();
	renderer.SetRenderObject();
	DrawScene();

	//renderer.SetLightEnable(FALSE);
	//mapEditor.Draw();
	//renderer.SetLightEnable(TRUE);
	

	renderer.SetSkinnedMeshInputLayout();
	renderer.SetRenderSkinnedMeshModel();

	player->Draw();
	ground->Draw();

	//SetOffScreenRender();
	//DrawScene();

	//SetLightEnable(FALSE);
	//DrawOffScreenRender();
	//SetLightEnable(FALSE);

#ifdef _DEBUG
	// デバッグ表示
	DrawDebugProc();
#endif

	// バックバッファ、フロントバッファ入れ替え
	renderer.Present();
}

void DrawScene()
{
	enemyManager.Draw();

	ground->Draw();
}

bool GetWindowActive(void)
{
	return g_IsWindowActive;
}

long GetMousePosX(void)
{
	return g_MouseX;
}


long GetMousePosY(void)
{
	return g_MouseY;
}


#ifdef _DEBUG
char* GetDebugStr(void)
{
	return g_DebugStr;
}
#endif