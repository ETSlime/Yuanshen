//=============================================================================
//
// player処理 [Player.cpp]
// Author : 
//
//=============================================================================

#include "renderer.h"
#include "Player.h"
#include "input.h"
#include "camera.h"
#include "debugproc.h"
#include "OctreeNode.h"
//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VALUE_MOVE			(12.0f)							// 移動量
#define VALUE_RUN			(120.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// 回転量
#define ROTATION_SPEED				(0.18f)
#define FALLING_SPEED		(25.0f)
//=============================================================================
// 初期化処理
//=============================================================================
Player::Player()
{
	sigewinne = new Sigewinne();

	//klee =  new Klee();

	//lumine = new Lumine();

	//hilichurl = new Hilichurl();

	//mitachurl = new Mitachurl();

	Transform transform;
	transform.pos = XMFLOAT3(-11400.0f, -3000.0f, -26000.0f);
	sigewinne->SetTransform(transform);
	playerGO = sigewinne;
}

Player::~Player()
{
	SAFE_DELETE(sigewinne);
	SAFE_DELETE(klee);
	SAFE_DELETE(lumine);
}

void Player::Update(void)
{
	if (lumine && GetKeyboardTrigger(DIK_1))
		playerGO = lumine;
	if (klee && GetKeyboardTrigger(DIK_2))
		playerGO = klee;
	if (sigewinne && GetKeyboardTrigger(DIK_3))
		playerGO = sigewinne;

	playerAttr = playerGO->GetAttributes();
	Transform transform = playerGO->GetTransform();
	CAMERA* camera = GetCamera();

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

	// 移動させちゃう
	if (GetKeyboardPress(DIK_A))
	{	// 左へ移動
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;

		if (GetKeyboardPress(DIK_W))
			playerAttr.targetDir = -XM_PI * 3 / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_S))
			playerAttr.targetDir = -XM_PI / 4 + camera->rot.y;
		else
			playerAttr.targetDir = -XM_PI / 2 + camera->rot.y;
	}
	if (GetKeyboardPress(DIK_D))
	{	// 右へ移動
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;
		if (GetKeyboardPress(DIK_W))
			playerAttr.targetDir = XM_PI * 3 / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_S))
			playerAttr.targetDir = XM_PI / 4 + camera->rot.y;
		else
			playerAttr.targetDir = XM_PI / 2 + camera->rot.y;
	}
	if (GetKeyboardPress(DIK_S))
	{	// 下へ移動
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;
		if (GetKeyboardPress(DIK_A))
			playerAttr.targetDir = -XM_PI * 3 / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_D))
			playerAttr.targetDir = -XM_PI * 5 / 4 + camera->rot.y;
		else
			playerAttr.targetDir = -XM_PI + camera->rot.y;
	}
	if (GetKeyboardPress(DIK_W))
	{	// 上へ移動
		playerAttr.spd = VALUE_MOVE;

		playerAttr.isMoving = true;
		if (GetKeyboardPress(DIK_A))
			playerAttr.targetDir = -XM_PI / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_D))
			playerAttr.targetDir = XM_PI / 4 + camera->rot.y;
		else
			playerAttr.targetDir = 0.0f + camera->rot.y;
	}

	if (!playerAttr.isAttacking)
		HandlePlayerMove(transform);

	if (playerAttr.isGrounded == false)
	{
		transform.pos.y -= FALLING_SPEED;
	}

	playerGO->SetTransform(transform);

	playerAttr.spd *= 0.93f;

	playerGO->UpdateAttributes(playerAttr);

	playerGO->Update();

	transform = playerGO->GetTransform();
	// プレイヤー視点
	XMFLOAT3 pos = transform.pos;
	pos.y += 70.0f;
	SetCameraAT(pos);
	SetCamera();

#ifdef _DEBUG
	PrintDebugProc("PosX: %f PosY: %f, PosZ: %f\n", transform.pos.x, transform.pos.y, transform.pos.z);
#endif
}

void Player::Draw(void)
{
	playerGO->Draw();
}

void Player::HandlePlayerMove(Transform& transform)
{

	CAMERA* cam = GetCamera();

	float deltaDir = playerAttr.targetDir - playerAttr.dir;
	if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
	if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
	playerAttr.dir += deltaDir * ROTATION_SPEED;

	transform.rot.y = playerAttr.dir;

	// 入力のあった方向へプレイヤーを向かせて移動させる
	transform.pos.x += sinf(transform.rot.y) * playerAttr.spd;
	transform.pos.z += cosf(transform.rot.y) * playerAttr.spd;

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_LCONTROL))
	{
		transform.pos.x += sinf(transform.rot.y) * playerAttr.spd * 5;
		transform.pos.z += cosf(transform.rot.y) * playerAttr.spd * 5;
	}
#endif // DEBUG

}

Transform Player::GetTransform(void)
{
	return playerGO->GetTransform();
}
