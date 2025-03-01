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

Lumine::Lumine()
{
	Instantiate("data/MODEL/character/Lumine", "Character_output.fbx", ModelType::Lumine);
	instance.collider.type = ColliderType::PLAYER;
	instance.collider.owner = this;
	CollisionManager::get_instance().RegisterCollider(&instance.collider);

	LoadWeapon("data/MODEL/character/Lumine", "Mitsurugi.fbx");


	AddAnimation("data/MODEL/character/Lumine/", "Idle.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/character/Lumine/", "Sneak Walk.fbx", AnimationClipName::ANIM_DASH);
	AddAnimation("data/MODEL/character/Lumine/", "Running.fbx", AnimationClipName::ANIM_RUN);
	//AddAnimation("data/MODEL/character/Lumine/", "Standing Aim Walk Left.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_LEFT);
	AddAnimation("data/MODEL/character/Lumine/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	//AddAnimation("data/MODEL/character/Lumine/", "Running.fbx", AnimationClipName::ANIM_RUN);
	//AddAnimation("data/MODEL/character/Lumine/", "Breakdance Uprock Var 2.fbx", AnimationClipName::ANIM_BREAKDANCE_UPROCK);

	instance.pModel->SetBodyDiffuseTexture("data/MODEL/character/Lumine/texture_0.png");
	instance.pModel->SetHairDiffuseTexture("data/MODEL/character/Lumine/hair.png");
	instance.pModel->SetFaceDiffuseTexture("data/MODEL/character/Lumine/face.png");

	playAnimSpeed = PLAY_ANIM_SPD;

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
	//weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
	//weapon.SetRotation(XMFLOAT3(WEAPON_ON_BACK_POS_OFFSET_X, WEAPON_ON_BACK_POS_OFFSET_Y, WEAPON_ON_BACK_POS_OFFSET_Z));
}

void Lumine::SetCurrentAnim(AnimationClipName clipName, float startTime)
{
	instance.pModel->SetCurrentAnim(clipName, startTime);
}

void Lumine::Update(void)
{
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

	weapon.Update();

	//XMMATRIX weaponMtx = instance.pModel->GetWeaponTransformMtx();
	//weaponMtx = XMMatrixMultiply(weapon.GetWorldMatrix(), weaponMtx);
	//weaponMtx = XMMatrixMultiply(weaponMtx, instance.transform.mtxWorld);

	//weapon.SetWorldMatrix(weaponMtx);

}

void Lumine::Draw(void)
{
	GameObject::Draw();
	weapon.Draw();
	//jumpyDumpy.Draw();
}

AnimationStateMachine* Lumine::GetStateMachine(void)
{
	return stateMachine;
}

void Lumine::SetupAnimationStateMachine()
{
	stateMachine = new AnimationStateMachine(dynamic_cast<ISkinnedMeshModelChar*>(this));

	stateMachine->AddState(STATE(PlayerState::IDLE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_IDLE));
	stateMachine->AddState(STATE(PlayerState::WALK), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_WALK));
	stateMachine->AddState(STATE(PlayerState::RUN), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_RUN));
	stateMachine->AddState(STATE(PlayerState::DASH), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_DASH));

	//ó‘Ô‘JˆÚ
	stateMachine->AddTransition(STATE(PlayerState::IDLE), STATE(PlayerState::WALK), &ISkinnedMeshModelChar::CanWalk);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::DASH), &ISkinnedMeshModelChar::CanRun);
	stateMachine->AddTransition(STATE(PlayerState::WALK), STATE(PlayerState::IDLE), &ISkinnedMeshModelChar::CanStopWalking);
	
	stateMachine->AddTransition(STATE(PlayerState::RUN), STATE(PlayerState::IDLE), &ISkinnedMeshModelChar::CanStopWalking);
	//stateMachine->AddTransition(PlayerState::IDLE, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	//stateMachine->AddTransition(PlayerState::WALK, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(STATE(PlayerState::DASH), STATE(PlayerState::RUN), &ISkinnedMeshModelChar::AlwaysTrue, true);

	//stateMachine->SetEndCallback(PlayerState::ATTACK, &ISkinnedMeshModelChar::OnAttackAnimationEnd);

	stateMachine->SetCurrentState(STATE(PlayerState::IDLE));
}

void Lumine::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_WALK);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_RUN);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayJumpAnim(void)
{

}

void Lumine::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayDashAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_DASH)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_DASH);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Lumine::PlayAttackAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_DRAW_ARROW)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Lumine::CanWalk(void) const
{
	return instance.attributes.isMoving && !instance.attributes.isAttacking;
}

bool Lumine::CanStopWalking() const
{
	return !instance.attributes.isMoving;
}

bool Lumine::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Lumine::CanRun(void) const
{
	return instance.attributes.isMoving && GetKeyboardPress(DIK_LSHIFT);
}

void Lumine::OnAttackAnimationEnd(void)
{
	instance.attributes.isAttacking = false;
}

void Lumine::OnDashAnimationEnd(void)
{
	instance.attributes.isDashing = false;
}