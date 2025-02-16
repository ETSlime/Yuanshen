#pragma once
//=============================================================================
//
// ÉÇÉfÉãèàóù [Sigewinne.h]
// Author : 
//
//=============================================================================
#include "GameObject.h"

class Sigewinne : public GameObject<SkinnedMeshModelInstance>, public ISkinnedMeshModel
{
public:
	Sigewinne();
	~Sigewinne();
	void AddAnimation(char* animPath, char* animName, AnimationClipName clipName);
	void LoadWeapon(char* modelPath, char* modelName);
	void SetCurrentAnim(AnimationClipName clipName, float startTime = 0);

	void PlayWalkAnim(void) override;
	void PlayRunAnim(void) override;
	void PlayJumpAnim(void) override;
	void PlayIdleAnim(void) override;

	void Update(void) override;
	void Draw(void) override;

	AnimationStateMachine* GetStateMachine(void) override;

private:
	GameObject<SkinnedMeshModelInstance> weapon;
	AnimationStateMachine stateMachine;
	float playAnimSpeed;
	FBXLoader& fbxLoader = FBXLoader::get_instance();
};