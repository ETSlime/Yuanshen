#pragma once
//=============================================================================
//
// ÉÇÉfÉãèàóù [Sigewinne.h]
// Author : 
//
//=============================================================================
#include "GameObject.h"

class Sigewinne : public GameObject<SkinnedMeshModelInstance>, public ISkinnedMeshModelChar
{
public:
	Sigewinne();
	~Sigewinne();
	void AddAnimation(char* animPath, char* animName, AnimationClipName clipName);
	void LoadWeapon(char* modelPath, char* modelName);
	void SetCurrentAnim(AnimationClipName clipName, float startTime = 0);

	void Update(void) override;
	void Draw(void) override;

	void SetupAnimationStateMachine();

	AnimationStateMachine* GetStateMachine(void) override;

	void PlayWalkAnim(void) override;
	void PlayRunAnim(void) override;
	void PlayJumpAnim(void) override;
	void PlayIdleAnim(void) override;

	void PlayAttackAnim(void);

	virtual bool CanWalk(void) const override;
	virtual bool CanStopWalking() const override;
	virtual bool CanAttack() const override;
	virtual bool CanRun(void) const override;

	virtual void OnAttackAnimationEnd(void) override;

private:
	GameObject<SkinnedMeshModelInstance> weapon;
	AnimationStateMachine* stateMachine;
	float playAnimSpeed;
	FBXLoader& fbxLoader = FBXLoader::get_instance();
};