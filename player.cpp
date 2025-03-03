//=============================================================================
//
// player���� [Player.cpp]
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
// �}�N����`
//*****************************************************************************
#define	VALUE_MOVE			(6.5f)							// �ړ���
#define VALUE_RUN			(120.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// ��]��
#define ROTATION_SPEED				(0.18f)
#define FALLING_SPEED		(25.0f)

#define PLAYER_INIT_POS		XMFLOAT3(12582.0f, -2424.0f, -19485.0f)
//=============================================================================
// ����������
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
		playerGO = lumine;
	}
	if (klee && GetKeyboardTrigger(DIK_2))
	{
		klee->SetTransform(transform);
		playerGO = klee;
	}
	if (sigewinne && GetKeyboardTrigger(DIK_3))
	{
		sigewinne->SetTransform(transform);
		playerGO = sigewinne;
	}

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

	if (IsMouseLeftTriggered())
	{
		PlayerAction action;
		action.actionType = ActionEnum::ATTACK;
		action.liveTime = 0;
		playerAttr.actionQueue[playerAttr.actionQueueEnd] = action;
		playerAttr.actionQueueEnd = (playerAttr.actionQueueEnd + 1) % ACTION_QUEUE_SIZE;

		// �L���[�̃I�[�o�[�t���[��h�~
		if (playerAttr.actionQueueEnd == playerAttr.actionQueueStart)
		{
			playerAttr.actionQueueStart = (playerAttr.actionQueueStart + 1) % ACTION_QUEUE_SIZE; // �ł��Â��A�N�V������j��
		}
	}

	if (GetKeyboardTrigger(DIK_SPACE))
	{
		attributes.isJumping = true;
	}

	// �ړ��������Ⴄ
	if (GetKeyboardPress(DIK_A))
	{	// ���ֈړ�
		attributes.spd = VALUE_MOVE;

		attributes.isMoving = true;

		if (GetKeyboardPress(DIK_W))
			attributes.targetDir = -XM_PI * 3 / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_S))
			attributes.targetDir = -XM_PI / 4 + camera->rot.y;
		else
			attributes.targetDir = -XM_PI / 2 + camera->rot.y;
	}
	if (GetKeyboardPress(DIK_D))
	{	// �E�ֈړ�
		attributes.spd = VALUE_MOVE;

		attributes.isMoving = true;
		if (GetKeyboardPress(DIK_W))
			attributes.targetDir = XM_PI * 3 / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_S))
			attributes.targetDir = XM_PI / 4 + camera->rot.y;
		else
			attributes.targetDir = XM_PI / 2 + camera->rot.y;
	}
	if (GetKeyboardPress(DIK_S))
	{	// ���ֈړ�
		attributes.spd = VALUE_MOVE;

		attributes.isMoving = true;
		if (GetKeyboardPress(DIK_A))
			attributes.targetDir = -XM_PI * 3 / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_D))
			attributes.targetDir = -XM_PI * 5 / 4 + camera->rot.y;
		else
			attributes.targetDir = -XM_PI + camera->rot.y;
	}
	if (GetKeyboardPress(DIK_W))
	{	// ��ֈړ�
		attributes.spd = VALUE_MOVE;

		attributes.isMoving = true;
		if (GetKeyboardPress(DIK_A))
			attributes.targetDir = -XM_PI / 4 + camera->rot.y;
		else if (GetKeyboardPress(DIK_D))
			attributes.targetDir = XM_PI / 4 + camera->rot.y;
		else
			attributes.targetDir = 0.0f + camera->rot.y;
	}

	if (!attributes.isAttacking && !attributes.isAttacking2 && !attributes.isAttacking3)
		HandlePlayerMove(transform);

	if (attributes.isGrounded == false)
	{
		transform.pos.y -= FALLING_SPEED;
	}

	playerGO->SetTransform(transform);

	attributes.spd *= 0.93f;

	playerGO->UpdateAttributes(attributes);

	playerGO->Update();

	transform = playerGO->GetTransform();
	// �v���C���[���_
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

	float deltaDir = attributes.targetDir - attributes.dir;
	if (deltaDir > XM_PI) deltaDir -= 2 * XM_PI;
	if (deltaDir < -XM_PI) deltaDir += 2 * XM_PI;
	attributes.dir += deltaDir * ROTATION_SPEED;

	transform.rot.y = attributes.dir;

	// ���͂̂����������փv���C���[���������Ĉړ�������
	transform.pos.x += sinf(transform.rot.y) * attributes.spd;
	transform.pos.z += cosf(transform.rot.y) * attributes.spd;

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_LCONTROL))
	{
		transform.pos.x += sinf(transform.rot.y) * attributes.spd * 5;
		transform.pos.z += cosf(transform.rot.y) * attributes.spd * 5;
	}
#endif // DEBUG

}

Transform Player::GetTransform(void)
{
	return playerGO->GetTransform();
}

void Player::UpdateActionQueue(void)
{
	playerAttr.actionQueueClearTime++;
	for (UINT i = playerAttr.actionQueueStart; i < playerAttr.actionQueueEnd; i++)
	{
		playerAttr.actionQueue[i].liveTime++;
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
