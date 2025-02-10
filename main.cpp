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
#include "player.h"
#include "enemy.h"
#include "shadow.h"
#include "light.h"
#include "ground.h"
#include "sprite.h"
#include "score.h"
#include "offScreenRender.h"
#include "MapEditor.h"
#include "FBXLoader.h"
#include "TextureMgr.h"
#include "SkinnedMeshModel.h"
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

MapEditor& mapEditor = MapEditor::get_instance();
EnemyManager& enemyManager = EnemyManager::get_instance();
Ground* ground = nullptr;
TextureMgr mTexMgr;
FBXLoader& fbxLoader = FBXLoader::get_instance();
SkinnedMeshModel model;
Renderer& renderer = Renderer::get_instance();

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

	// ライトの初期化
	InitLight();

	// カメラの初期化
	InitCamera();

	// 入力処理の初期化
	InitInput(hInstance, hWnd);

	InitVertex();

	// 影の初期化処理
	InitShadow();

	// フィールドの初期化
	InitField();

	mTexMgr.Init(renderer.GetDevice());

	fbxLoader.LoadModel(renderer.GetDevice(), mTexMgr, model, "data/MODEL/enemy/Sigewinne","Character_output.fbx", nullptr);
	model.SetBodyDiffuseTexture(mTexMgr, "data/MODEL/enemy/Sigewinne/texture_0.png");
	model.SetHairDiffuseTexture(mTexMgr, "data/MODEL/enemy/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Hair_Diffuse.png");
	model.SetFaceDiffuseTexture(mTexMgr, "data/MODEL/enemy/Sigewinne/face.png");

	// プレイヤーの初期化
	InitPlayer();

	// エネミーの初期化
	//InitEnemy();
	enemyManager.Init();

	//InitGround();
	ground = new Ground();

	InitOffScreenRender();

	InitScore();

	mapEditor.Init();

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
	//UninitGround();
	delete ground;

	// エネミーの終了処理
	//UninitEnemy();

	// プレイヤーの終了処理
	UninitPlayer();

	// フィールドの終了処理
	UninitField();

	// 影の終了処理
	UninitShadow();

	UninitVertex();

	// カメラの終了処理
	UninitCamera();

	//入力の終了処理
	UninitInput();

	UninitScore();

	// レンダラーの終了処理
	renderer.Uninit();

	UninitOffScreenRender();

	mapEditor.Uninit();

	mTexMgr.Uninit();
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

	// フィールドの更新処理
	UpdateField();

	// プレイヤーの更新処理
	UpdatePlayer();

	// エネミーの更新処理
	//UpdateEnemy();
	enemyManager.Update();

	model.Update();

	//UpdateGround();
	ground->Update();

	// 影の更新処理
	UpdateShadow();

	mapEditor.Update();
}

//=============================================================================
// 描画処理
//=============================================================================
void Draw(void)
{
	// バックバッファクリア
	renderer.Clear();

	// プレイヤー視点
	XMFLOAT3 pos = GetPlayer()->trans[ALL].pos;
	pos.y = 0.0f;			// カメラ酔いを防ぐためにクリアしている
	SetCameraAT(pos);
	SetCamera();

	renderer.SetModelInputLayout();
	for (int i = 2; i >= 0; i--)
	{
		LIGHT* light = GetLightData(i);
		if (light->Enable == FALSE) continue;

		renderer.SetRenderShadowMap(i);

		//DrawPlayer();

		//DrawEnemy();

		//DrawGround();

		//DrawScene();
	}

	renderer.SetRenderObject();
	//DrawScene();

	renderer.SetLightEnable(FALSE);
	//mapEditor.Draw();
	//DrawScore();
	renderer.SetLightEnable(TRUE);
	
	renderer.SetRenderSkinnedMeshModel();
	renderer.SetSkinnedMeshInputLayout();
	model.Draw();

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
	DrawField();

	//DrawPlayer();

	//DrawEnemy();
	//enemyManager.Draw();

	//DrawGround();
	ground->Draw();
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
