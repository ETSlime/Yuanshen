//=============================================================================
//
// ���C������ [main.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "Renderer.h"
#include "input.h"
#include "Camera.h"
#include "Debugproc.h"
#include "Model.h"
#include "EnemyManager.h"
#include "LightManager.h"
#include "Ground.h"
#include "sprite.h"
#include "score.h"
#include "offScreenRender.h"
#include "FBXLoader.h"
#include "TextureMgr.h"
#include "SkinnedMeshModel.h"
#include "CollisionManager.h"
#include "Timer.h"
#include "ShaderManager.h"
#include "ShadowMapRenderer.h"
#include "Scene.h"
#include "UIManager.h"
#include "GameSystem.h"
#include "PauseModal.h"
#include "CursorManager.h"
#include "imgui/imgui_impl_win32.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define CLASS_NAME		"AppClass"			// �E�C���h�E�̃N���X��
#define WINDOW_NAME		"���b�V���\��"		// �E�C���h�E�̃L���v�V������

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void Uninit(void);
void Update(void);
void Draw(void);

// ImGui Win32 ���̓n���h���[�̑O���錾
// - �w�b�_�[�� <windows.h> ���C���N���[�h���Ȃ����߂̉����
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//*****************************************************************************
// �O���[�o���ϐ�:
//*****************************************************************************
long g_MouseX = 0;
long g_MouseY = 0;

bool g_IsWindowActive = true; // �f�t�H���g�ŃA�N�e�B�u
bool g_IsGamePaused = false;; // �Q�[�����ꎞ��~�����ǂ���

//MapEditor& mapEditor = MapEditor::get_instance();
DebugProc& debugProc = DebugProc::get_instance();
GameSystem& gameSystem = GameSystem::get_instance();
CollisionManager& collisionManager = CollisionManager::get_instance();
Renderer& renderer = Renderer::get_instance();
Timer& timer = Timer::get_instance();
Camera& camera = Camera::get_instance();
LightManager& lightManager = LightManager::get_instance();
ShaderManager& shaderManager = ShaderManager::get_instance();
ShadowMapRenderer& shadowMapRenderer = ShadowMapRenderer::get_instance();
Scene& scene = Scene::get_instance();
UIManager& uiManager = UIManager::get_instance();
CursorManager& cursorManager = CursorManager::get_instance();

#ifdef _DEBUG
int		g_CountFPS;							// FPS�J�E���^
char	g_DebugStr[2048] = WINDOW_NAME;		// �f�o�b�O�����\���p

#endif

//=============================================================================
// ���C���֐�
//=============================================================================
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);	// �����Ă��ǂ����ǁA�x�����o��i���g�p�錾�j
	UNREFERENCED_PARAMETER(lpCmdLine);		// �����Ă��ǂ����ǁA�x�����o��i���g�p�錾�j

	// ���Ԍv���p
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
	
	// �E�B���h�E�N���X�̓o�^
	RegisterClassEx(&wcex);

	// �E�B���h�E�̍쐬
	hWnd = CreateWindow(CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,																		// �E�B���h�E�̍����W
		CW_USEDEFAULT,																		// �E�B���h�E�̏���W
		SCREEN_WIDTH + GetSystemMetrics(SM_CXDLGFRAME) * 2,									// �E�B���h�E����
		SCREEN_HEIGHT + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),	// �E�B���h�E�c��
		NULL,
		NULL,
		hInstance,
		NULL);

	// ����������(�E�B���h�E���쐬���Ă���s��)
	if(FAILED(Init(hInstance, hWnd, TRUE)))
	{
		return -1;
	}

	// �t���[���J�E���g������
	timeBeginPeriod(1);	// ����\��ݒ�
	dwExecLastTime = dwFPSLastTime = timeGetTime();	// �V�X�e���������~���b�P�ʂŎ擾
	dwCurrentTime = dwFrameCount = 0;

	// �E�C���h�E�̕\��(�����������̌�ɌĂ΂Ȃ��Ƒʖ�)
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	if (!(GetForegroundWindow() == hWnd))
	{
		gameSystem.PauseGame();
	}

	// ���b�Z�[�W���[�v
	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{// PostQuitMessage()���Ă΂ꂽ�烋�[�v�I��
				break;
			}
			else
			{
				// ���b�Z�[�W�̖|��Ƒ��o
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)	// 1�b���ƂɎ��s
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;
#endif
				dwFPSLastTime = dwCurrentTime;				// FPS�𑪒肵��������ۑ�
				dwFrameCount = 0;							// �J�E���g���N���A
			}

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))	// 1/60�b���ƂɎ��s
			{
				dwExecLastTime = dwCurrentTime;	// ��������������ۑ�

#ifdef _DEBUG	// �f�o�b�O�ł̎�����FPS��\������
				wsprintf(g_DebugStr, WINDOW_NAME);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " FPS:%d", g_CountFPS);
#endif

				Update();			// �X�V����
				Draw();				// �`�揈��

#ifdef _DEBUG	// �f�o�b�O�ł̎������\������
				wsprintf(&g_DebugStr[strlen(g_DebugStr)], " MX:%d MY:%d", GetMousePosX(), GetMousePosY());
				SetWindowText(hWnd, g_DebugStr);
#endif

				dwFrameCount++;
			}
		}
	}

	timeEndPeriod(1);				// ����\��߂�

	// �E�B���h�E�N���X�̓o�^������
	UnregisterClass(CLASS_NAME, wcex.hInstance);

	// �I������
	Uninit();

	return (int)msg.wParam;
}

//=============================================================================
// �v���V�[�W��
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ImGui �̃}�E�X/�L�[�{�[�h���͂���������֐�
	// - TRUE ��Ԃ����ꍇ�̓A�v�����ŏ������s��Ȃ��悤�ɂ���
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;


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

	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{
			// �}�E�X�J�[�\����ݒ肷��
			SetCursor(cursorManager.GetCurrentCursor());
			return TRUE;
		}
		break;
	case WM_SYSKEYDOWN:
		if (wParam == VK_MENU) // VK_MENU = Alt�L�[
		{
			// Alt�L�[�����������̕W��������L�����Z��
			return 0;
		}
		break;
	case WM_SYSCHAR:
		return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE)
		{
			// �E�B���h�E���A�N�e�B�u�ɂȂ���
			g_IsWindowActive = true;
			g_IsGamePaused = false;
			cursorManager.RememberCursorPosition(); // �J�[�\���ʒu��ۑ�
			SetMouseRecentered(true); // �J������]�h�~�p�t���O�ݒ�
		}
		else
		{
			// �E�B���h�E����A�N�e�B�u�ɂȂ���
			g_IsWindowActive = false;
			g_IsGamePaused = true;
			
			// �����|�[�Y�i�蓮���͂Ɠ��������j
			gameSystem.PauseGame();
			uiManager.SetModalIfNotExist<PauseModal>();
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//=============================================================================
// ����������
//=============================================================================
HRESULT Init(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	// �����_���[�̏�����
	renderer.Init(hInstance, hWnd, bWindow);

	debugProc.Init(hWnd);

	shaderManager.Init(renderer.GetDevice());
	renderer.SetShadersets();

	shadowMapRenderer.Init(CSM_SHADOW_MAP_SIZE, MAX_CASCADES);

	// �J�����̏�����
	camera.Init();

	gameSystem.Init();

	// ���͏����̏�����
	InitInput(hInstance, hWnd);

	InitOffScreenRender();

	InitScore();

	//mapEditor.Init();

	// ���C�g��L����
	lightManager.SetLightEnable(TRUE);

	// �w�ʃ|���S�����J�����O
	renderer.SetCullingMode(CULL_MODE_BACK);

	timer.Init();

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void Uninit(void)
{
	// �J�����̏I������
	camera.Uninit();

	//���͂̏I������
	UninitInput();

	UninitScore();

	gameSystem.Uninit();

	// �����_���[�̏I������
	renderer.Uninit();

	UninitOffScreenRender();

	debugProc.Uninit();

	//mapEditor.Uninit();
}

//=============================================================================
// �X�V����
//=============================================================================
void Update(void)
{
	timer.Update();

	// ���͂̍X�V����
	UpdateInput();

	// �J�����X�V
	camera.Update();

	// �Q�[���V�X�e���̍X�V
	gameSystem.Update();

	//mapEditor.Update();

	collisionManager.Update();

	lightManager.Update();
}

//=============================================================================
// �`�揈��
//=============================================================================
void Draw(void)
{
	// �o�b�N�o�b�t�@�N���A
	renderer.Clear();


	// �V���h�E�}�b�v�̕`��
	gameSystem.RenderShadowPass();

	// ���C���p�X�`��
	gameSystem.Draw();

	//const DoubleLinkedList<Light*>& lightList = lightManager.GetLightList();
	//int lightIdx = 0;
	//for (const auto& light : lightList)
	//{
	//	if (light->GetLightData().Enable == FALSE) continue;
	//	if (lightIdx >= LIGHT_MAX) continue;

	//	renderer.ClearShadowDSV(lightIdx);
	//	lightIdx++;
	//}

	//renderer.SetShadowPassViewport();

	//// obj model shadow map
	//renderer.SetModelInputLayout();
	//lightIdx = 0;
	//for (const auto& light : lightList)
	//{
	//	if (light->GetLightData().Enable == FALSE) continue;
	//	if (lightIdx >= LIGHT_MAX) continue;

	//	renderer.SetRenderShadowMap(lightIdx);
	//	ground->Draw();

	//	lightIdx++;
	//}

	//// skinned mesh model shadow map
	//renderer.SetSkinnedMeshInputLayout();
	//lightIdx = 0;
	//for (const auto& light : lightList)
	//{
	//	if (light->GetLightData().Enable == FALSE) continue;
	//	if (lightIdx >= LIGHT_MAX) continue;

	//	renderer.SetRenderSkinnedMeshShadowMap(lightIdx);
	//	player->Draw();
	//	enemyManager.Draw();
	//	ground->Draw();

	//	lightIdx++;
	//}

	//// instance shadow map
	//lightIdx = 0;
	//for (const auto& light : lightList)
	//{
	//	if (light->GetLightData().Enable == FALSE) continue;
	//	if (lightIdx >= LIGHT_MAX) continue;

	//	renderer.SetRenderInstanceShadowMap(lightIdx);
	//	ground->Draw();

	//	lightIdx++;
	//}

#ifdef _DEBUG
	// �[�x�e�X�g�𖳌���
	renderer.SetDepthEnable(FALSE);
	// �f�o�b�O�\��
	debugProc.BeginFrame();
	debugProc.Draw();
	// �[�x�e�X�g��L����
	renderer.SetDepthEnable(TRUE);
#endif

	// �o�b�N�o�b�t�@�A�t�����g�o�b�t�@����ւ�
	renderer.Present();
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

int GetRand(int min, int max)
{
	static int flagRand = 0;
	static std::mt19937 g_mt;

	if (flagRand == 0)
	{
		// �����_����������
		std::random_device rnd;	// �񌈒�I�ȗ���������
		g_mt.seed(rnd());		// �����Z���k�E�c�C�X�^�Ł@�����͏���SEED
		flagRand = 1;
	}

	std::uniform_int_distribution<> random(min, max);	// ���������_����0�`100�͈̔�
	int answer = random(g_mt);
	return answer;
}


float GetRandFloat(float min, float max) 
{
	static std::mt19937 g_mt(std::random_device{}());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(g_mt);
}