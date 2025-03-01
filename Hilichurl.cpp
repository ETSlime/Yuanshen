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

Hilichurl::Hilichurl()
{
	Instantiate("data/MODEL/enemy/Hilichurl", "Character_output.fbx", ModelType::Hilichurl);
	instance.collider.type = ColliderType::PLAYER;
	instance.collider.owner = this;
	CollisionManager::get_instance().RegisterCollider(&instance.collider);

	//LoadWeapon("data/MODEL/enemy/Hilichurl", "Mitsurugi.fbx");


	AddAnimation("data/MODEL/enemy/Hilichurl/", "Swing Dancing.fbx", AnimationClipName::ANIM_IDLE);

	playAnimSpeed = PLAY_ANIM_SPD;

	SetupAnimationStateMachine();
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

	if (stateMachine->GetCurrentState() == STATE(PlayerState::IDLE))
	{
		if (IsMouseLeftTriggered())
		{
			//instance.attributes.isAttacking = true;
		}
	}

	if (!GetKeyboardPress(DIK_W)
		&& !GetKeyboardPress(DIK_S)
		&& !GetKeyboardPress(DIK_A)
		&& !GetKeyboardPress(DIK_D))
		instance.attributes.isMoving = false;

	GameObject::Update();

	stateMachine->Update(ANIM_BLEND_SPD, dynamic_cast<ISkinnedMeshModelChar*>(this));

	switch (stateMachine->GetCurrentState())
	{
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
		break;
	case STATE(PlayerState::ATTACK):
		PlayAttackAnim();
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
	//jumpyDumpy.Draw();
}

AnimationStateMachine* Hilichurl::GetStateMachine(void)
{
	return stateMachine;
}

void Hilichurl::SetupAnimationStateMachine()
{
	stateMachine = new AnimationStateMachine(dynamic_cast<ISkinnedMeshModelChar*>(this));

	stateMachine->AddState(STATE(EnemyState::IDLE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_IDLE));

	//ó‘Ô‘JˆÚ
	//stateMachine->AddTransition(PlayerState::IDLE, PlayerState::WALK, &ISkinnedMeshModelChar::CanWalk);
	//stateMachine->AddTransition(PlayerState::WALK, PlayerState::DASH, &ISkinnedMeshModelChar::CanRun);
	//stateMachine->AddTransition(PlayerState::WALK, PlayerState::IDLE, &ISkinnedMeshModelChar::CanStopWalking);
	//
	//stateMachine->AddTransition(PlayerState::RUN, PlayerState::IDLE, &ISkinnedMeshModelChar::CanStopWalking);
	////stateMachine->AddTransition(PlayerState::IDLE, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	////stateMachine->AddTransition(PlayerState::WALK, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	//stateMachine->AddTransition(PlayerState::DASH, PlayerState::RUN, &ISkinnedMeshModelChar::AlwaysTrue, true);

	//stateMachine->SetEndCallback(PlayerState::ATTACK, &ISkinnedMeshModelChar::OnAttackAnimationEnd);

	stateMachine->SetCurrentState(STATE(EnemyState::IDLE));
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

void Hilichurl::PlayJumpAnim(void)
{

}

void Hilichurl::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayDashAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_DASH)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_DASH);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Hilichurl::PlayAttackAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_DRAW_ARROW)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING_DRAW_ARROW);
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

void Hilichurl::OnAttackAnimationEnd(void)
{
	instance.attributes.isAttacking = false;
}

void Hilichurl::OnDashAnimationEnd(void)
{
	instance.attributes.isDashing = false;
}