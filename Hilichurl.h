#pragma once
//=============================================================================
//
// ÉÇÉfÉãèàóù [Hilichurl.h]
// Author : 
//
//=============================================================================
#include "Enemy.h"

class Hilichurl : public Enemy, public ISkinnedMeshModelChar
{
public:
	Hilichurl(Transform transform, EnemyState initState = EnemyState::IDLE);
	~Hilichurl();
	void AddAnimation(char* animPath, char* animName, AnimationClipName clipName);
	void LoadWeapon(char* modelPath, char* modelName);
	void SetCurrentAnim(AnimationClipName clipName, float startTime = 0);

	void Update(void) override;
	void Draw(void) override;

	void SetupAnimationStateMachine(EnemyState initState);

	AnimationStateMachine* GetStateMachine(void) override;

	void PlayWalkAnim(void) override;
	void PlayRunAnim(void) override;
	void PlayIdleAnim(void) override;

	void PlayAttackAnim(void) override;
	void PlayHitAnim(void) override;
	void PlayHit2Anim(void);
	void PlaySitAnim(void);

	virtual bool CanWalk(void) const override;
	virtual bool CanStopWalking() const override;
	virtual bool CanAttack() const override;
	virtual bool CanRun(void) const override;
	virtual bool CanHit(void) const override;
	virtual bool CanHit2(void) const override;
	virtual void OnAttackAnimationEnd(void) override;
	virtual void OnHitAnimationEnd(void) override;

private:
	GameObject<SkinnedMeshModelInstance> weapon;
	AnimationStateMachine* stateMachine;
	float playAnimSpeed;
	FBXLoader& fbxLoader = FBXLoader::get_instance();
};