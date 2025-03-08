#pragma once
//=============================================================================
//
// ���f������ [Mitachurl.h]
// Author : 
//
//=============================================================================
#include "GameObject.h"

class Mitachurl : public GameObject<SkinnedMeshModelInstance>, public ISkinnedMeshModelChar
{
public:
	Mitachurl();
	~Mitachurl();
	void AddAnimation(char* animPath, char* animName, AnimationClipName clipName);
	void LoadWeapon(char* modelPath, char* modelName);

	void Update(void) override;
	void Draw(void) override;

	void SetupAnimationStateMachine();

	AnimationStateMachine* GetStateMachine(void) override;

	void PlayWalkAnim(void) override;
	void PlayRunAnim(void) override;
	void PlayIdleAnim(void) override;

	void PlayAttackAnim(void);

	virtual bool CanWalk(void) const override;
	virtual bool CanStopMoving() const override;
	virtual bool CanAttack() const override;
	virtual bool CanRun(void) const override;

	virtual void OnAttackAnimationEnd(void) override;
	virtual void OnDashAnimationEnd(void) override;

private:
	GameObject<SkinnedMeshModelInstance> weapon;
	AnimationStateMachine* stateMachine;
	float playAnimSpeed;
	FBXLoader& fbxLoader = FBXLoader::get_instance();
};