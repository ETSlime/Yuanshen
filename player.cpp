//=============================================================================
//
// player処理 [Player.cpp]
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
// マクロ定義
//*****************************************************************************
#define	VALUE_MOVE			(2.0f)							// 移動量
#define VALUE_RUN			(4.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量
#define ROTATION_SPEED				(0.18f)

//=============================================================================
// 初期化処理
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

	// 移動させちゃう
	if (GetKeyboardPress(DIK_A))
	{	// 左へ移動
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
	{	// 右へ移動
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
	{	// 下へ移動
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
	{	// 上へ移動
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

	// 入力のあった方向へプレイヤーを向かせて移動させる
	transform.pos.x += sinf(transform.rot.y) * playerAttr.spd;
	transform.pos.z += cosf(transform.rot.y) * playerAttr.spd;

	playerGO->SetTransform(transform);
}

Transform Player::GetTransform(void)
{
	return playerGO->GetTransform();
}
