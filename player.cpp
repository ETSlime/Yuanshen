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
	sigewinne->SetCurrentAnim(AnimationClipName::ANIM_WALK);

	playerAttribute.spd = 0;
	playerAttribute.stopRun = TRUE;
	playerAttribute.state = PlayerState::IDLE;
}

Player::~Player()
{
}

void Player::Update(void)
{
	// �ړ��������Ⴄ
	if (GetKeyboardPress(DIK_A))
	{	// ���ֈړ�
		playerAttribute.spd = VALUE_MOVE;
		playerAttribute.stopRun = FALSE;
		if (playerAttribute.state == PlayerState::IDLE)
			playerAttribute.state = PlayerState::WALK;
		if (GetKeyboardPress(DIK_LSHIFT) && 
			(playerAttribute.state == PlayerState::WALK || playerAttribute.state == PlayerState::RUN))
		{
			playerAttribute.spd = VALUE_RUN;
			playerAttribute.state = RUN;
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
		if (playerAttribute.state == IDLE)
			playerAttribute.state = WALK;
		if (GetKeyboardPress(DIK_LSHIFT) && (playerAttribute.state == WALK || playerAttribute.state == RUN))
		{
			playerAttribute.spd = VALUE_RUN;
			playerAttribute.state = RUN;
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
		if (playerAttribute.state == IDLE)
			playerAttribute.state = WALK;
		if (GetKeyboardPress(DIK_LSHIFT) && (playerAttribute.state == WALK || playerAttribute.state == RUN))
		{
			playerAttribute.spd = VALUE_RUN;
			playerAttribute.state = RUN;
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
		if (playerAttribute.state == IDLE)
			playerAttribute.state = WALK;
		if (GetKeyboardPress(DIK_LSHIFT) && (playerAttribute.state == WALK || playerAttribute.state == RUN))
		{
			playerAttribute.spd = VALUE_RUN;
			playerAttribute.state = RUN;
		}
		if (GetKeyboardPress(DIK_A))
			playerAttribute.targetDir = -XM_PI / 4;
		else if (GetKeyboardPress(DIK_D))
			playerAttribute.targetDir = XM_PI / 4;
		else
			playerAttribute.targetDir = 0.0f;
	}

	if (CheckIdle())
		playerAttribute.state = PlayerState::IDLE;

	float deltaDir = playerAttribute.targetDir - playerAttribute.dir;
	if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
	if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
	playerAttribute.dir += deltaDir * ROTATION_SPEED;

	HandlePlayerMove();

	// Key���͂���������ړ���������
	switch (playerAttribute.state)
	{
	case PlayerState::IDLE:
		sigewinne->PlayIdleAnim();
		break;
	case PlayerState::WALK:
		sigewinne->PlayMoveAnim();
	case PlayerState::RUN:
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

	playerAttribute.spd *= 0.93f;

	sigewinne->Update();
}

void Player::Draw(void)
{
	sigewinne->Draw();
}

void Player::HandlePlayerMove()
{

	CAMERA* cam = GetCamera();

	Transform transform = sigewinne->GetTransform();

	transform.rot.y = playerAttribute.dir + cam->rot.y;

	// ���͂̂����������փv���C���[���������Ĉړ�������
	transform.pos.x += sinf(transform.rot.y) * playerAttribute.spd;
	transform.pos.z += cosf(transform.rot.y) * playerAttribute.spd;

	sigewinne->SetTransform(transform);

	//if (playerAttribute.state == WALK || playerAttribute.state == RUN)
	//{
	//	sigewinne->PlayMoveAnim();
	//}
	//else if (playerAttribute.state == JUMP)
	//{
	//	sigewinne->PlayJumpAnim();
	//}
}

Transform Player::GetTransform(void)
{
	return sigewinne->GetTransform();
}

BOOL Player::CheckIdle(void)
{
	if (!GetKeyboardPress(DIK_W)
		&& !GetKeyboardPress(DIK_S)
		&& !GetKeyboardPress(DIK_A)
		&& !GetKeyboardPress(DIK_D))
		return true;
	else
		return false;
}
