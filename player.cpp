//=============================================================================
//
// player���� [Player.cpp]
// Author : 
//
//=============================================================================

#include "Renderer.h"
#include "Player.h"
#include "InputManager.h"
#include "Camera.h"
#include "Debugproc.h"
#include "OctreeNode.h"
#include "CursorManager.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	VALUE_MOVE			(4.5f)							// �ړ���
#define VALUE_RUN			(120.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// ��]��
#define ROTATION_SPEED				(0.18f)
#define FALLING_SPEED		(2.50f)
#define SPD_DECAY_RATE		(0.93f)

#define PLAYER_INIT_POS		XMFLOAT3(12160.0f, -2110.0f, -20320.0f)
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
	m_playerGO = lumine;
	//m_playerGO->SetDrawWorldAABB(true);

	m_light = new DirectionalLight();
	m_light->SetEnable(true);
	m_light->BindToTransform(m_playerGO->GetTransformP());
	m_lightMgr.AddLight(m_light);
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

	m_attributes = m_playerGO->GetAttributes();
	Transform transform = m_playerGO->GetTransform();

	if (lumine && m_inputManager.GetKeyboardTrigger(DIK_1))
	{
		lumine->SetTransform(transform);
		//lumine->SetRenderProgress(0.0f);
		//lumine->SetSwitchCharEffect(true);
		m_playerGO = lumine;
		m_light->BindToTransform(m_playerGO->GetTransformP());
	}
	if (klee && m_inputManager.GetKeyboardTrigger(DIK_2))
	{
		klee->SetTransform(transform);
		//klee->SetRenderProgress(0.0f);
		//klee->SetSwitchCharEffect(true);
		m_playerGO = klee;
		m_light->BindToTransform(m_playerGO->GetTransformP());
	}
	if (sigewinne && m_inputManager.GetKeyboardTrigger(DIK_3))
	{
		sigewinne->SetTransform(transform);
		//sigewinne->SetRenderProgress(0.0f);
		//sigewinne->SetSwitchCharEffect(true);
		m_playerGO = sigewinne;
		m_light->BindToTransform(m_playerGO->GetTransformP());
	}

	Camera& camera = Camera::get_instance();

#ifdef _DEBUG
	if (m_inputManager.GetKeyboardPress(DIK_UP))
	{
		transform.pos.y += 50;
	}
	if (m_inputManager.GetKeyboardPress(DIK_DOWN))
	{
		transform.pos.y -= 50;
	}
#endif

	if (m_inputManager.IsMouseLeftTriggered() && !CursorManager::get_instance().IsMouseFreeMode())
	{
		PlayerAction action;
		action.actionType = ActionEnum::ATTACK;
		action.liveTime = 0;
		m_playerAttr.actionQueue[m_playerAttr.actionQueueEnd] = action;
		m_playerAttr.actionQueueEnd = (m_playerAttr.actionQueueEnd + 1) % ACTION_QUEUE_SIZE;

		// �L���[�̃I�[�o�[�t���[��h�~
		if (m_playerAttr.actionQueueEnd == m_playerAttr.actionQueueStart)
		{
			m_playerAttr.actionQueueStart = (m_playerAttr.actionQueueStart + 1) % ACTION_QUEUE_SIZE; // �ł��Â��A�N�V������j��
		}
	}

	if (m_inputManager.GetKeyboardTrigger(DIK_SPACE))
	{
		m_attributes.isJumping = true;
	}

	// �ړ��������Ⴄ
	if (!m_attributes.isAttacking && !m_attributes.isAttacking2 && !m_attributes.isAttacking3 && !m_attributes.isHit1)
	{
		if (m_inputManager.GetKeyboardPress(DIK_A))
		{	// ���ֈړ�
			m_attributes.spd = VALUE_MOVE;

			m_attributes.isMoving = true;

			if (m_inputManager.GetKeyboardPress(DIK_W))
				m_attributes.targetDir = -XM_PI * 3 / 4 + camera.GetRotation().y;
			else if (m_inputManager.GetKeyboardPress(DIK_S))
				m_attributes.targetDir = -XM_PI / 4 + camera.GetRotation().y;
			else
				m_attributes.targetDir = -XM_PI / 2 + camera.GetRotation().y;
		}
		if (m_inputManager.GetKeyboardPress(DIK_D))
		{	// �E�ֈړ�
			m_attributes.spd = VALUE_MOVE;

			m_attributes.isMoving = true;
			if (m_inputManager.GetKeyboardPress(DIK_W))
				m_attributes.targetDir = XM_PI * 3 / 4 + camera.GetRotation().y;
			else if (m_inputManager.GetKeyboardPress(DIK_S))
				m_attributes.targetDir = XM_PI / 4 + camera.GetRotation().y;
			else
				m_attributes.targetDir = XM_PI / 2 + camera.GetRotation().y;
		}
		if (m_inputManager.GetKeyboardPress(DIK_S))
		{	// ���ֈړ�
			m_attributes.spd = VALUE_MOVE;

			m_attributes.isMoving = true;
			if (m_inputManager.GetKeyboardPress(DIK_A))
				m_attributes.targetDir = -XM_PI * 3 / 4 + camera.GetRotation().y;
			else if (m_inputManager.GetKeyboardPress(DIK_D))
				m_attributes.targetDir = -XM_PI * 5 / 4 + camera.GetRotation().y;
			else
				m_attributes.targetDir = -XM_PI + camera.GetRotation().y;
		}
		if (m_inputManager.GetKeyboardPress(DIK_W))
		{	// ��ֈړ�
			m_attributes.spd = VALUE_MOVE;

			m_attributes.isMoving = true;
			if (m_inputManager.GetKeyboardPress(DIK_A))
				m_attributes.targetDir = -XM_PI / 4 + camera.GetRotation().y;
			else if (m_inputManager.GetKeyboardPress(DIK_D))
				m_attributes.targetDir = XM_PI / 4 + camera.GetRotation().y;
			else
				m_attributes.targetDir = 0.0f + camera.GetRotation().y;
		}

	}
	HandlePlayerMove(transform);

	if (m_attributes.isHit1 || m_attributes.hitTimer > 0)
	{
		m_attributes.hitTimer -= m_playerGO->timer.GetScaledDeltaTime();
		if (m_attributes.hitTimer < 0)
		{
			m_attributes.isHit1 = false;
		}
	}

	if (m_attributes.isGrounded == false)
	{
		transform.pos.y -= FALLING_SPEED * m_playerGO->timer.GetScaledDeltaTime();
	}

	m_playerGO->SetTransform(transform);

	m_attributes.spd *= pow(SPD_DECAY_RATE,  m_playerGO->timer.GetScaledDeltaTime());

	m_playerGO->UpdateAttributes(m_attributes);

	m_playerGO->Update();

	transform = m_playerGO->GetTransform();
	// �v���C���[���_
	XMFLOAT3 pos = transform.pos;
	pos.y += 70.0f;
	Camera::get_instance().SetCameraAT(pos);
	Camera::get_instance().SetCamera();

#ifdef _DEBUG
	m_debugProc.PrintDebugProc("PosX: %f PosY: %f, PosZ: %f\n", transform.pos.x, transform.pos.y, transform.pos.z);
#endif
}

void Player::Draw(void)
{
	m_playerGO->Draw();
}

void Player::DrawEffect(void)
{
	m_playerGO->DrawEffect();
}

void Player::HandlePlayerMove(Transform& transform)
{
	float deltaDir = m_attributes.targetDir - m_attributes.dir;
	if (deltaDir > XM_PI) deltaDir -= XM_2PI;
	if (deltaDir < -XM_PI) deltaDir += XM_2PI;
	m_attributes.dir += deltaDir * ROTATION_SPEED * m_playerGO->timer.GetScaledDeltaTime();

	transform.rot.y = m_attributes.dir;

	if (m_attributes.isMoveBlocked)
		return;

	// ���͂̂����������փv���C���[���������Ĉړ�������
	transform.pos.x += sinf(transform.rot.y) * m_attributes.spd * m_playerGO->timer.GetScaledDeltaTime();
	transform.pos.z += cosf(transform.rot.y) * m_attributes.spd * m_playerGO->timer.GetScaledDeltaTime();

#ifdef _DEBUG
	if (m_inputManager.GetKeyboardPress(DIK_TAB))
	{
		transform.pos.x += sinf(transform.rot.y) * m_attributes.spd * 5 * m_playerGO->timer.GetScaledDeltaTime();
		transform.pos.z += cosf(transform.rot.y) * m_attributes.spd * 5 * m_playerGO->timer.GetScaledDeltaTime();
	}
#endif // DEBUG

}

Transform Player::GetTransform(void) const
{
	return m_playerGO->GetTransform();
}

void Player::UpdateActionQueue(void)
{
	m_playerAttr.actionQueueClearTime++;
	for (UINT i = m_playerAttr.actionQueueStart; i < m_playerAttr.actionQueueEnd; i++)
	{
		m_playerAttr.actionQueue[i].liveTime += m_playerGO->timer.GetScaledDeltaTime();
		if (m_playerAttr.actionQueue[i].liveTime >= ACTION_QUEUE_CLEAR_WAIT)
		{
			m_playerAttr.actionQueueStart++;
			continue;
		}

		if (dynamic_cast<ISkinnedMeshModelChar*>(m_playerGO)->ExecuteAction(m_playerAttr.actionQueue[i].actionType))
		{
			m_playerAttr.actionQueueStart++;
		}
		else
		{
			break;
		}
	}
	if (m_playerAttr.actionQueueStart >= m_playerAttr.actionQueueEnd)
	{
		m_playerAttr.actionQueueStart = m_playerAttr.actionQueueEnd = 0;
	}
}
