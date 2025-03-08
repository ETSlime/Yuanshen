//=============================================================================
//
// Lumineˆ— [Lumine.cpp]
// Author : 
//
//=============================================================================
#include "Lumine.h"
#include "input.h"
#include "debugproc.h"
#include "Player.h"
#include "Enemy.h"
//*****************************************************************************
// ƒ}ƒNƒ’è‹`
//*****************************************************************************
#define PLAY_ANIM_SPD				1.0f
#define ANIM_BLEND_SPD				0.032f

#define WEAPON_SIZE						50.0f
#define WEAPON_ON_HANDS_POS_OFFSET_X	-55.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Y	95.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Z	-35.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_X	0.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_Y	XM_PI
#define WEAPON_ON_HANDS_ROT_OFFSET_Z	0.0f

#define WEAPON_ON_BACK_POS_OFFSET_X		0.0f
#define WEAPON_ON_BACK_POS_OFFSET_Y		95.0f
#define WEAPON_ON_BACK_POS_OFFSET_Z		25.0f
#define WEAPON_ON_BACK_ROT_OFFSET_X		XM_PI * 1.5f
#define WEAPON_ON_BACK_ROT_OFFSET_Y		0.0f
#define WEAPON_ON_BACK_ROT_OFFSET_Z		XM_PI

#define ATTACK_COMBO_WINDOW		(70)
#define ATTACK_RANGE			(120.0f)
#define	MAX_ATTACK_STEP			(800.0f)
#define WEAPON_ON_BACK_TIME		(600.0f)
#define WEAPON_ON_HAND_TIME		(200.0f)

Lumine::Lumine()
{
	Instantiate("data/MODEL/character/Lumine", "Character_output.fbx", ModelType::Lumine);
	instance.collider.type = ColliderType::PLAYER;
	instance.collider.owner = this;
	instance.collider.enable = true;
	CollisionManager::get_instance().RegisterCollider(&instance.collider);

	LoadWeapon("data/MODEL/character/Lumine", "Mitsurugi.fbx");
	weapon.SetColliderType(ColliderType::PLAYER_ATTACK);
	weapon.SetColliderOwner(&weapon);
	CollisionManager::get_instance().RegisterCollider(&weapon.GetCollider());
	
	instance.pModel->SetDrawBoundingBox(false);
	weapon.GetSkinnedMeshModel()->SetDrawBoundingBox(false);

	AddAnimation("data/MODEL/character/Lumine/", "Idle.fbx", AnimationClipName::ANIM_STANDING);
	AddAnimation("data/MODEL/character/Lumine/", "Looking Around.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/character/Lumine/", "Sneak Walk.fbx", AnimationClipName::ANIM_DASH);
	AddAnimation("data/MODEL/character/Lumine/", "Running.fbx", AnimationClipName::ANIM_RUN);
	AddAnimation("data/MODEL/character/Lumine/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	AddAnimation("data/MODEL/character/Lumine/", "Jump.fbx", AnimationClipName::ANIM_JUMP);
	AddAnimation("data/MODEL/character/Lumine/", "Standing React Small From Left.fbx", AnimationClipName::ANIM_HIT_REACTION_1);
	AddAnimation("data/MODEL/character/Lumine/", "Sword And Shield Slash.fbx", AnimationClipName::ANIM_SWORD_SHIELD_SLASH);
	AddAnimation("data/MODEL/character/Lumine/", "Sword And Shield Slash 2.fbx", AnimationClipName::ANIM_SWORD_SHIELD_SLASH2);
	AddAnimation("data/MODEL/character/Lumine/", "Sword And Shield Slash 3.fbx", AnimationClipName::ANIM_SWORD_SHIELD_SLASH3);

	instance.pModel->SetBodyDiffuseTexture("data/MODEL/character/Lumine/texture_0.png");
	instance.pModel->SetHairDiffuseTexture("data/MODEL/character/Lumine/hair.png");
	instance.pModel->SetFaceDiffuseTexture("data/MODEL/character/Lumine/face.png");

	playAnimSpeed = PLAY_ANIM_SPD;
	weaponOnBackTimer = 0;
	weaponOnHandTimer = 0;
	weaponOnBack = false;
	storeWeaponOnBack = false;
	storeWeapon = false;
	instance.renderProgress.progress = 0.0f;
	SetupAnimationStateMachine();
}

Lumine::~Lumine()
{
	SAFE_DELETE(stateMachine);
}

void Lumine::AddAnimation(char* animPath, char* animName, AnimationClipName clipName)
{
	if (instance.pModel)
		fbxLoader.LoadAnimation(renderer.GetDevice(), *instance.pModel, animPath, animName, clipName);
}

void Lumine::LoadWeapon(char* modelPath, char* modelName)
{
	weapon.Instantiate(modelPath, modelName, ModelType::Weapon);
	weapon.SetScale(XMFLOAT3(WEAPON_SIZE, WEAPON_SIZE, WEAPON_SIZE));
	weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
	weapon.SetRotation(XMFLOAT3(WEAPON_ON_HANDS_ROT_OFFSET_X, WEAPON_ON_HANDS_ROT_OFFSET_Y, WEAPON_ON_HANDS_ROT_OFFSET_Z));
}

void Lumine::Update(void)
{
	if (!GetKeyboardPress(DIK_W)
		&& !GetKeyboardPress(DIK_S)
		&& !GetKeyboardPress(DIK_A)
		&& !GetKeyboardPress(DIK_D))
		instance.attributes.isMoving = false;



	if (instance.attributes.attackWindow2 == true
		|| instance.attributes.attackWindow3 == true)
	{
		instance.attributes.attackWinwdowCnt++;
		if (instance.attributes.attackWinwdowCnt >= ATTACK_COMBO_WINDOW
			|| instance.attributes.isMoving)
		{
			instance.attributes.attackWindow2 = false;
			instance.attributes.attackWindow3 = false;
			instance.attributes.attackWinwdowCnt = 0;
		}
	}



	switch (stateMachine->GetCurrentState())
	{
	case STATE(PlayerState::STANDING):
		PlayStandingAnim();
		break;
	case STATE(PlayerState::IDLE):
		PlayIdleAnim();
		break;
	case STATE(PlayerState::WALK):
		PlayWalkAnim();
		break;
	case STATE(PlayerState::RUN):
		PlayRunAnim();
		break;
	case STATE(PlayerState::DASH):
		PlayDashAnim();
		break;
	case STATE(PlayerState::JUMP):
		PlayJumpAnim();
		break;
	case STATE(PlayerState::ATTACK_1):
		PlayAttackAnim();
		break;
	case STATE(PlayerState::ATTACK_2):
		PlayAttack2Anim();
		break;
	case STATE(PlayerState::ATTACK_3):
		PlayAttack3Anim();
		break;
	case STATE(PlayerState::HIT):
		PlayHitAnim();
		break;
	default:
		break;
	}

	GameObject::Update();
	stateMachine->Update(ANIM_BLEND_SPD, dynamic_cast<ISkinnedMeshModelChar*>(this));
	instance.pModel->UpdateBoneTransform(stateMachine->GetBoneMatrices());

	UpdateWeapon();
}

void Lumine::Draw(void)
{
	if (storeWeaponOnBack)
	{
		instance.renderProgress.isRandomFade = true;
		instance.renderProgress.progress -= 0.01f;
		if (instance.renderProgress.progress < 0.0f)
		{
			instance.renderProgress.progress = 0.0f;
			weaponOnBack = true;
			weaponOnBackTimer = WEAPON_ON_BACK_TIME;
			storeWeaponOnBack = false;
			weapon.SetPosition(XMFLOAT3(WEAPON_ON_BACK_POS_OFFSET_X, WEAPON_ON_BACK_POS_OFFSET_Y, WEAPON_ON_BACK_POS_OFFSET_Z));
			weapon.SetRotation(XMFLOAT3(WEAPON_ON_BACK_ROT_OFFSET_X, WEAPON_ON_BACK_ROT_OFFSET_Y, WEAPON_ON_BACK_ROT_OFFSET_Z));
		}
	}
	else if (weaponOnBackTimer >= 0 && weaponOnBack == true)
	{
		instance.renderProgress.progress += 0.01f;
		if (instance.renderProgress.progress >= 1.0f)
		{
			//weaponOnBack = false;
			instance.renderProgress.progress = 1.0f;
		}
	}
	else if (storeWeapon)
	{
		instance.renderProgress.progress -= 0.01f;
		if (instance.renderProgress.progress < 0.0f)
		{
			instance.renderProgress.progress = 0.0f;
			storeWeapon = false;
		}
	}

	renderer.SetRenderProgress(instance.renderProgress);
	if (instance.renderProgress.progress > 0)
		weapon.Draw();

	if (instance.attributes.charSwitchEffect == TRUE)
	{
		instance.renderProgress.progress += 0.01f;
		if (instance.renderProgress.progress >= 1.0f)
		{
			instance.renderProgress.progress = 1.0f;
			instance.attributes.charSwitchEffect = FALSE;
		}
	}
	else
	{
		RenderProgressBuffer defaultRenderProgress;
		defaultRenderProgress.isRandomFade = false;
		defaultRenderProgress.progress = 1.0f;

		renderer.SetRenderProgress(defaultRenderProgress);

	}
	GameObject::Draw();



}

AnimationStateMachine* Lumine::GetStateMachine(void)
{
	return stateMachine;
}

void Lumine::SetupAnimationStateMachine()
{
	stateMachine = new AnimationStateMachine(dynamic_cast<ISkinnedMeshModelChar*>(this));

	stateMachine->AddState(STATE(PlayerState::IDLE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_IDLE));
	stateMachine->AddState(STATE(PlayerState::STANDING), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_STANDING));
	stateMachine->AddState(STATE(PlayerState::WALK), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_WALK));
	stateMachine->AddState(STATE(PlayerState::RUN), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_RUN));
	stateMachine->AddState(STATE(PlayerState::DASH), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_DASH));
	stateMachine->AddState(STATE(PlayerState::JUMP), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_JUMP));
	stateMachine->AddState(STATE(PlayerState::ATTACK_1), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_SWORD_SHIELD_SLASH));
	stateMachine->AddState(STATE(PlayerState::ATTACK_2), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_SWORD_SHIELD_SLASH2));
	stateMachine->AddState(STATE(PlayerState::ATTACK_3), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_SWORD_SHIELD_SLASH3));
	stateMachine->AddState(STATE(PlayerState::HIT), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_HIT_REACTION_1));

	//ó‘Ô‘JˆÚ
	stateMachine->AddTransition(STATE(PlayerState::STANDING), STATE(PlayerState::WALK), &ISkinnedMeshModelChar::CanWalk);
	stateMachine->AddTransition(STATE(PlayerState::STANDING), STATE(PlayerState::ATTACK_1), &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(STATE(PlayerState::STANDING), STATE(PlayerState::ATTACK_2), &ISkinnedMeshModelChar::CanAttack2);
	stateMachine->AddTransition(STATE(PlayerState::STANDING), STATE(PlayerState::ATTACK_3), &ISkinnedMeshModelChar::CanAttack3);
	stateMachine->AddTransition(STATE(PlayerState::STANDING), STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::CanJump);
	stateMachine->AddTransition(STATE(PlayerState::STANDING), STATE(PlayerState::HIT), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(PlayerState::JUMP), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::ATTACK_1), &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::CanStopMoving);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::RUN), &ISkinnedMeshModelChar::CanRun);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::CanJump);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::HIT), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(PlayerState::RUN), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::CanStopMoving);
	stateMachine->AddTransition(STATE(PlayerState::RUN), STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::CanJump);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_1), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_1), STATE(PlayerState::HIT), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_2), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_2), STATE(PlayerState::HIT), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_3), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_3), STATE(PlayerState::HIT), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_1), STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::CanJump, false);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_2), STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::CanJump, false);
	stateMachine->AddTransition(STATE(PlayerState::ATTACK_3), STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::CanJump, false);
	stateMachine->AddTransition(STATE(PlayerState::HIT), STATE(PlayerState::STANDING), &ISkinnedMeshModelChar::AlwaysTrue, true);


	stateMachine->SetEndCallback(STATE(PlayerState::ATTACK_1), &ISkinnedMeshModelChar::OnAttackAnimationEnd);
	stateMachine->SetEndCallback(STATE(PlayerState::ATTACK_2), &ISkinnedMeshModelChar::OnAttackAnimationEnd);
	stateMachine->SetEndCallback(STATE(PlayerState::ATTACK_3), &ISkinnedMeshModelChar::OnAttackAnimationEnd);
	stateMachine->SetEndCallback(STATE(PlayerState::HIT), &ISkinnedMeshModelChar::OnHitAnimationEnd);
	stateMachine->SetEndCallback(STATE(PlayerState::JUMP), &ISkinnedMeshModelChar::OnJumpAnimationEnd);

	stateMachine->SetCurrentState(STATE(PlayerState::STANDING));
}

void Lumine::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayJumpAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_JUMP)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayStandingAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Lumine::ExecuteAction(ActionEnum action)
{
	switch (action)
	{
	case ActionEnum::ATTACK:
		if (stateMachine->GetCurrentState() == STATE(PlayerState::ATTACK_1) ||
			stateMachine->GetCurrentState() == STATE(PlayerState::ATTACK_2) ||
			stateMachine->GetCurrentState() == STATE(PlayerState::ATTACK_3) ||
			stateMachine->GetCurrentState() == STATE(PlayerState::JUMP) || 
			stateMachine->GetCurrentState() == STATE(PlayerState::RUN))
			return false;
		else
		{
			if (instance.attributes.attackWindow2)
			{
				instance.attributes.isAttacking = false;
				instance.attributes.isAttacking2 = true;
				instance.attributes.isAttacking3 = false;
			}

			else if (instance.attributes.attackWindow3)
			{
				instance.attributes.isAttacking = false;
				instance.attributes.isAttacking2 = false;
				instance.attributes.isAttacking3 = true;
			}
			else
			{
				instance.attributes.isAttacking = true;
				instance.attributes.isAttacking2 = false;
				instance.attributes.isAttacking3 = false;
			}
			weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
			weapon.SetRotation(XMFLOAT3(WEAPON_ON_HANDS_ROT_OFFSET_X, WEAPON_ON_HANDS_ROT_OFFSET_Y, WEAPON_ON_HANDS_ROT_OFFSET_Z));
			weaponOnBack = false;
			instance.renderProgress.progress = 1.0f;
			instance.attributes.attackWinwdowCnt = 0;
			return true;
		}
	default:
		return false;
	}
}

void Lumine::PlayDashAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_DASH)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayAttackAnim(void)
{
	FaceToNearestEnemy();

	weapon.SetColliderEnable(true);
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_SWORD_SHIELD_SLASH)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed * 1.5f);
}

void Lumine::PlayAttack2Anim(void)
{
	FaceToNearestEnemy();

	weapon.SetColliderEnable(true);
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_SWORD_SHIELD_SLASH2)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed * 1.5f);
}

void Lumine::PlayAttack3Anim(void)
{
	FaceToNearestEnemy();

	weapon.SetColliderEnable(true);
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_SWORD_SHIELD_SLASH3)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed * 1.5f);
}

void Lumine::PlayHitAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_HIT_REACTION_1)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed * 1.2f);
}

void Lumine::FaceToNearestEnemy(void)
{
	XMVECTOR playerPosVec = XMLoadFloat3(&instance.transform.pos);
	auto enemyList = EnemyManager::get_instance().GetEnemy();
	Node<Enemy*>* cur = enemyList->getHead();
	Enemy* nearestEnemy = nullptr;
	float minDistSq = FLT_MAX;
	while (cur != nullptr)
	{
		XMVECTOR enemyPosVec = XMLoadFloat3(&cur->data->GetTransform().pos);
		XMVECTOR diff = XMVectorSubtract(enemyPosVec, playerPosVec);
		float distSq = XMVectorGetX(XMVector3LengthSq(diff));

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			nearestEnemy = cur->data;
		}

		cur = cur->next;
	}

	float dx = nearestEnemy->GetTransform().pos.x - instance.transform.pos.x;
	float dz = nearestEnemy->GetTransform().pos.z - instance.transform.pos.z;

	instance.transform.rot.y = -atan2(dz, dx) + XM_PI * 0.5f;
	instance.attributes.dir = instance.transform.rot.y;
	instance.attributes.targetDir = instance.transform.rot.y;

	if (minDistSq > ATTACK_RANGE * ATTACK_RANGE)
	{
		float moveStep = min(minDistSq - ATTACK_RANGE, MAX_ATTACK_STEP);

		float t = min(moveStep / minDistSq, 1.0f);


		instance.transform.pos.x = (1 - t) * instance.transform.pos.x + t * nearestEnemy->GetTransform().pos.x;
		instance.transform.pos.z = (1 - t) * instance.transform.pos.z + t * nearestEnemy->GetTransform().pos.z;
	}
}

void Lumine::UpdateWeapon(void)
{
	if (weaponOnHandTimer > 0)
	{
		weaponOnHandTimer--;
		if (weaponOnHandTimer <= 0)
		{
			storeWeaponOnBack = true;
			instance.renderProgress.progress = 1.0f;
		}
	}

	if (weaponOnBackTimer > 0)
	{
		weaponOnBackTimer--;
		if (weaponOnBackTimer <= 0)
		{
			storeWeapon = true;
		}
	}




	weapon.Update();
	XMMATRIX weaponMtx;
	if (!weaponOnBack)
		weaponMtx = instance.pModel->GetWeaponTransformMtx();
	else
		weaponMtx = instance.pModel->GetBodyTransformMtx();

	weaponMtx = XMMatrixMultiply(weapon.GetWorldMatrix(), weaponMtx);
	weaponMtx = XMMatrixMultiply(weaponMtx, instance.transform.mtxWorld);

	weapon.SetWorldMatrix(weaponMtx);
	BOUNDING_BOX weaponBB = weapon.GetSkinnedMeshModel()->GetBoundingBox();
	XMVECTOR maxPointVect = XMVectorSet(weaponBB.maxPoint.x, weaponBB.maxPoint.y, weaponBB.maxPoint.z, 1.0f);
	XMVECTOR minPointVect = XMVectorSet(weaponBB.minPoint.x, weaponBB.minPoint.y, weaponBB.minPoint.z, 1.0f);

	XMVECTOR transformedMaxPoint = XMVector3TransformCoord(maxPointVect, weaponMtx);
	XMVECTOR transformedMinPoint = XMVector3TransformCoord(minPointVect, weaponMtx);

	XMFLOAT3 newMaxPoint, newMinPoint;
	XMStoreFloat3(&newMaxPoint, transformedMaxPoint);
	XMStoreFloat3(&newMinPoint, transformedMinPoint);

	weaponBB.maxPoint = XMFLOAT3(
		max(newMaxPoint.x, newMinPoint.x),
		max(newMaxPoint.y, newMinPoint.y),
		max(newMaxPoint.z, newMinPoint.z)
	);

	weaponBB.minPoint = XMFLOAT3(
		min(newMaxPoint.x, newMinPoint.x),
		min(newMaxPoint.y, newMinPoint.y),
		min(newMaxPoint.z, newMinPoint.z)
	);

	weapon.SetColliderBoundingBox(weaponBB);
}

bool Lumine::CanWalk(void) const
{
	return instance.attributes.isMoving && 
		!instance.attributes.isAttacking &&
		!instance.attributes.isAttacking2 &&
		!instance.attributes.isAttacking3;
}

bool Lumine::CanStopMoving() const
{
	return !instance.attributes.isMoving;
}

bool Lumine::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Lumine::CanAttack2() const
{
	return (instance.attributes.attackWindow2 && instance.attributes.isAttacking2);
}

bool Lumine::CanAttack3() const
{
	return (instance.attributes.attackWindow3 && instance.attributes.isAttacking3);
}

bool Lumine::CanRun(void) const
{
	return instance.attributes.isMoving && GetKeyboardPress(DIK_LSHIFT);
}

bool Lumine::CanHit(void) const
{
	return instance.attributes.isHit1;
}

bool Lumine::CanJump(void) const
{
	return instance.attributes.isJumping;
}

void Lumine::OnAttackAnimationEnd(void)
{
	weapon.SetColliderEnable(false);
	storeWeaponOnBack = false;
	weaponOnHandTimer = WEAPON_ON_HAND_TIME;
	instance.attributes.isAttacking = false;
	instance.attributes.isAttacking2 = false;
	instance.attributes.isAttacking3 = false;

	if (instance.attributes.attackWindow2 == false && 
		instance.attributes.attackWindow3 == false)
	{
		instance.attributes.attackWindow2 = true;
	}
	else if (instance.attributes.attackWindow3 == false)
	{
		instance.attributes.attackWindow2 = false;
		instance.attributes.attackWindow3 = true;
	}
	else
	{
		instance.attributes.attackWindow2 = false;
		instance.attributes.attackWindow3 = false;
	}

	instance.attributes.attackWinwdowCnt = 0;
}

void Lumine::OnHitAnimationEnd(void)
{
	instance.attributes.isHit1 = false;
}

void Lumine::OnJumpAnimationEnd(void)
{
	instance.attributes.isJumping = false;
}
