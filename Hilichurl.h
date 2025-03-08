#pragma once
//=============================================================================
//
// モデル処理 [Hilichurl.h]
// Author : 
//
//=============================================================================
#include "Enemy.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define HILI_VIEW_ANGLE					(XM_PI * 0.3f)
#define HILI_VIEW_DISTANCE				(800.0f)
#define HILI_CHASING_RANGE				(1500.0f)
#define HILI_ATTACK_RANGE				(210.0f)
#define HILI_MAX_ATTACK_STEP			(520.0f)
#define HILI_MIN_ATTACK_CDTIME			(180.0f)
#define HILI_MAX_ATTACK_CDTIME			(360.0f)
#define	HILI_MAX_COOLDOWN_MOVE_DISTANCE	(70.0f)
#define HILI_MIN_COOLDOWN_MOVE_DISTANCE	(35.0f)
#define	HILI_MAX_COOLDOWN_WAIT_TIME		(0.4f)
#define HILI_MIN_COOLDOWN_WAIT_TIME		(0.2f)
#define HILICHURL_SIZE					XMFLOAT3(0.9f, 0.9f, 0.9f)

#define HILI_MAX_HP					(50.0f)

class Hilichurl : public Enemy, public ISkinnedMeshModelChar
{
public:
	Hilichurl(Transform transform, EnemyState initState = EnemyState::IDLE);
	~Hilichurl();
	void AddAnimation(char* animPath, char* animName, AnimationClipName clipName);
	void LoadWeapon(char* modelPath, char* modelName);

	void Update(void) override;
	void Draw(void) override;

	void UpdateWeapon(void);

	void SetupAnimationStateMachine(EnemyState initState);

	AnimationStateMachine* GetStateMachine(void) override;

	void PlayWalkAnim(void) override;
	void PlayRunAnim(void) override;
	void PlayIdleAnim(void) override;

	void PlayAttackAnim(void) override;
	void PlayHitAnim(void) override;
	void PlayHit2Anim(void);
	void PlaySitAnim(void);
	void PlaySurprisedAnim(void);
	void PlayDieAnim(void);
	void PlayDanceAnim(void);

	virtual virtual void Initialize(void) override;

	virtual bool CanWalk(void) const override;
	virtual bool CanStopMoving() const override;
	virtual bool CanStopRunning() const override;
	virtual bool CanAttack() const override;
	virtual bool CanRun(void) const override;
	virtual bool CanHit(void) const override;
	virtual bool CanHit2(void) const override;
	virtual bool CanSurprised(void) const override;
	virtual bool CanDie(void) const override;
	virtual void OnAttackAnimationEnd(void) override;
	virtual void OnHitAnimationEnd(void) override;
	virtual void OnSurprisedEnd(void) override;
	virtual void OnDieAnimationEnd() override;

private:
	GameObject<SkinnedMeshModelInstance> weapon;
	AnimationStateMachine* stateMachine;
	float playAnimSpeed;
	FBXLoader& fbxLoader = FBXLoader::get_instance();
};