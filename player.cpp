//=============================================================================
//
// player処理 [Player.cpp]
// Author : 
//
//=============================================================================

#include "Renderer.h"
#include "Player.h"
#include "input.h"
#include "Camera.h"
#include "debugproc.h"
#include "OctreeNode.h"
#include "CursorManager.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VALUE_MOVE			(4.5f)							// 移動量
#define VALUE_RUN			(120.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量
#define ROTATION_SPEED				(0.18f)
#define FALLING_SPEED		(2.50f)
#define SPD_DECAY_RATE		(0.93f)

#define PLAYER_INIT_POS		XMFLOAT3(14582.0f, -2424.0f, -19485.0f)
//#define PLAYER_INIT_POS		XMFLOAT3(0.0f, -0.0f, -0.0f)
//=============================================================================
// 初期化処理
//=============================================================================
Player::Player()
{
	//sigewinne = new Sigewinne();

	//klee =  new Klee();

	lumine = new Lumine();

	//hilichurl = new Hilichurl();

	//mitachurl = new Mitachurl();

	Transform transform;
	transform.pos = PLAYER_INIT_POS;
	lumine->SetTransform(transform);
	playerGO = lumine;

	light = new DirectionalLight();
	light->SetEnable(true);
	light->BindToTransform(playerGO->GetTransformP());
	lightMgr.AddLight(light);
}

Player::~Player()
{
	SAFE_DELETE(sigewinne);
	SAFE_DELETE(klee);
	SAFE_DELETE(lumine);
}

void Player::Update(void)
{
	UpdateActionQueue();

	attributes = playerGO->GetAttributes();
	Transform transform = playerGO->GetTransform();

	if (lumine && GetKeyboardTrigger(DIK_1))
	{
		lumine->SetTransform(transform);
		//lumine->SetRenderProgress(0.0f);
		//lumine->SetSwitchCharEffect(true);
		playerGO = lumine;
		light->BindToTransform(playerGO->GetTransformP());
	}
	if (klee && GetKeyboardTrigger(DIK_2))
	{
		klee->SetTransform(transform);
		//klee->SetRenderProgress(0.0f);
		//klee->SetSwitchCharEffect(true);
		playerGO = klee;
		light->BindToTransform(playerGO->GetTransformP());
	}
	if (sigewinne && GetKeyboardTrigger(DIK_3))
	{
		sigewinne->SetTransform(transform);
		//sigewinne->SetRenderProgress(0.0f);
		//sigewinne->SetSwitchCharEffect(true);
		playerGO = sigewinne;
		light->BindToTransform(playerGO->GetTransformP());
	}

	Camera& camera = Camera::get_instance();

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_UP))
	{
		transform.pos.y += 50;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{
		transform.pos.y -= 50;
	}
#endif

	if (IsMouseLeftTriggered() && !CursorManager::get_instance().IsMouseFreeMode())
	{
		PlayerAction action;
		action.actionType = ActionEnum::ATTACK;
		action.liveTime = 0;
		playerAttr.actionQueue[playerAttr.actionQueueEnd] = action;
		playerAttr.actionQueueEnd = (playerAttr.actionQueueEnd + 1) % ACTION_QUEUE_SIZE;

		// キューのオーバーフローを防止
		if (playerAttr.actionQueueEnd == playerAttr.actionQueueStart)
		{
			playerAttr.actionQueueStart = (playerAttr.actionQueueStart + 1) % ACTION_QUEUE_SIZE; // 最も古いアクションを破棄
		}
	}

	if (GetKeyboardTrigger(DIK_SPACE))
	{
		attributes.isJumping = true;
	}

	// 移動させちゃう
	if (!attributes.isAttacking && !attributes.isAttacking2 && !attributes.isAttacking3 && !attributes.isHit1)
	{
		if (GetKeyboardPress(DIK_A))
		{	// 左へ移動
			attributes.spd = VALUE_MOVE;

			attributes.isMoving = true;

			if (GetKeyboardPress(DIK_W))
				attributes.targetDir = -XM_PI * 3 / 4 + camera.GetRotation().y;
			else if (GetKeyboardPress(DIK_S))
				attributes.targetDir = -XM_PI / 4 + camera.GetRotation().y;
			else
				attributes.targetDir = -XM_PI / 2 + camera.GetRotation().y;
		}
		if (GetKeyboardPress(DIK_D))
		{	// 右へ移動
			attributes.spd = VALUE_MOVE;

			attributes.isMoving = true;
			if (GetKeyboardPress(DIK_W))
				attributes.targetDir = XM_PI * 3 / 4 + camera.GetRotation().y;
			else if (GetKeyboardPress(DIK_S))
				attributes.targetDir = XM_PI / 4 + camera.GetRotation().y;
			else
				attributes.targetDir = XM_PI / 2 + camera.GetRotation().y;
		}
		if (GetKeyboardPress(DIK_S))
		{	// 下へ移動
			attributes.spd = VALUE_MOVE;

			attributes.isMoving = true;
			if (GetKeyboardPress(DIK_A))
				attributes.targetDir = -XM_PI * 3 / 4 + camera.GetRotation().y;
			else if (GetKeyboardPress(DIK_D))
				attributes.targetDir = -XM_PI * 5 / 4 + camera.GetRotation().y;
			else
				attributes.targetDir = -XM_PI + camera.GetRotation().y;
		}
		if (GetKeyboardPress(DIK_W))
		{	// 上へ移動
			attributes.spd = VALUE_MOVE;

			attributes.isMoving = true;
			if (GetKeyboardPress(DIK_A))
				attributes.targetDir = -XM_PI / 4 + camera.GetRotation().y;
			else if (GetKeyboardPress(DIK_D))
				attributes.targetDir = XM_PI / 4 + camera.GetRotation().y;
			else
				attributes.targetDir = 0.0f + camera.GetRotation().y;
		}

	}
	HandlePlayerMove(transform);

	if (attributes.isHit1 || attributes.hitTimer > 0)
	{
		attributes.hitTimer -= playerGO->timer.GetScaledDeltaTime();
		if (attributes.hitTimer < 0)
		{
			attributes.isHit1 = false;
		}
	}

	if (attributes.isGrounded == false)
	{
		//transform.pos.y -= FALLING_SPEED * playerGO->timer.GetScaledDeltaTime();
	}

	playerGO->SetTransform(transform);

	attributes.spd *= pow(SPD_DECAY_RATE,  playerGO->timer.GetScaledDeltaTime());

	playerGO->UpdateAttributes(attributes);

	playerGO->Update();

	transform = playerGO->GetTransform();
	// プレイヤー視点
	XMFLOAT3 pos = transform.pos;
	pos.y += 70.0f;
	Camera::get_instance().SetCameraAT(pos);
	Camera::get_instance().SetCamera();

#ifdef _DEBUG
	debugProc.PrintDebugProc("PosX: %f PosY: %f, PosZ: %f\n", transform.pos.x, transform.pos.y, transform.pos.z);
#endif
}

void Player::Draw(void)
{
	playerGO->Draw();
}

void Player::DrawEffect(void)
{
	playerGO->DrawEffect();
}

void Player::HandlePlayerMove(Transform& transform)
{
	float deltaDir = attributes.targetDir - attributes.dir;
	if (deltaDir > XM_PI) deltaDir -= XM_2PI;
	if (deltaDir < -XM_PI) deltaDir += XM_2PI;
	attributes.dir += deltaDir * ROTATION_SPEED * playerGO->timer.GetScaledDeltaTime();

	transform.rot.y = attributes.dir;

	if (attributes.isMoveBlocked)
		return;

	// 入力のあった方向へプレイヤーを向かせて移動させる
	transform.pos.x += sinf(transform.rot.y) * attributes.spd * playerGO->timer.GetScaledDeltaTime();
	transform.pos.z += cosf(transform.rot.y) * attributes.spd * playerGO->timer.GetScaledDeltaTime();

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_TAB))
	{
		transform.pos.x += sinf(transform.rot.y) * attributes.spd * 5 * playerGO->timer.GetScaledDeltaTime();
		transform.pos.z += cosf(transform.rot.y) * attributes.spd * 5 * playerGO->timer.GetScaledDeltaTime();
	}
#endif // DEBUG

}

Transform Player::GetTransform(void) const
{
	return playerGO->GetTransform();
}

void Player::UpdateActionQueue(void)
{
	playerAttr.actionQueueClearTime++;
	for (UINT i = playerAttr.actionQueueStart; i < playerAttr.actionQueueEnd; i++)
	{
		playerAttr.actionQueue[i].liveTime += playerGO->timer.GetScaledDeltaTime();
		if (playerAttr.actionQueue[i].liveTime >= ACTION_QUEUE_CLEAR_WAIT)
		{
			playerAttr.actionQueueStart++;
			continue;
		}

		if (dynamic_cast<ISkinnedMeshModelChar*>(playerGO)->ExecuteAction(playerAttr.actionQueue[i].actionType))
		{
			playerAttr.actionQueueStart++;
		}
		else
		{
			break;
		}
	}
	if (playerAttr.actionQueueStart >= playerAttr.actionQueueEnd)
	{
		playerAttr.actionQueueStart = playerAttr.actionQueueEnd = 0;
	}
}
