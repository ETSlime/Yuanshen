#pragma once
//=============================================================================
//
// �v���C���[���쁕��������Ǘ��N���X [Player.h]
// Author : 
// �v���C���[�̍s���E�ړ��E�`��E�G�t�F�N�g���ꊇ�ŒS�������l�������Ǘ��N���X�ł����I
// Sigewinne��Klee�����̐؂�ւ��������Ő��䂵�āA�A�N�V�����L���[�œ������������񂵂܂�
// 
//=============================================================================
#include "Scene/Character/Sigewinne.h"
#include "Scene/Character/Klee.h"
#include "Scene/Character/Lumine.h"
#include "Scene/Character/Hilichurl.h"
#include "Scene/Character/Mitachurl.h"
#include "Core/LightManager.h"
#include "Utility/InputManager.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define ACTION_QUEUE_SIZE		(4)
#define ACTION_QUEUE_CLEAR_WAIT	(40)
#define PLAYER_HIT_WINDOW		(90)
#define Action(action)			static_cast<UINT>(action)

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct PlayerAction
{
	ActionEnum	actionType;
	float		liveTime;
};

struct PlayerAttributes
{
	UINT			actionQueueClearTime;
	PlayerAction	actionQueue[ACTION_QUEUE_SIZE];
	UINT			actionQueueStart;
	UINT			actionQueueEnd;
};


class Player : public IDebugUI
{
public:
	Player();
	~Player();
	void Update(void);
	void Draw(void);
	void DrawEffect(void);
	Transform GetTransform(void) const;

private:

	void UpdateActionQueue(void);
	void HandlePlayerMove(Transform& transform);

	virtual void RenderImGui(void) override;
	virtual const char* GetPanelName(void) const override { return "Player"; };

	Attributes m_attributes;
	PlayerAttributes m_playerAttr;
	Sigewinne* sigewinne;
	Klee* klee;
	Lumine* lumine;

	Hilichurl* hilichurl;
	Mitachurl* mitachurl;

	bool m_drawBoundingBox = true;

	GameObject<SkinnedMeshModelInstance>* m_playerGO;

	Light* m_light;

	Renderer& m_renderer = Renderer::get_instance();
	LightManager& m_lightMgr = LightManager::get_instance();

	DebugProc& m_debugProc = DebugProc::get_instance();
	InputManager& m_inputManager = InputManager::get_instance();
};