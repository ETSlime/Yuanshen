//=============================================================================
//
// �G�l�~�[���f������ [Enemy.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "Renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "Enemy.h"
#include "Camera.h"
#include "MapEditor.h"
#include "score.h"
#include "Hilichurl.h"
#include "Player.h"
#include "sprite.h"
#include "BehaviorTree.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************

#define	VALUE_MOVE			(3.5f)							// �ړ���
#define VALUE_RUN			(120.0f)
#define	VALUE_ROTATE		(XM_PI * 0.02f)					// ��]��

#define ENEMY_SHADOW_SIZE	(1.1f)						// �e�̑傫��
#define ENEMY_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��
#define ENEMY_SCALE			(1.8f)

#define	TEXTURE_MAX			(2)

#define ROTATION_SPEED		(0.09f)
#define FALLING_SPEED		(5.0f)
#define SPD_DECAY_RATE		(0.93f)

#define	HPGAUGE_PATH		"data/TEXTURE/EnemyHPGauge.png"
#define	HPGAUGE_COVER_PATH	"data/TEXTURE/EnemyHPGauge_bg.png"
#define HPGAUGE_WIDTH_SCL	(0.5f)
#define HPGAUGE_HEIGHT		(5.0f)
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************

//void Enemy::UpdateEditorSelect(int sx, int sy)
//{
//	if (MapEditor::get_instance().GetOnEditorCursor() == TRUE)
//	{
//		return;
//	}
//		
//	this->SetIsCursorIn(FALSE);
//
//	CAMERA* camera = GetCamera();
//	XMMATRIX P = XMLoadFloat4x4(&camera->mtxProjection);
//
//	// Compute picking ray in view space.
//	float vx = (+2.0f * sx / SCREEN_WIDTH - 1.0f) / P.r[0].m128_f32[0];
//	float vy = (-2.0f * sy / SCREEN_HEIGHT + 1.0f) / P.r[1].m128_f32[1];
//
//	// Ray definition in view space.
//	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
//	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);
//
//	// Tranform ray to local space of Mesh.
//	XMMATRIX V = XMLoadFloat4x4(&camera->mtxView);
//	XMMATRIX invView = XMLoadFloat4x4(&camera->mtxInvView);
//
//	XMMATRIX W = instance.transform.mtxWorld;
//	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
//
//	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);
//
//	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
//	rayDir = XMVector3TransformNormal(rayDir, toLocal);
//
//	// Make the ray direction unit length for the intersection tests.
//	rayDir = XMVector3Normalize(rayDir);
//	
//	XMVECTOR minPoint = XMLoadFloat3(&instance.pModel->GetBoundingBox().minPoint);
//	XMVECTOR maxPoint = XMLoadFloat3(&instance.pModel->GetBoundingBox().maxPoint);
//
//
//	float tMin = 0.0f;
//	float tMax = FLT_MAX;
//	BOOL intersect = FALSE;
//	for (int i = 0; i < 3; ++i) 
//	{
//		float rayOriginComponent = XMVectorGetByIndex(rayOrigin, i);
//		float rayDirComponent = XMVectorGetByIndex(rayDir, i);
//		float minComponent = XMVectorGetByIndex(minPoint, i);
//		float maxComponent = XMVectorGetByIndex(maxPoint, i);
//
//		if (fabs(rayDirComponent) < 1e-8f) {
//			if (rayOriginComponent < minComponent || rayOriginComponent > maxComponent) 
//			{
//				intersect =  FALSE;
//				return;
//			}
//		}
//		else 
//		{
//			float t1 = (minComponent - rayOriginComponent) / rayDirComponent;
//			float t2 = (maxComponent - rayOriginComponent) / rayDirComponent;
//
//			if (t1 > t2) 
//			{
//				float temp = t1;
//				t1 = t2;
//				t2 = temp;
//			}
//
//			tMin = max(tMin, t1);
//			tMax = min(tMax, t2);
//
//			if (tMin > tMax) 
//			{
//				intersect = FALSE;
//				return;
//			}
//		}
//	}
//
//	this->SetIsCursorIn(TRUE);
//
//
//	this->UpdateModelEditor();
//
//	PrintDebugProc("Enemy:X:%f Y:%f Z:%f\n", instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);
//
//}

//=============================================================================
// ����������
//=============================================================================
Enemy::Enemy(EnemyType enemyType, Transform trans)
{

	//MapEditor::get_instance().AddToList(this);
	enemyAttr.enemyType = enemyType;

	instance.load = true;
	behaviorTree = nullptr;
	player = nullptr;

	instance.attributes.spd = 0.0f;

	switch (enemyAttr.enemyType)
	{
	case EnemyType::Hilichurl:
		enemyAttr.maxHP = HILI_MAX_HP;
		enemyAttr.HP = HILI_MAX_HP;
		enemyAttr.viewAngle = HILI_VIEW_ANGLE;
		enemyAttr.viewDistance = HILI_VIEW_DISTANCE;
		enemyAttr.chaseRange = HILI_CHASING_RANGE;
		enemyAttr.attackRange = HILI_ATTACK_RANGE;

		behaviorTree = new BehaviorTree(this);
		break;
	default:
		break;
	}

	instance.transform.pos = trans.pos;
	instance.transform.rot = trans.rot;
	instance.transform.scl = trans.scl;
	enemyAttr.initTrans.pos = trans.pos;
	enemyAttr.initTrans.rot = trans.rot;
	enemyAttr.initTrans.scl = trans.scl;

	HPGaugeTex = TextureMgr::get_instance().CreateTexture(HPGAUGE_PATH);
	HPGaugeCoverTex = TextureMgr::get_instance().CreateTexture(HPGAUGE_COVER_PATH);

	instance.use = true;		// true:�����Ă�
}

Enemy::~Enemy()
{
	SafeRelease(&HPGaugeTex);
	SafeRelease(&HPGaugeCoverTex);
	SafeRelease(&HPGaugeVertexBuffer);
}

//=============================================================================
// �X�V����
//=============================================================================
void Enemy::Update(void)
{
	if (instance.use == true)		// ���̃G�l�~�[���g���Ă���H
	{								// Yes



		if (!UpdateAliveState())
			return;

		UpdateHPGauge();

		GameObject::Update();

		BOUNDING_BOX aabb = instance.pModel->GetBoundingBox();
		float height = aabb.maxPoint.y - aabb.minPoint.y;
		HPGauge.pos = instance.transform.pos;
		HPGauge.pos.y += height;

		enemyAttr.timer += timer.GetScaledDeltaTime();

		if (instance.attributes.isGrounded == false)
		{
			instance.transform.pos.y -= FALLING_SPEED * timer.GetScaledDeltaTime();
		}

		if (!CheckAvailableToMove())
			return;

		behaviorTree->RunBehaviorTree();

		float deltaDir = instance.attributes.targetDir - instance.attributes.dir;
		if (deltaDir > XM_PI) deltaDir -= XM_2PI;
		if (deltaDir < -XM_PI) deltaDir += XM_2PI;
		instance.attributes.dir += deltaDir * ROTATION_SPEED * timer.GetScaledDeltaTime();
		instance.transform.rot.y = instance.attributes.dir;

		if (instance.attributes.isMoveBlocked)
			return;

		if (enemyAttr.fixedDirMove)
		{
			// �p�x�̑������v�Z�i���x�Ɋ�Â���]�ʁj
			float deltaTheta = (instance.attributes.spd / enemyAttr.cooldownOrbitRadius)* enemyAttr.cooldownMoveDirection;

			XMVECTOR fixedDirVec = XMVectorReplicate(enemyAttr.fixedDir);
			XMVECTOR deltaThetaVec = XMVectorReplicate(deltaTheta);

			// �p�x��deltaTheta�����ω��������cos��sin���v�Z
			XMVECTOR cosNew = XMVectorCos(XMVectorAdd(fixedDirVec, deltaThetaVec));
			XMVECTOR cosOld = XMVectorCos(fixedDirVec);
			XMVECTOR sinNew = XMVectorSin(XMVectorAdd(fixedDirVec, deltaThetaVec));
			XMVECTOR sinOld = XMVectorSin(fixedDirVec);

			// X �������̑���
			XMVECTOR dxVec = XMVectorScale(XMVectorSubtract(sinNew, sinOld), enemyAttr.cooldownOrbitRadius);
			// Z �������̑���
			XMVECTOR dzVec = XMVectorScale(XMVectorSubtract(cosNew, cosOld), enemyAttr.cooldownOrbitRadius);

			float dx = XMVectorGetX(dxVec);
			float dz = XMVectorGetX(dzVec);

			instance.transform.pos.x += dx * timer.GetScaledDeltaTime();
			instance.transform.pos.z += dz * timer.GetScaledDeltaTime();

		}
		else
		{
			instance.transform.pos.x += sinf(instance.transform.rot.y) * instance.attributes.spd * timer.GetScaledDeltaTime();
			instance.transform.pos.z += cosf(instance.transform.rot.y) * instance.attributes.spd * timer.GetScaledDeltaTime();
		}


		instance.attributes.spd *= pow(SPD_DECAY_RATE, timer.GetScaledDeltaTime());

		{
			//UpdateEditorSelect(GetMousePosX(), GetMousePosY());
		}
	}
}

bool Enemy::UpdateAliveState(void)
{
	if (enemyAttr.isDead == true)
	{
		enemyAttr.respawnTimer -= timer.GetScaledDeltaTime();
		if (enemyAttr.respawnTimer <= 0.0f)
		{
			enemyAttr.respawnTimer = 0.0f;
			Initialize();
		}
		else
			return false;
	}

	if (instance.renderProgress.progress < 1.0f && !enemyAttr.startFadeOut)
	{
		instance.renderProgress.isRandomFade = true;
		instance.renderProgress.progress += 0.01f * timer.GetScaledDeltaTime();
		if (instance.renderProgress.progress > 1.0f)
		{
			instance.renderProgress.isRandomFade = false;
			instance.renderProgress.progress = 1.0f;
		}
	}
	else if (enemyAttr.startFadeOut == true)
	{
		instance.renderProgress.isRandomFade = true;
		instance.renderProgress.progress -= 0.01f * timer.GetScaledDeltaTime();
		if (instance.renderProgress.progress <= 0.0f)
		{
			instance.renderProgress.isRandomFade = false;
			instance.renderProgress.progress = 0.0f;
			enemyAttr.isDead = true;
			instance.collider.enable = false;
		}
		return false;
	}

	if (enemyAttr.HP <= 0.0f)
	{
		enemyAttr.respawnTimer = ENEMY_RESPAWN_TIME;
		enemyAttr.die = true;
		return false;
	}

	return true;
}

//=============================================================================
// �`�揈��
//=============================================================================
void Enemy::Draw(void)
{
	GameObject::Draw();
}

void Enemy::DrawUI(EnemyUIType type)
{
	switch (type)
	{
	case EnemyUIType::HPGauge:
		if (HPGauge.bUse)
			DrawHPGauge();
		break;
	case EnemyUIType::HPGaugeCover:
		if (HPGauge.bUse)
			DrawHPGaugeCover();
		break;
	default:
		break;
	}
}

void Enemy::DrawHPGauge(void)
{
	XMMATRIX mtxWorld;

	float ratio = enemyAttr.HP / enemyAttr.maxHP;

	// �����̃��[���h���W���v�Z
	XMMATRIX worldPosition = XMMatrixTranslation(HPGauge.pos.x, HPGauge.pos.y, HPGauge.pos.z);
	// ���[�����[�J�����W�̌��_�Ɉړ�
	XMMATRIX moveToLeft = XMMatrixTranslation(HPGauge.fWidth * 0.5f, 0.0f, 0.0f);
	// ���_����ɃX�P�[�����O
	XMMATRIX scaleMatrix = XMMatrixScaling(ratio, 1.0f, 1.0f);
	// ���[���Œ肷�邽�߂̕␳�ړ�
	XMMATRIX moveBack = XMMatrixTranslation(-HPGauge.fWidth * 0.5f, 0.0f, 0.0f);

	// ���[�����_�Ɉړ�
	mtxWorld = XMMatrixMultiply(moveToLeft, scaleMatrix);
	// ���[���Œ肷�邽�߂̕␳�ړ�
	mtxWorld = XMMatrixMultiply(mtxWorld, moveBack);
	// �r���{�[�h�̉�]��K�p
	mtxWorld = XMMatrixMultiply(mtxWorld, HPGauge.rot);
	// �ŏI�I�ȃ��[���h���W�Ɉړ�
	mtxWorld = XMMatrixMultiply(mtxWorld, worldPosition);

	// ���[���h�}�g���b�N�X�̐ݒ�
	Renderer::get_instance().SetCurrentWorldMatrix(&mtxWorld);

	HPGauge.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Renderer::get_instance().SetMaterial(HPGauge.material);

	// �v���~�e�B�u�g�|���W�ݒ�
	Renderer::get_instance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::get_instance().GetDeviceContext()->IASetVertexBuffers(0, 1, &HPGaugeVertexBuffer, &stride, &offset);

	Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &HPGaugeTex);

	// �|���S���̕`��
	Renderer::get_instance().GetDeviceContext()->Draw(4, 0);


}

void Enemy::DrawHPGaugeCover(void)
{
	XMMATRIX mtxTranslate, mtxWorld;

	mtxTranslate = XMMatrixTranslation(HPGauge.pos.x, HPGauge.pos.y, HPGauge.pos.z);
	mtxWorld = XMMatrixMultiply(HPGauge.rot, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	Renderer::get_instance().SetCurrentWorldMatrix(&mtxWorld);

	// �}�e���A���ݒ�
	HPGauge.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	Renderer::get_instance().SetMaterial(HPGauge.material);

	// �v���~�e�B�u�g�|���W�ݒ�
	Renderer::get_instance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::get_instance().GetDeviceContext()->IASetVertexBuffers(0, 1, &HPGaugeVertexBuffer, &stride, &offset);

	// �e�N�X�`���ݒ�
	Renderer::get_instance().GetDeviceContext()->PSSetShaderResources(0, 1, &HPGaugeCoverTex);

	// �|���S���̕`��
	Renderer::get_instance().GetDeviceContext()->Draw(4, 0);
}

void Enemy::InitHPGauge(void)
{
	BOUNDING_BOX aabb = instance.pModel->GetBoundingBox();
	float height = aabb.maxPoint.y - aabb.minPoint.y;
	float width = (aabb.maxPoint.x - aabb.minPoint.x) * HPGAUGE_WIDTH_SCL;

	HPGauge.pos = instance.transform.pos;
	HPGauge.pos.y += height;
	HPGauge.fWidth = width;
	HPGauge.fHeight = HPGAUGE_HEIGHT;
	MakeVertexHPGauge(HPGauge.fWidth, HPGauge.fHeight);

	HPGauge.bUse = true;
}

void Enemy::UpdateHPGauge(void)
{
	Camera& cam = Camera::get_instance();

	// �r���[�}�g���b�N�X���擾
	XMMATRIX mtxView = XMLoadFloat4x4(&cam.GetViewMatrix());

	// �����s��i�����s��j��]�u�s�񂳂��ċt�s�������Ă��(����)
	XMMATRIX billboardRotation = XMMatrixIdentity();
	billboardRotation.r[0] = XMVectorSet(mtxView.r[0].m128_f32[0], mtxView.r[1].m128_f32[0], mtxView.r[2].m128_f32[0], 0.0f);
	billboardRotation.r[1] = XMVectorSet(mtxView.r[0].m128_f32[1], mtxView.r[1].m128_f32[1], mtxView.r[2].m128_f32[1], 0.0f);
	billboardRotation.r[2] = XMVectorSet(mtxView.r[0].m128_f32[2], mtxView.r[1].m128_f32[2], mtxView.r[2].m128_f32[2], 0.0f);
	
	HPGauge.rot = billboardRotation;
}

void Enemy::Initialize(void)
{
	enemyAttr.Initialize();

	instance.collider.enable = true;
	instance.transform.pos = enemyAttr.initTrans.pos;
	instance.transform.rot = enemyAttr.initTrans.rot;
	instance.transform.scl = enemyAttr.initTrans.scl;

	switch (enemyAttr.enemyType)
	{
	case EnemyType::Hilichurl:
		enemyAttr.maxHP = HILI_MAX_HP;
		enemyAttr.HP = HILI_MAX_HP;
		break;
	default:
		break;
	}

	instance.renderProgress.isRandomFade = true;
	instance.renderProgress.progress = 0.0f;

}

void Enemy::SetNewPosTarget()
{
	instance.attributes.targetDir = GetRandFloat(0.0f, XM_2PI);
	enemyAttr.moveDuration = GetRandFloat(100.0f, 800.0f);  // �ړ�����
	enemyAttr.moveTimer = 0.0f;
	enemyAttr.timer = 0.0f;
}

void  Enemy::StartWaiting() 
{
	enemyAttr.isWaiting = true;
	enemyAttr.waitTime = GetRandFloat(300.0f, 900.0f);
	enemyAttr.timer = 0.0f;
}

void Enemy::CooldownWait(void)
{
	// �G����v���C���[�ւ̕����x�N�g��
	XMVECTOR enemyPosVec = XMLoadFloat3(&instance.transform.pos);
	XMVECTOR playerPosVec = XMLoadFloat3(&player->GetTransform().pos);
	XMVECTOR toPlayerVec = XMVectorSubtract(playerPosVec, enemyPosVec);
	toPlayerVec = XMVector3Normalize(toPlayerVec);

	instance.attributes.targetDir = atan2f(XMVectorGetX(toPlayerVec), XMVectorGetZ(toPlayerVec));
	enemyAttr.fixedDirMove = false;
	instance.attributes.spd = 0.0f;
	instance.attributes.isMoving = false;
}

bool Enemy::DetectPlayer(void)
{
	if (enemyAttr.isChasingPlayer == true) return true;

	XMVECTOR enemyPosVec = XMLoadFloat3(&instance.transform.pos);
	XMVECTOR playerPosVec = XMLoadFloat3(&player->GetTransform().pos);

	// �v���C���[�ւ̕����x�N�g��
	XMVECTOR diff = XMVectorSubtract(playerPosVec, enemyPosVec);
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));

	// ���F�͈͊O�Ȃ甭�����Ȃ�
	if (distSq > enemyAttr.viewDistance * enemyAttr.viewDistance) return false;

	// ���ʕ����x�N�g���iZ�������j
	XMVECTOR forward = XMVectorSet(sinf(instance.transform.rot.y), 0.0f, cosf(instance.transform.rot.y), 0.0f);
	diff = XMVector3Normalize(diff); // ���K��

	// ���ςŊp�x�����߂�
	float dot = XMVectorGetX(XMVector3Dot(forward, diff));
	float angle = acosf(dot);

	return angle <= enemyAttr.viewAngle * 0.5f;
}

bool Enemy::CheckAvailableToMove(void)
{
	if (instance.attributes.isHit1 ||
		instance.attributes.isHit2)
	{
		instance.attributes.hitTimer--;
		if (instance.attributes.hitTimer < 0)
		{
			instance.attributes.isHit1 = false;
			instance.attributes.isHit2 = false;
		}
	}

	if (instance.attributes.isHit1 ||
		instance.attributes.isHit2 ||
		instance.attributes.isAttacking)
		return false;
	else
		return true;
}

void Enemy::ChasePlayer(void)
{
	if (player == nullptr) return;

	if (enemyAttr.isChasingPlayer == false) return;

	Transform playerTransform = player->GetTransform();

	// �G����v���C���[�ւ̕����x�N�g��
	XMVECTOR enemyPosVec = XMLoadFloat3(&instance.transform.pos);
	XMVECTOR playerPosVec = XMLoadFloat3(&playerTransform.pos);

	// �v���C���[�܂ł̋���
	XMVECTOR diff = XMVectorSubtract(playerPosVec, enemyPosVec);
	float distSq = XMVectorGetX(XMVector3LengthSq(diff));
	enemyAttr.distPlayerSq = distSq;

	if (distSq < enemyAttr.attackRange * enemyAttr.attackRange * 1.5f)
	{
		// �U���͈͂ɓ�������U�����[�h
		AttackPlayer();
		return;
	}
	else if (distSq > enemyAttr.chaseRange * enemyAttr.chaseRange)
	{
		// �ǐՔ͈͂𒴂����烉���_���ړ��ɖ߂�
		enemyAttr.isChasingPlayer = false;
		enemyAttr.fixedDirMove = false;
		instance.attributes.isMoving = false;
		SetNewPosTarget();
		return;
	}
	else
		enemyAttr.fixedDirMove = false;

	// �ڕW�������v�Z
	float dx = instance.transform.pos.x - playerTransform.pos.x;
	float dz = instance.transform.pos.z - playerTransform.pos.z;

	instance.attributes.targetDir = -atan2(dz, dx) - XM_PI * 0.5f;

	// �v���C���[�����Ɉړ�
	instance.attributes.isMoving = true;
	instance.attributes.spd = VALUE_MOVE;
}

void Enemy::AttackPlayer(void)
{
	instance.attributes.isAttacking = true;
}

void Enemy::Patrol(void)
{
	// �ʏ�̈ړ����W�b�N
	if (enemyAttr.isWaiting)
	{
		if (enemyAttr.timer >= enemyAttr.waitTime)
		{
			enemyAttr.isWaiting = false;
			SetNewPosTarget();
		}
	}
	else
	{
		// �ړ�����
		enemyAttr.moveTimer += timer.GetScaledDeltaTime();

		// �ڕW���ԂɒB�����ꍇ�A�ҋ@���[�h�ɓ���
		if (enemyAttr.moveTimer >= enemyAttr.moveDuration)
		{
			instance.attributes.isMoving = false;
			StartWaiting();
		}
		else
		{
			// ���͂̂����������֌������Ĉړ�������
			instance.attributes.isMoving = true;
			instance.attributes.spd = VALUE_MOVE;
		}
	}
}

void Enemy::CooldownMove(void)
{
	// �G����v���C���[�ւ̕����x�N�g��
	XMVECTOR enemyPosVec = XMLoadFloat3(&instance.transform.pos);
	XMVECTOR playerPosVec = XMLoadFloat3(&player->GetTransform().pos);
	XMVECTOR toPlayerVec = XMVectorSubtract(playerPosVec, enemyPosVec);

	// �v���C���[�܂ł̋���
	float distSq = XMVectorGetX(XMVector3LengthSq(toPlayerVec));
	enemyAttr.distPlayerSq = distSq;

	toPlayerVec = XMVector3Normalize(toPlayerVec);
	enemyAttr.fixedDir = atan2f(XMVectorGetX(toPlayerVec), XMVectorGetZ(toPlayerVec)) * enemyAttr.cooldownMoveDirection;
	enemyAttr.fixedDirMove = true;
	enemyAttr.cooldownOrbitRadius = sqrtf(enemyAttr.distPlayerSq);
	instance.attributes.targetDir = enemyAttr.fixedDir;
	instance.attributes.spd = VALUE_MOVE * 0.5f;
	instance.attributes.isMoving = true;
}

//=============================================================================
// ���_���̍쐬
//=============================================================================
HRESULT Enemy::MakeVertexHPGauge(float width, float height)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Renderer::get_instance().GetDevice()->CreateBuffer(&bd, NULL, &HPGaugeVertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	Renderer::get_instance().GetDeviceContext()->Map(HPGaugeVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	HPGauge.fHeight = height;
	HPGauge.fWidth = width;
	ZeroMemory(&HPGauge.material, sizeof(HPGauge.material));

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(-HPGauge.fWidth / 2.0f, HPGauge.fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(HPGauge.fWidth / 2.0f, HPGauge.fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-HPGauge.fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(HPGauge.fWidth / 2.0f, 0.0f, 0.0f);

	// �@���̐ݒ�
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	renderer.GetDeviceContext()->Unmap(HPGaugeVertexBuffer, 0);

	return S_OK;
}