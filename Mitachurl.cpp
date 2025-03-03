//=============================================================================
//
// Mitachurlˆ— [Mitachurl.cpp]
// Author : 
//
//=============================================================================
#include "Mitachurl.h"
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

Mitachurl::Mitachurl()
{
	Instantiate("data/MODEL/enemy/Mitachurl", "Character_output.fbx", ModelType::Mitachurl);

	//LoadWeapon("data/MODEL/enemy/Mitachurl", "Mitsurugi.fbx");

	AddAnimation("data/MODEL/enemy/Mitachurl/", "Orc Idle.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/enemy/Mitachurl/", "Orc Idle.fbx", AnimationClipName::ANIM_IDLE);

	playAnimSpeed = PLAY_ANIM_SPD;

	SetupAnimationStateMachine();
}

Mitachurl::~Mitachurl()
{
	SAFE_DELETE(stateMachine);
}

void Mitachurl::AddAnimation(char* animPath, char* animName, AnimationClipName clipName)
{
	if (instance.pModel)
		fbxLoader.LoadAnimation(renderer.GetDevice(), *instance.pModel, animPath, animName, clipName);
}

void Mitachurl::LoadWeapon(char* modelPath, char* modelName)
{
	weapon.Instantiate(modelPath, modelName, ModelType::Weapon);
	weapon.SetScale(XMFLOAT3(WEAPON_SIZE, WEAPON_SIZE, WEAPON_SIZE));
	//weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
	//weapon.SetRotation(XMFLOAT3(WEAPON_ON_BACK_POS_OFFSET_X, WEAPON_ON_BACK_POS_OFFSET_Y, WEAPON_ON_BACK_POS_OFFSET_Z));
}

void Mitachurl::SetCurrentAnim(AnimationClipName clipName, float startTime)
{
	instance.pModel->SetCurrentAnim(clipName, startTime);
}

void Mitachurl::Update(void)
{
	if (GetKeyboardPress(DIK_UP))
	{
		instance.transform.pos.y--;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{
		instance.transform.pos.y++;
	}

	if (GetKeyboardPress(DIK_LEFT))
	{
		instance.transform.rot.y += 0.03f;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{
		instance.transform.rot.y -= 0.03f;
	}

	if (stateMachine->GetCurrentState() == STATE(PlayerState::IDLE))
	{
		if (IsMouseLeftTriggered())
		{
			instance.attributes.isAttacking = true;
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
		break;
	case STATE(PlayerState::ATTACK_1):
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

void Mitachurl::Draw(void)
{
	GameObject::Draw();
	//weapon.Draw();
	//jumpyDumpy.Draw();
}

AnimationStateMachine* Mitachurl::GetStateMachine(void)
{
	return stateMachine;
}

void Mitachurl::SetupAnimationStateMachine()
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

void Mitachurl::PlayStandingAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Mitachurl::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_WALK);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Mitachurl::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_RUN);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Mitachurl::PlayJumpAnim(void)
{

}

void Mitachurl::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Mitachurl::PlayDashAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_DASH)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_DASH);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Mitachurl::PlayAttackAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_DRAW_ARROW)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Mitachurl::CanWalk(void) const
{
	return instance.attributes.isMoving && !instance.attributes.isAttacking;
}

bool Mitachurl::CanStopWalking() const
{
	return !instance.attributes.isMoving;
}

bool Mitachurl::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Mitachurl::CanRun(void) const
{
	return instance.attributes.isMoving && GetKeyboardPress(DIK_LSHIFT);
}

void Mitachurl::OnAttackAnimationEnd(void)
{
	instance.attributes.isAttacking = false;
}

void Mitachurl::OnDashAnimationEnd(void)
{
	instance.attributes.isDashing = false;
}