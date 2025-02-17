//=============================================================================
//
// Sigewinneˆ— [Sigewinne.cpp]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"
#include "input.h"
#include "debugproc.h"
#include "Player.h"
//*****************************************************************************
// ƒ}ƒNƒ’è‹`
//*****************************************************************************
#define PLAY_ANIM_SPD		1.0f
#define ANIM_BLEND_SPD		0.032f
Sigewinne::Sigewinne()
{
	Instantiate("data/MODEL/character/Sigewinne", "Character_output.fbx", ModelType::Sigewinne);
	LoadWeapon("data/MODEL/character/Sigewinne", "bow.fbx");
	weapon.SetScale(XMFLOAT3(100.0f, 100.0f, 100.0f));

	AddAnimation("data/MODEL/character/Sigewinne/", "Breathing Idle.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/character/Sigewinne/", "Standing Draw Arrow.fbx", AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	AddAnimation("data/MODEL/character/Sigewinne/", "Standing Aim Walk Forward.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_FORWARD);
	AddAnimation("data/MODEL/character/Sigewinne/", "Standing Aim Walk Left.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_LEFT);
	AddAnimation("data/MODEL/character/Sigewinne/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	AddAnimation("data/MODEL/character/Sigewinne/", "Running.fbx", AnimationClipName::ANIM_RUN);
	AddAnimation("data/MODEL/character/Sigewinne/", "Breakdance Uprock Var 2.fbx", AnimationClipName::ANIM_BREAKDANCE_UPROCK);

	instance.pModel->SetBodyDiffuseTexture("data/MODEL/character/Sigewinne/texture_0.png");
	instance.pModel->SetBodyLightMapTexture("data/MODEL/character/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Body_Lightmap.png");
	instance.pModel->SetBodyNormalMapTexture("data/MODEL/character/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Body_Normalmap.png");
	instance.pModel->SetHairDiffuseTexture("data/MODEL/character/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Hair_Diffuse.png");
	instance.pModel->SetHairLightMapTexture("data/MODEL/character/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Hair_Lightmap.png");
	instance.pModel->SetFaceDiffuseTexture("data/MODEL/character/Sigewinne/face.png");
	instance.pModel->SetFaceLightMapTexture("data/MODEL/character/Sigewinne/Avatar_Loli_Tex_FaceLightmap.png");

	playAnimSpeed = PLAY_ANIM_SPD;

	SetupAnimationStateMachine();
}

Sigewinne::~Sigewinne()
{

}

void Sigewinne::AddAnimation(char* animPath, char* animName, AnimationClipName clipName)
{
	if (instance.pModel)
		fbxLoader.LoadAnimation(renderer.GetDevice(), *instance.pModel, animPath, animName, clipName);
}

void Sigewinne::LoadWeapon(char* modelPath, char* modelName)
{
	weapon.Instantiate(modelPath, modelName, ModelType::Weapon);
}

void Sigewinne::SetCurrentAnim(AnimationClipName clipName, float startTime)
{
	instance.pModel->SetCurrentAnim(clipName, startTime);
}

void Sigewinne::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_WALK);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Sigewinne::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_RUN);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Sigewinne::PlayJumpAnim(void)
{

}

void Sigewinne::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Sigewinne::PlayAttackAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_DRAW_ARROW)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Sigewinne::CanWalk(void) const
{
	return instance.attributes.isMoving && !instance.attributes.isAttacking;
}

bool Sigewinne::CanStopWalking() const
{
	return !instance.attributes.isMoving;
}

bool Sigewinne::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Sigewinne::CanRun(void) const
{
	return instance.attributes.isMoving && GetKeyboardPress(DIK_LSHIFT);
}

void Sigewinne::OnAttackAnimationEnd(void)
{
	instance.attributes.isAttacking = false;
}

void Sigewinne::Update(void)
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

	if (stateMachine->GetCurrentState() == PlayerState::IDLE)
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
	case PlayerState::IDLE:
		PlayIdleAnim();
		break;
	case PlayerState::WALK:
		PrintDebugProc("dsdfds\n");
		PlayWalkAnim();
		break;
	case PlayerState::RUN:
		PlayRunAnim();
		break;
	case PlayerState::JUMP:
		break;
	case PlayerState::ATTACK:
		PlayAttackAnim();
		break;
	default:
		break;
	}

	instance.pModel->UpdateBoneTransform(stateMachine->GetBoneMatrices());
	weapon.Update();
	//weapon.instance.pModel->UpdateBoneTransform(nullptr);
}

void Sigewinne::Draw(void)
{
	GameObject::Draw();
	weapon.Draw();
}

AnimationStateMachine* Sigewinne::GetStateMachine(void)
{
	return stateMachine;
}

void Sigewinne::SetupAnimationStateMachine()
{
	stateMachine = new AnimationStateMachine(dynamic_cast<ISkinnedMeshModelChar*>(this));

	stateMachine->AddState(PlayerState::IDLE, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_IDLE));
	stateMachine->AddState(PlayerState::WALK, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_WALK));
	stateMachine->AddState(PlayerState::RUN, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_RUN));
	stateMachine->AddState(PlayerState::ATTACK, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_STANDING_DRAW_ARROW));

	//ó‘Ô‘JˆÚ
	stateMachine->AddTransition(PlayerState::IDLE, PlayerState::WALK, &ISkinnedMeshModelChar::CanWalk);
	stateMachine->AddTransition(PlayerState::WALK, PlayerState::RUN, &ISkinnedMeshModelChar::CanRun);
	stateMachine->AddTransition(PlayerState::WALK, PlayerState::IDLE, &ISkinnedMeshModelChar::CanStopWalking);
	stateMachine->AddTransition(PlayerState::RUN, PlayerState::IDLE, &ISkinnedMeshModelChar::CanStopWalking);
	stateMachine->AddTransition(PlayerState::IDLE, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(PlayerState::WALK, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	stateMachine->AddTransition(PlayerState::ATTACK, PlayerState::IDLE, &ISkinnedMeshModelChar::AlwaysTrue, true);

	stateMachine->SetEndCallback(PlayerState::ATTACK, &ISkinnedMeshModelChar::OnAttackAnimationEnd);

	stateMachine->SetCurrentState(PlayerState::IDLE);
}
