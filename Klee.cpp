//=============================================================================
//
// Kleeˆ— [Klee.cpp]
// Author : 
//
//=============================================================================
#include "Klee.h"
#include "input.h"
#include "debugproc.h"
#include "Player.h"
//*****************************************************************************
// ƒ}ƒNƒ’è‹`
//*****************************************************************************
#define PLAY_ANIM_SPD				1.0f
#define ANIM_BLEND_SPD				0.032f

#define WEAPON_SIZE						30.0f
#define JUMPYDUMPTY_SIZE				30.0f
#define BOMB_SIZE						30.0f
#define CLOVER_SIZE						30.0f
#define NORMAL_ATTACK_SIZE				30.0f
#define WEAPON_ON_HANDS_POS_OFFSET_X	25.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Y	65.0f
#define WEAPON_ON_HANDS_POS_OFFSET_Z	-5.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_X	25.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_Y	65.0f
#define WEAPON_ON_HANDS_ROT_OFFSET_Z	-5.0f
#define WEAPON_ON_BACK_POS_OFFSET_X		0.0f
#define WEAPON_ON_BACK_POS_OFFSET_Y		XM_PI
#define WEAPON_ON_BACK_POS_OFFSET_Z		0.0f

Klee::Klee()
{
	Instantiate("data/MODEL/character/Klee", "Idle.fbx", ModelType::Klee);
	jumpyDumpy.Instantiate("data/MODEL/character/Klee", "JumpyDumpty.fbx");
	jumpyDumpy.SetScale(XMFLOAT3(JUMPYDUMPTY_SIZE, JUMPYDUMPTY_SIZE, JUMPYDUMPTY_SIZE));

	bomb.Instantiate("data/MODEL/character/Klee", "bomb.fbx");
	bomb.SetScale(XMFLOAT3(BOMB_SIZE, BOMB_SIZE, BOMB_SIZE));

	normalAttack.Instantiate("data/MODEL/character/Klee", "normalAttack.fbx");
	normalAttack.SetScale(XMFLOAT3(NORMAL_ATTACK_SIZE, NORMAL_ATTACK_SIZE, NORMAL_ATTACK_SIZE));

	clover.Instantiate("data/MODEL/character/Klee", "clover.fbx");
	clover.SetScale(XMFLOAT3(CLOVER_SIZE, CLOVER_SIZE, CLOVER_SIZE));

	//clover.Instantiate("data/MODEL/Environment/", "Church.fbx");
	//clover.SetScale(XMFLOAT3(CLOVER_SIZE, CLOVER_SIZE, CLOVER_SIZE));

	LoadWeapon("data/MODEL/character/Klee", "Pulpfi.fbx");


	AddAnimation("data/MODEL/character/Klee/", "Idle.fbx", AnimationClipName::ANIM_IDLE);
	//AddAnimation("data/MODEL/character/Klee/", "Standing Draw Arrow.fbx", AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	//AddAnimation("data/MODEL/character/Klee/", "Standing Aim Walk Forward.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_FORWARD);
	//AddAnimation("data/MODEL/character/Klee/", "Standing Aim Walk Left.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_LEFT);
	//AddAnimation("data/MODEL/character/Klee/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	//AddAnimation("data/MODEL/character/Klee/", "Running.fbx", AnimationClipName::ANIM_RUN);
	//AddAnimation("data/MODEL/character/Klee/", "Breakdance Uprock Var 2.fbx", AnimationClipName::ANIM_BREAKDANCE_UPROCK);

	instance.pModel->SetBodyDiffuseTexture("data/MODEL/character/Klee/texture_0.png");
	instance.pModel->SetHairDiffuseTexture("data/MODEL/character/Klee/Avatar_Loli_Catalyst_Klee_Tex_Hair_Diffuse.png");
	instance.pModel->SetFaceDiffuseTexture("data/MODEL/character/Klee/face.png");

	playAnimSpeed = PLAY_ANIM_SPD;

	SetupAnimationStateMachine();
}

Klee::~Klee()
{
	SAFE_DELETE(stateMachine);
}

void Klee::AddAnimation(char* animPath, char* animName, AnimationClipName clipName)
{
	if (instance.pModel)
		fbxLoader.LoadAnimation(renderer.GetDevice(), *instance.pModel, animPath, animName, clipName);
}

void Klee::LoadWeapon(char* modelPath, char* modelName)
{
	weapon.Instantiate(modelPath, modelName, ModelType::Weapon);
	weapon.SetScale(XMFLOAT3(WEAPON_SIZE, WEAPON_SIZE, WEAPON_SIZE));
	//weapon.SetPosition(XMFLOAT3(WEAPON_ON_HANDS_POS_OFFSET_X, WEAPON_ON_HANDS_POS_OFFSET_Y, WEAPON_ON_HANDS_POS_OFFSET_Z));
	//weapon.SetRotation(XMFLOAT3(WEAPON_ON_BACK_POS_OFFSET_X, WEAPON_ON_BACK_POS_OFFSET_Y, WEAPON_ON_BACK_POS_OFFSET_Z));
}

void Klee::SetCurrentAnim(AnimationClipName clipName, float startTime)
{
	instance.pModel->SetCurrentAnim(clipName, startTime);
}

void Klee::Update(void)
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

	jumpyDumpy.Update();
	bomb.Update();
	normalAttack.Update();
	clover.Update();
}

void Klee::Draw(void)
{
	GameObject::Draw();
	clover.Draw();
	//jumpyDumpy.Draw();
}

AnimationStateMachine* Klee::GetStateMachine(void)
{
	return stateMachine;
}

void Klee::SetupAnimationStateMachine()
{
	stateMachine = new AnimationStateMachine(dynamic_cast<ISkinnedMeshModelChar*>(this));

	stateMachine->AddState(STATE(PlayerState::IDLE), instance.pModel->GetAnimationClip(AnimationClipName::ANIM_IDLE));
	//stateMachine->AddState(PlayerState::WALK, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_WALK));
	//stateMachine->AddState(PlayerState::RUN, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_RUN));
	//stateMachine->AddState(PlayerState::ATTACK, instance.pModel->GetAnimationClip(AnimationClipName::ANIM_STANDING_DRAW_ARROW));

	//ó‘Ô‘JˆÚ
	//stateMachine->AddTransition(PlayerState::IDLE, PlayerState::WALK, &ISkinnedMeshModelChar::CanWalk);
	//stateMachine->AddTransition(PlayerState::WALK, PlayerState::RUN, &ISkinnedMeshModelChar::CanRun);
	//stateMachine->AddTransition(PlayerState::WALK, PlayerState::IDLE, &ISkinnedMeshModelChar::CanStopWalking);
	//stateMachine->AddTransition(PlayerState::RUN, PlayerState::IDLE, &ISkinnedMeshModelChar::CanStopWalking);
	//stateMachine->AddTransition(PlayerState::IDLE, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	//stateMachine->AddTransition(PlayerState::WALK, PlayerState::ATTACK, &ISkinnedMeshModelChar::CanAttack);
	//stateMachine->AddTransition(PlayerState::ATTACK, PlayerState::IDLE, &ISkinnedMeshModelChar::AlwaysTrue, true);

	//stateMachine->SetEndCallback(PlayerState::ATTACK, &ISkinnedMeshModelChar::OnAttackAnimationEnd);

	stateMachine->SetCurrentState(STATE(PlayerState::IDLE));
}

void Klee::PlayWalkAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_WALK);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Klee::PlayRunAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_RUN)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_RUN);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Klee::PlayJumpAnim(void)
{

}

void Klee::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

void Klee::PlayAttackAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_STANDING_DRAW_ARROW)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	instance.pModel->PlayCurrentAnim(playAnimSpeed);
}

bool Klee::CanWalk(void) const
{
	return instance.attributes.isMoving && !instance.attributes.isAttacking;
}

bool Klee::CanStopWalking() const
{
	return !instance.attributes.isMoving;
}

bool Klee::CanAttack() const
{
	return instance.attributes.isAttacking;
}

bool Klee::CanRun(void) const
{
	return instance.attributes.isMoving && GetKeyboardPress(DIK_LSHIFT);
}

void Klee::OnAttackAnimationEnd(void)
{
	instance.attributes.isAttacking = false;
}