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
#include "debugproc.h"
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

	playerGO = sigewinne;
}

Player::~Player()
{
	delete sigewinne;
}

void Player::Update(void)
{
	playerAttr = playerGO->GetAttributes();

	// �ړ��������Ⴄ
	if (GetKeyboardPress(DIK_A))
	{	// ���ֈړ�
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;

		if (GetKeyboardPress(DIK_W))
			playerAttr.targetDir = -XM_PI * 3 / 4;
		else if (GetKeyboardPress(DIK_S))
			playerAttr.targetDir = -XM_PI / 4;
		else
			playerAttr.targetDir = -XM_PI / 2;
	}
	if (GetKeyboardPress(DIK_D))
	{	// �E�ֈړ�
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;
		if (GetKeyboardPress(DIK_W))
			playerAttr.targetDir = XM_PI * 3 / 4;
		else if (GetKeyboardPress(DIK_S))
			playerAttr.targetDir = XM_PI / 4;
		else
			playerAttr.targetDir = XM_PI / 2;
	}
	if (GetKeyboardPress(DIK_S))
	{	// ���ֈړ�
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;
		if (GetKeyboardPress(DIK_A))
			playerAttr.targetDir = -XM_PI * 3 / 4;
		else if (GetKeyboardPress(DIK_D))
			playerAttr.targetDir = -XM_PI * 5 / 4;
		else
			playerAttr.targetDir = -XM_PI;
	}
	if (GetKeyboardPress(DIK_W))
	{	// ��ֈړ�
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;
		if (GetKeyboardPress(DIK_A))
			playerAttr.targetDir = -XM_PI / 4;
		else if (GetKeyboardPress(DIK_D))
			playerAttr.targetDir = XM_PI / 4;
		else
			playerAttr.targetDir = 0.0f;
	}

	if (!playerAttr.isAttacking)
		HandlePlayerMove();


	playerAttr.spd *= 0.93f;

	playerGO->UpdateAttributes(playerAttr);

	playerGO->Update();
}

void Player::Draw(void)
{
	playerGO->Draw();
}

void Player::HandlePlayerMove()
{

	CAMERA* cam = GetCamera();

	float deltaDir = playerAttr.targetDir - playerAttr.dir;
	if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
	if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
	playerAttr.dir += deltaDir * ROTATION_SPEED;

	Transform transform = playerGO->GetTransform();

	transform.rot.y = playerAttr.dir + cam->rot.y;

	// ���͂̂����������փv���C���[���������Ĉړ�������
	transform.pos.x += sinf(transform.rot.y) * playerAttr.spd;
	transform.pos.z += cosf(transform.rot.y) * playerAttr.spd;

	playerGO->SetTransform(transform);
}

Transform Player::GetTransform(void)
{
	return playerGO->GetTransform();
}
