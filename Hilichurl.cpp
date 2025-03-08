//=============================================================================
//
// Hilichurlˆ— [Hilichurl.cpp]
// Author : 
//
//=============================================================================
#include "Hilichurl.h"
#include "input.h"
#include "debugproc.h"
#include "Player.h"
//*****************************************************************************
// ƒ}ƒNƒ’è‹`
//*****************************************************************************
#define PLAY_ANIM_SPD				1.0f
#define ANIM_BLEND_SPD				0.032f

#define WEAPON_SIZE						35.0f
#define WEAPON_ON_HANDS_POS_OFFSET_X	-55.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Y	45.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Z	-35.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_X	0.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_Y	XM_PI * 0.5f
#define WEAPON_ON_HANDS_ROT_OFFSET_Z	XM_PI * 0.5f

Hilichurl::Hilichurl(Transform transform, EnemyState initState):Enemy(EnemyType::Hilichurl, transform)
{
	Instantiate("data/MODEL/enemy/Hilichurl", "Character_output.fbx", ModelType::Hilichurl);
	instance.collider.type = ColliderType::ENEMY;
	instance.collider.owner = this;
	instance.collider.enable = true;
	CollisionManager::get_instance().RegisterCollider(&instance.collider);

	LoadWeapon("data/MODEL/enemy/Hilichurl", "Stick.fbx");
	weapon.SetColliderType(ColliderType::ENEMY_ATTACK);
	weapon.SetColliderOwner(&weapon);
	CollisionManager::get_instance().RegisterCollider(&weapon.GetCollider());

	//instance.pModel->SetDrawBoundingBox(false);
	//weapon.GetSkinnedMeshModel()->SetDrawBoundingBox(false);

	AddAnimation("data/MODEL/enemy/Hilichurl/", "Happy Idle.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Running.fbx", AnimationClipName::ANIM_RUN);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Standing Melee Attack Horizontal.fbx", AnimationClipName::ANIM_STANDING_MELEE_ATTACK);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Falling Back Death.fbx", AnimationClipName::ANIM_FALLING_BACK_DEATH);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Flying Back Death.fbx", AnimationClipName::ANIM_FLYING_BACK_DEATH);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Getting Up.fbx", AnimationClipName::ANIM_GETTING_UP);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Hit Reaction 1.fbx", AnimationClipName::ANIM_HIT_REACTION_1);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Hit Reaction 2.fbx", AnimationClipName::ANIM_HIT_REACTION_2);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Sit.fbx", AnimationClipName::ANIM_SIT);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Surprised.fbx", AnimationClipName::ANIM_SURPRISED);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Swing Dancing.fbx", AnimationClipName::ANIM_DANCE);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Falling Back Death.fbx", AnimationClipName::ANIM_DIE);


	playAnimSpeed = PLAY_ANIM_SPD;

	SetupAnimationStateMachine(initState);
}

void Hilichurl::Initialize(void)
{
	Enemy::Initialize();
	stateMachine->SetCurrentState(STATE(enemyAttr.initState));
}

Hilichurl::~Hilichurl()
{
	SAFE_DELETE(stateMachine);
}

void Hilichurl::AddAnimation(char* animPath, char* animName, AnimationClipName clipName)
{
	if (instance.pModel)
		fbxLoader.LoadAnimation(renderer.GetDevice(), *instance.pModel, animPath, animName, clipName);
}

void Hilichurl::LoadWeapon(char* modelPath, char* modelName)
{
	weapon.Instantiate(modelPath, modelName, ModelType::Weapon);
	weapon.SetScale(XMFLOAT3(WEAPON_SIZE, WEAPON_SIZE, WEAPON_SIZE));
	weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
	weapon.SetRotation(XMFLOAT3(WEAPON_ON_HANDS_ROT_OFFSET_X, WEAPON_ON_HANDS_ROT_OFFSET_Y, WEAPON_ON_HANDS_ROT_OFFSET_Z));
}

void Hilichurl::Update(void)
{
	Enemy::Update();

	stateMachine->Update(ANIM_BLEND_SPD, dynamic_cast<ISkinnedMeshModelChar*>(this));

	switch (stateMachine->GetCurrentState())
	{
	case STATE(EnemyState::IDLE):
		PlayIdleAnim();
		break;
	case STATE(EnemyState::HILI_WALK):
		PlayWalkAnim();
		break;
	case STATE(EnemyState::HILI_RUN):
		PlayRunAnim();
		break;
	case STATE(EnemyState::HILI_ATTACK):
		PlayAttackAnim();
		break;
	case STATE(EnemyState::HILI_HIT1):
		PlayHitAnim();
		break;
	case STATE(EnemyState::HILI_HIT2):
		PlayHit2Anim();
		break;
	case STATE(EnemyState::HILI_SIT):
		PlaySitAnim();
		break;
	case STATE(EnemyState::HILI_SURPRISED):
		PlaySurprisedAnim();
		break;
	case STATE(EnemyState::HILI_DIE):
		PlayDieAnim();
		break;
	case STATE(EnemyState::HILI_DANCE):
		PlayDanceAnim();
		break;
	default:
		break;
	}

	instance.pModel->UpdateBoneTransform(stateMachine->GetBoneMatrices());

	UpdateWeapon();

}

void Hilichurl::Draw(void)
{
	GameObject::Draw();
	weapon.Draw();
}

void Hilichurl::UpdateWeapon(void)
{
	weapon.Update();
	XMMATRIX weaponMtx = instance.pModel->GetWeaponTransformMtx();
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

AnimationStateMachine* Hilichurl::GetStateMachine(void)
{
	return stateMachine;
}

void Hilichurl::SetupAnimationStateMachine(EnemyState initState)
{
	stateMachine = new AnimationStateMachine(dynamic_cast<ISkinnedMeshModelChar*>(this));

	stateMachine->AddState(STATE(EnemyState::IDLE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_IDLE));
	stateMachine->AddState(STATE(EnemyState::HILI_SIT), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_SIT));
	stateMachine->AddState(STATE(EnemyState::HILI_ATTACK), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_STANDING_MELEE_ATTACK));
	stateMachine->AddState(STATE(EnemyState::HILI_WALK), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_WALK));
	stateMachine->AddState(STATE(EnemyState::HILI_RUN), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_RUN));
	stateMachine->AddState(STATE(EnemyState::HILI_HIT1), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_HIT_REACTION_1));
	stateMachine->AddState(STATE(EnemyState::HILI_HIT2), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_HIT_REACTION_2));
	stateMachine->AddState(STATE(EnemyState::HILI_SURPRISED), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_SURPRISED));
	instance.pModel->GetAnimationClip(AnimationClipName::ANIM_DIE)->SetAnimLoop(false);
	stateMachine->AddState(STATE(EnemyState::HILI_DIE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_DIE));
	stateMachine->AddState(STATE(EnemyState::HILI_DANCE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_DANCE));

	//ó‘Ô‘JˆÚ
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_WALK), &ISkinnedMeshModelChar::CanWalk);
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_ATTACK), &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_SURPRISED), &ISkinnedMeshModelChar::CanSurprised);
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::HILI_RUN), &ISkinnedMeshModelChar::CanRun);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::CanStopMoving);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::HILI_SURPRISED), &ISkinnedMeshModelChar::CanSurprised);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::HILI_ATTACK), &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_RUN), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::CanStopMoving);
	stateMachine->AddTransition(STATE(EnemyState::HILI_RUN), STATE(EnemyState::HILI_WALK), &ISkinnedMeshModelChar::CanStopRunning);
	stateMachine->AddTransition(STATE(EnemyState::HILI_RUN), STATE(EnemyState::HILI_WALK), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(EnemyState::HILI_RUN), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT1), STATE(EnemyState::HILI_HIT2), &ISkinnedMeshModelChar::CanHit2, false);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT1), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT2), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit, false);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT2), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT1), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT2), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_SURPRISED), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_SURPRISED), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_SURPRISED), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_ATTACK), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_ATTACK), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_ATTACK), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::CanDie);
	stateMachine->AddTransition(STATE(EnemyState::HILI_DIE), STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_DANCE), STATE(EnemyState::HILI_SURPRISED), &ISkinnedMeshModelChar::CanSurprised);
	stateMachine->AddTransition(STATE(EnemyState::HILI_DANCE), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT1), STATE(EnemyState::HILI_SURPRISED), &ISkinnedMeshModelChar::CanHit);

	stateMachine->SetEndCallback(STATE(EnemyState::HILI_ATTACK), &ISkinnedMeshModelChar::OnAttackAnimationEnd);
	stateMachine->SetEndCallback(STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::OnHitAnimationEnd);
	stateMachine->SetEndCallback(STATE(EnemyState::HILI_HIT2), &ISkinnedMeshModelChar::OnHitAnimationEnd);
	stateMachine->SetEndCallback(STATE(EnemyState::HILI_SURPRISED), &ISkinnedMeshModelChar::OnSurprisedEnd);
	stateMachine->SetEndCallback(STATE(EnemyState::HILI_DIE), &ISkinnedMeshModelChar::OnDieAnimationEnd);
	

	enemyAttr.initState = initState;
	stateMachine->SetCurrentState(STATE(initState));
}

void Hilichurl::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayAttackAnim(void)
{
	weapon.SetColliderEnable(true);

	float dist = enemyAttr.distPlayer;

	if (dist > enemyAttr.attackRange * enemyAttr.attackRange)
	{
		float moveStep = min(dist - enemyAttr.attackRange, HILI_MAX_ATTACK_STEP);

		float t = min(moveStep / dist, 1.0f);

		instance.transform.pos.x = (1 - t) * instance.transform.pos.x + t * player->GetTransform().pos.x;
		instance.transform.pos.z = (1 - t) * instance.transform.pos.z + t * player->GetTransform().pos.z;
	}


	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_MELEE_ATTACK)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayHitAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_HIT_REACTION_1)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayHit2Anim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_HIT_REACTION_2)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlaySitAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_SIT)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlaySurprisedAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_SURPRISED)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed * 1.2f);
}

void Hilichurl::PlayDieAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_DIE)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayDanceAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_DANCE)
		instance.pModel->SetCurrentAnim(stateMachine->GetCurrentAnimClip());
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Hilichurl::CanWalk(void) const
{
	return instance.attributes.isMoving && !instance.attributes.isAttacking;
}

bool Hilichurl::CanStopMoving() const
{
	return !instance.attributes.isMoving;
}

bool Hilichurl::CanStopRunning() const
{
	return !instance.attributes.isRunning && instance.attributes.isMoving;
}

bool Hilichurl::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Hilichurl::CanRun(void) const
{
	return instance.attributes.isRunning;
}

bool Hilichurl::CanHit(void) const
{
	return instance.attributes.isHit1;
}

bool Hilichurl::CanHit2(void) const
{
	return instance.attributes.isHit2;
}

bool Hilichurl::CanSurprised(void) const
{
	return enemyAttr.isSurprised;
}

bool Hilichurl::CanDie(void) const
{
	return enemyAttr.die;
}

void Hilichurl::OnAttackAnimationEnd(void)
{
	weapon.SetColliderEnable(false);

	instance.attributes.isAttacking = false;

	enemyAttr.isInCooldown = true;
	enemyAttr.attackCooldownTimer = GetRandFloat(HILI_MIN_ATTACK_CDTIME, HILI_MAX_ATTACK_CDTIME);

	instance.attributes.isMoving = false;
	enemyAttr.cooldownMoveDirection = (GetRand(0, 1) == 0) ? -1.0f : 1.0f;
	enemyAttr.cooldownMoveDistance = GetRandFloat(HILI_MIN_COOLDOWN_MOVE_DISTANCE, HILI_MAX_COOLDOWN_MOVE_DISTANCE);
	enemyAttr.cooldownWaitTime = enemyAttr.attackCooldownTimer * GetRandFloat(HILI_MIN_COOLDOWN_WAIT_TIME, HILI_MAX_COOLDOWN_WAIT_TIME);
}

void Hilichurl::OnHitAnimationEnd(void)
{
	instance.attributes.isHit1 = false;
	instance.attributes.isHit2 = false;
	instance.attributes.hitTimer = 0;

	enemyAttr.isChasingPlayer = true;
}

void Hilichurl::OnSurprisedEnd(void)
{
	enemyAttr.isSurprised = false;
	enemyAttr.isChasingPlayer = true;
	enemyAttr.randomMove = true;
}

void Hilichurl::OnDieAnimationEnd()
{
	enemyAttr.startFadeOut = true;
}
