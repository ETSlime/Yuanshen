#pragma once
//=============================================================================
//
// ÉÇÉfÉãèàóù [Sigewinne.h]
// Author : 
//
//=============================================================================
#include "GameObject.h"

class Sigewinne : public GameObject<SkinnedMeshModelInstance>
{
public:
	Sigewinne();
	~Sigewinne();
	void AddAnimation(char* animPath, char* animName, AnimationClipName clipName);
	void LoadWeapon(char* modelPath, char* modelName);
	void SetCurrentAnim(AnimationClipName clipName, float startTime = 0);

	void PlayMoveAnim(void);
	void PlayJumpAnim(void);
	void PlayIdleAnim(void);

	void Update(void) override;
	void Draw(void) override;

private:
	GameObject<SkinnedMeshModelInstance> weapon;
	FBXLoader& fbxLoader = FBXLoader::get_instance();
};