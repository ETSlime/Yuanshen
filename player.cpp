//=============================================================================
//
// player���� [Player.cpp]
// Author : 
//
//=============================================================================

#include "main.h"
#include "renderer.h"
#include "Player.h"
#include "input.h"
#include "camera.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	VALUE_MOVE			(2.0f)							// �ړ���
#define VALUE_RUN			(4.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// ��]��
#define ROTATION_SPEED				(0.18f)

//=============================================================================
// ����������
//=============================================================================
Player::Player()
{
	sigewinne = new Sigewinne();

	playerAttribute.spd = 0;
	playerAttribute.stopRun = TRUE;
	playerAttribute.state = PlayerState::IDLE;

	playerGO = sigewinne;
}

Player::~Player()
{
	delete sigewinne;
}

void Player::Update(void)
{
	if (ISkinnedMeshModel* model = dynamic_cast<ISkinnedMeshModel*>(playerGO))
	{
		// �ړ��������Ⴄ
		if (GetKeyboardPress(DIK_A))
		{	// ���ֈړ�
			playerAttribute.spd = VALUE_MOVE;
			playerAttribute.stopRun = FALSE;

			if (model->GetStateMachine()->GetCurrentState() == PlayerState::IDLE)
				model->GetStateMachine()->SetCurrentState(PlayerState::WALK);
			if (GetKeyboardPress(DIK_LSHIFT) &&
				(model->GetStateMachine()->GetCurrentState() == PlayerState::WALK
					|| model->GetStateMachine()->GetCurrentState() == PlayerState::RUN))
			{
				playerAttribute.spd = VALUE_RUN;
				model->GetStateMachine()->SetCurrentState(PlayerState::RUN);
			}
			if (GetKeyboardPress(DIK_W))
				playerAttribute.targetDir = -XM_PI * 3 / 4;
			else if (GetKeyboardPress(DIK_S))
				playerAttribute.targetDir = -XM_PI / 4;
			else
				playerAttribute.targetDir = -XM_PI / 2;
		}
		if (GetKeyboardPress(DIK_D))
		{	// �E�ֈړ�
			playerAttribute.spd = VALUE_MOVE;
			playerAttribute.stopRun = FALSE;
			if (model->GetStateMachine()->GetCurrentState() == PlayerState::IDLE)
				model->GetStateMachine()->SetCurrentState(PlayerState::WALK);
			if (GetKeyboardPress(DIK_LSHIFT) && 
				(model->GetStateMachine()->GetCurrentState() == PlayerState::WALK
					|| model->GetStateMachine()->GetCurrentState() == PlayerState::RUN))
			{
				playerAttribute.spd = VALUE_RUN;
				model->GetStateMachine()->SetCurrentState(PlayerState::RUN);
			}
			if (GetKeyboardPress(DIK_W))
				playerAttribute.targetDir = XM_PI * 3 / 4;
			else if (GetKeyboardPress(DIK_S))
				playerAttribute.targetDir = XM_PI / 4;
			else
				playerAttribute.targetDir = XM_PI / 2;
		}
		if (GetKeyboardPress(DIK_S))
		{	// ���ֈړ�
			playerAttribute.spd = VALUE_MOVE;
			playerAttribute.stopRun = FALSE;
			if (model->GetStateMachine()->GetCurrentState() == PlayerState::IDLE)
				model->GetStateMachine()->SetCurrentState(PlayerState::WALK);
			if (GetKeyboardPress(DIK_LSHIFT) && 
				(model->GetStateMachine()->GetCurrentState() == PlayerState::WALK
					|| model->GetStateMachine()->GetCurrentState() == PlayerState::RUN))
			{
				playerAttribute.spd = VALUE_RUN;
				model->GetStateMachine()->SetCurrentState(PlayerState::RUN);
			}
			if (GetKeyboardPress(DIK_A))
				playerAttribute.targetDir = -XM_PI * 3 / 4;
			else if (GetKeyboardPress(DIK_D))
				playerAttribute.targetDir = -XM_PI * 5 / 4;
			else
				playerAttribute.targetDir = -XM_PI;
		}
		if (GetKeyboardPress(DIK_W))
		{	// ��ֈړ�
			playerAttribute.spd = VALUE_MOVE;
			playerAttribute.stopRun = FALSE;
			if (model->GetStateMachine()->GetCurrentState() == PlayerState::IDLE)
				model->GetStateMachine()->SetCurrentState(PlayerState::WALK);
			if (GetKeyboardPress(DIK_LSHIFT) && 
				(model->GetStateMachine()->GetCurrentState() == PlayerState::WALK
					|| model->GetStateMachine()->GetCurrentState() == PlayerState::RUN))
			{
				playerAttribute.spd = VALUE_RUN;
				model->GetStateMachine()->SetCurrentState(PlayerState::RUN);
			}
			if (GetKeyboardPress(DIK_A))
				playerAttribute.targetDir = -XM_PI / 4;
			else if (GetKeyboardPress(DIK_D))
				playerAttribute.targetDir = XM_PI / 4;
			else
				playerAttribute.targetDir = 0.0f;
		}

		if (CheckIdle())
			model->GetStateMachine()->SetCurrentState(PlayerState::IDLE);
	}


	float deltaDir = playerAttribute.targetDir - playerAttribute.dir;
	if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
	if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
	playerAttribute.dir += deltaDir * ROTATION_SPEED;

	HandlePlayerMove();

	if (ISkinnedMeshModel* model = dynamic_cast<ISkinnedMeshModel*>(playerGO))
	{
		// Key���͂���������ړ���������
		switch (model->GetStateMachine()->GetCurrentState())
		{
		case PlayerState::IDLE:
			model->PlayIdleAnim();
			model->GetStateMachine()->SetCurrentState(PlayerState::IDLE);
			break;
		case PlayerState::WALK:
			model->PlayWalkAnim();
			model->GetStateMachine()->SetCurrentState(PlayerState::WALK);
			break;
		case PlayerState::RUN:
			model->PlayRunAnim();
			model->GetStateMachine()->SetCurrentState(PlayerState::RUN);

			//if (playerAttribute.spd >= VALUE_MOVE * SPD_DECAY_RATE * SPD_DECAY_RATE * SPD_DECAY_RATE)
			//{
			//	if (GetKeyboardRelease(DIK_LSHIFT) && playerAttribute.spd >= VALUE_MOVE)
			//	{
			//		playerAttribute.stopRun = TRUE;
			//	}
			//	else if (playerAttribute.stopRun == FALSE)
			//		HandlePlayerMove();
			//}
			//else if (playerAttribute.stopRun == TRUE)
			//{
			//	PlayStopRunAnim();
			//}
			//else
			//{
			//	HandleStopMove();
			//}
			break;
		case PlayerState::JUMP:
			HandlePlayerMove();
			break;
		case PlayerState::ATTACK:
			//PlayAttackAnim();
			break;
		default:
			break;
		}

		//model->GetStateMachine()->Update(0.01f);
	}


	playerAttribute.spd *= 0.93f;

	playerGO->Update();
}

void Player::Draw(void)
{
	playerGO->Draw();
}

void Player::HandlePlayerMove()
{

	CAMERA* cam = GetCamera();

	Transform transform = playerGO->GetTransform();

	transform.rot.y = playerAttribute.dir + cam->rot.y;

	// ���͂̂����������փv���C���[���������Ĉړ�������
	transform.pos.x += sinf(transform.rot.y) * playerAttribute.spd;
	transform.pos.z += cosf(transform.rot.y) * playerAttribute.spd;

	playerGO->SetTransform(transform);
}

Transform Player::GetTransform(void)
{
	return playerGO->GetTransform();
}

BOOL Player::CheckIdle(void)
{
	if (ISkinnedMeshModel* model = dynamic_cast<ISkinnedMeshModel*>(playerGO))
	{
		if (model->GetStateMachine()->GetCurrentState() == PlayerState::IDLE)
			return false;
	}

	if (!GetKeyboardPress(DIK_W)
		&& !GetKeyboardPress(DIK_S)
		&& !GetKeyboardPress(DIK_A)
		&& !GetKeyboardPress(DIK_D))
		return true;
	else
		return false;
}
