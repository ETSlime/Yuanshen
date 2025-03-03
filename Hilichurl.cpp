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

#define WEAPON_SIZE						30.0f
#define WEAPON_ON_HANDS_POS_OFFSET_X	25.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Y	65.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Z	-5.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_X	25.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_Y	65.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_Z	-5.0f
#define WEAPON_ON_BACK_POS_OFFSET_X		0.0f
#define WEAPON_ON_BACK_POS_OFFSET_Y		XM_PI
#define WEAPON_ON_BACK_POS_OFFSET_Z		0.0f

Hilichurl::Hilichurl(Transform transform, EnemyState initState):Enemy(EnemyType::Hilichurl, transform)
{
	Instantiate("data/MODEL/enemy/Hilichurl", "Character_output.fbx", ModelType::Hilichurl);
	instance.collider.type = ColliderType::ENEMY;
	instance.collider.owner = this;
	instance.collider.enable = true;
	CollisionManager::get_instance().RegisterCollider(&instance.collider);


	AddAnimation("data/MODEL/enemy/Hilichurl/", "Happy Idle.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Running.fbx", AnimationClipName::ANIM_RUN);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Standing Melee Attack Downward.fbx", AnimationClipName::ANIM_STANDING_MELEE_ATTACK);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Falling Back Death.fbx", AnimationClipName::ANIM_FALLING_BACK_DEATH);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Flying Back Death.fbx", AnimationClipName::ANIM_FLYING_BACK_DEATH);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Getting Up.fbx", AnimationClipName::ANIM_GETTING_UP);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Hit Reaction 1.fbx", AnimationClipName::ANIM_HIT_REACTION_1);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Hit Reaction 2.fbx", AnimationClipName::ANIM_HIT_REACTION_2);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Sit.fbx", AnimationClipName::ANIM_SIT);
	AddAnimation("data/MODEL/enemy/Hilichurl/", "Surprised.fbx", AnimationClipName::ANIM_SURPRISED);

	playAnimSpeed = PLAY_ANIM_SPD;

	SetupAnimationStateMachine(initState);
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
	//weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
	//weapon.SetRotation(XMFLOAT3(WEAPON_ON_BACK_POS_OFFSET_X, WEAPON_ON_BACK_POS_OFFSET_Y, WEAPON_ON_BACK_POS_OFFSET_Z));
}

void Hilichurl::SetCurrentAnim(AnimationClipName clipName, float startTime)
{
	instance.pModel->SetCurrentAnim(clipName, startTime);
}

void Hilichurl::Update(void)
{

	//if (stateMachine->GetCurrentState() == STATE(EnemyState::IDLE))
	//{
	//	if (IsMouseLeftTriggered())
	//	{
	//		instance.attributes.isAttacking = true;
	//	}
	//}

	//if (!GetKeyboardPress(DIK_W)
	//	&& !GetKeyboardPress(DIK_S)
	//	&& !GetKeyboardPress(DIK_A)
	//	&& !GetKeyboardPress(DIK_D))
	//	instance.attributes.isMoving = false;

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
	default:
		break;
	}

	instance.pModel->UpdateBoneTransform(stateMachine->GetBoneMatrices());

	//weapon.Update();

	//XMMATRIX weaponMtx = instance.pModel->GetWeaponTransformMtx();
	//weaponMtx = XMMatrixMultiply(weapon.GetWorldMatrix(), weaponMtx);
	//weaponMtx = XMMatrixMultiply(weaponMtx, instance.transform.mtxWorld);

	//weapon.SetWorldMatrix(weaponMtx);

}

void Hilichurl::Draw(void)
{
	GameObject::Draw();
	//weapon.Draw();
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

	//ó‘Ô‘JˆÚ
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_WALK), &ISkinnedMeshModelChar::CanWalk);
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_ATTACK), &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(STATE(EnemyState::HILI_WALK), STATE(EnemyState::HILI_RUN), &ISkinnedMeshModelChar::CanRun);
	stateMachine->AddTransition(STATE(EnemyState::IDLE), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT1), STATE(EnemyState::HILI_HIT2), &ISkinnedMeshModelChar::CanHit2, false);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT2), STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::CanHit, false);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT1), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::AlwaysTrue, true);
	stateMachine->AddTransition(STATE(EnemyState::HILI_HIT2), STATE(EnemyState::IDLE), &ISkinnedMeshModelChar::AlwaysTrue, true);

	stateMachine->SetEndCallback(STATE(EnemyState::HILI_ATTACK), &ISkinnedMeshModelChar::OnAttackAnimationEnd);
	stateMachine->SetEndCallback(STATE(EnemyState::HILI_HIT1), &ISkinnedMeshModelChar::OnHitAnimationEnd);

	stateMachine->SetCurrentState(STATE(initState));
}

void Hilichurl::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_WALK);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_RUN);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayAttackAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_MELEE_ATTACK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING_MELEE_ATTACK);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayHitAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_HIT_REACTION_1)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_HIT_REACTION_1);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayHit2Anim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_HIT_REACTION_2)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_HIT_REACTION_2);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlaySitAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_SIT)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_SIT);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Hilichurl::CanWalk(void) const
{
	return instance.attributes.isMoving && !instance.attributes.isAttacking;
}

bool Hilichurl::CanStopWalking() const
{
	return !instance.attributes.isMoving;
}



bool Hilichurl::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Hilichurl::CanRun(void) const
{
	return instance.attributes.isMoving && GetKeyboardPress(DIK_LSHIFT);
}

bool Hilichurl::CanHit(void) const
{
	return instance.attributes.isHit1;
}

bool Hilichurl::CanHit2(void) const
{
	return instance.attributes.isHit2;
}

void Hilichurl::OnAttackAnimationEnd(void)
{
	instance.attributes.isAttacking = false;
}

void Hilichurl::OnHitAnimationEnd(void)
{
	instance.attributes.isHit1 = false;
	instance.attributes.isHit2 = false;
	instance.attributes.hitTimer = 0;
}
