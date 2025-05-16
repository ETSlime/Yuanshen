//=============================================================================
//
// �f�o�b�O�����܂Ƃ߂Ėʓ|�݂�Ǘ����o���� [Debugproc.h]
// Author :
// ImGui�x�[�X��UI��o�^�E�`�悵�āA���O�o�͂�t���[�����Ƃ̃f�o�b�O�\���𐧌䂷��R�A�N���X�ł����I
// ����Ԃ�IDebugUI����񂽂���D�����܂Ƃ߂Ă����A��Âŗ����i�ߓ��Ȃ�~
// 
//=============================================================================
#pragma once
#include "main.h"
#include "Renderer.h"
#include "SingletonBase.h"
#include "SimpleArray.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

class IDebugUI
{
public:
	virtual void RenderImGui() = 0;
	virtual void RenderDebugInfo() {}
	virtual const char* GetPanelName() const = 0;
};

class DebugProc : public SingletonBase<DebugProc>
{
public:
	void Init(HWND hwnd);
	void Shutdown(void);

	void PrintDebugProc(char* fmt, ...);
	void Register(IDebugUI* ui);

	void BeginFrame(void);
	void Draw(void);


private:

	void DrawDebugProc(void);

	SimpleArray<IDebugUI*> m_debugUIs;
	HWND m_hwnd = nullptr;
	Renderer& m_Renderer = Renderer::get_instance();
	char m_aStrDebug[1024] = { "\0" };	// �f�o�b�O���
	static constexpr int MAX_DEBUG_LINES = 100;
	char m_lines[MAX_DEBUG_LINES][512];
	int m_lineCount = 0;
};