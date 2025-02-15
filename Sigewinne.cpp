//=============================================================================
//
// Sigewinneˆ— [Sigewinne.cpp]
// Author : 
//
//=============================================================================
#include "Sigewinne.h"
#include "input.h"

Sigewinne::Sigewinne()
{
	Instantiate("data/MODEL/enemy/Sigewinne", "Character_output.fbx", ModelType::Sigewinne);
	LoadWeapon("data/MODEL/enemy/Sigewinne", "bow.fbx");
	weapon.SetScale(XMFLOAT3(100.0f, 100.0f, 100.0f));

	AddAnimation("data/MODEL/enemy/Sigewinne/", "Breathing Idle.fbx", AnimationClipName::ANIM_IDLE);
	AddAnimation("data/MODEL/enemy/Sigewinne/", "Standing Draw Arrow.fbx", AnimationClipName::ANIM_STANDING_DRAW_ARROW);
	AddAnimation("data/MODEL/enemy/Sigewinne/", "Standing Aim Walk Forward.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_FORWARD);
	AddAnimation("data/MODEL/enemy/Sigewinne/", "Standing Aim Walk Left.fbx", AnimationClipName::ANIM_STANDING_AIM_WALK_LEFT);
	AddAnimation("data/MODEL/enemy/Sigewinne/", "Walking.fbx", AnimationClipName::ANIM_WALK);
	AddAnimation("data/MODEL/enemy/Sigewinne/", "Breakdance Uprock Var 2.fbx", AnimationClipName::ANIM_BREAKDANCE_UPROCK);

	instance.pModel->SetBodyDiffuseTexture("data/MODEL/enemy/Sigewinne/texture_0.png");
	instance.pModel->SetBodyLightMapTexture("data/MODEL/enemy/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Body_Lightmap.png");
	instance.pModel->SetBodyNormalMapTexture("data/MODEL/enemy/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Body_Normalmap.png");
	instance.pModel->SetHairDiffuseTexture("data/MODEL/enemy/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Hair_Diffuse.png");
	instance.pModel->SetHairLightMapTexture("data/MODEL/enemy/Sigewinne/Avatar_Loli_Bow_Sigewinne_Tex_Hair_Lightmap.png");
	instance.pModel->SetFaceDiffuseTexture("data/MODEL/enemy/Sigewinne/face.png");
	instance.pModel->SetFaceLightMapTexture("data/MODEL/enemy/Sigewinne/Avatar_Loli_Tex_FaceLightmap.png");
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

void Sigewinne::PlayMoveAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_WALK)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_WALK);
	instance.pModel->PlayCurrentAnim();
}

void Sigewinne::PlayJumpAnim(void)
{
}

void Sigewinne::PlayIdleAnim(void)
{
	if (instance.pModel->GetCurrentAnim() != AnimationClipName::ANIM_IDLE)
		instance.pModel->SetCurrentAnim(AnimationClipName::ANIM_IDLE);
	instance.pModel->PlayCurrentAnim();
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

	GameObject::Update();
	instance.pModel->Update();
	weapon.Update();
}

void Sigewinne::Draw(void)
{
	GameObject::Draw();
	weapon.Draw();
}
