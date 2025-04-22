//=============================================================================
//
// �f�o�b�O�\������ [Debugproc.cpp]
// Author : 
//
//=============================================================================
#include <stdio.h>
#include "Debugproc.h"
#include "ShadowMapRenderer.h"

void DebugProc::Init(HWND hwnd)
{
	m_hwnd = hwnd;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(m_Renderer.GetDevice(), m_Renderer.GetDeviceContext());

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
}

void DebugProc::Uninit()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

//=============================================================================
// �f�o�b�O�\�������̕`�揈��
//=============================================================================
void DebugProc::DrawDebugProc(void)
{
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

	m_Renderer.DebugTextOut(m_aStrDebug, 0, 0);

	// ���N���A
	memset(m_aStrDebug, 0, sizeof m_aStrDebug);
}

//=============================================================================
// �f�o�b�O�\���̓o�^
//=============================================================================
void DebugProc::PrintDebugProc(char* fmt, ...)
{
#if 0
	long* pParam;
	static char aBuf[256];

	pParam = (long*)&fmt;
	sprintf(aBuf, fmt, pParam[1], pParam[2], pParam[3], pParam[4],
		pParam[5], pParam[6], pParam[7], pParam[8],
		pParam[9], pParam[10], pParam[11], pParam[12]);
#else
	va_list list;			// �ψ�������������ׂɎg�p����ϐ�
	char* pCur;
	char aBuf[256] = { "\0" };
	char aWk[32];

	// �ψ����ɃA�N�Z�X����O�̏�������
	va_start(list, fmt);

	pCur = fmt;
	for (; *pCur; ++pCur)
	{
		if (*pCur != '%')
		{
			sprintf_s(aWk, "%c", *pCur);
		}
		else
		{
			pCur++;

			switch (*pCur)
			{
			case 'd':
				// �ψ����ɃA�N�Z�X���Ă��̕ϐ������o������
				sprintf_s(aWk, "%d", va_arg(list, int));
				break;

			case 'f':
				// �ψ����ɃA�N�Z�X���Ă��̕ϐ������o������
				sprintf_s(aWk, "%.2f", va_arg(list, double));		// double�^�Ŏw��
				break;

			case 's':
				// �ψ����ɃA�N�Z�X���Ă��̕ϐ������o������
				sprintf_s(aWk, "%s", va_arg(list, char*));
				break;

			case 'c':
				// �ψ����ɃA�N�Z�X���Ă��̕ϐ������o������
				sprintf_s(aWk, "%c", va_arg(list, char));
				break;

			default:
				sprintf_s(aWk, "%c", *pCur);
				break;
			}
		}
		strcat_s(aBuf, aWk);
	}

	// �ψ����ɃA�N�Z�X������̏I������
	va_end(list);

	// �A��
	if ((strlen(m_aStrDebug) + strlen(aBuf)) < ((sizeof m_aStrDebug) - 1))
	{
		strcat_s(m_aStrDebug, aBuf);
	}
#endif
}

void DebugProc::Register(IDebugUI* ui)
{
	m_debugUIs.push_back(ui);
}

void DebugProc::BeginFrame(void)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_lineCount = 0;
}

void DebugProc::Draw(void)
{
	DrawDebugProc();

	//ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
	//ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	ImGui::Begin("Debug Panel", nullptr, ImGuiWindowFlags_None);

	for (auto* ui : m_debugUIs)
	{
		if (ImGui::CollapsingHeader(ui->GetPanelName()))
			ui->RenderImGui();
		ui->RenderDebugInfo();
	}

	for (int i = 0; i < m_lineCount; ++i)
	{
		ImGui::TextUnformatted(m_lines[i]);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


