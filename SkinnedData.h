#pragma once

//=============================================================================
//
// SkinnedData [SkinnedData.h]
// Author : 
//
//=============================================================================
#include "main.h"
#include "SimpleArray.h"
#include "HashMap.h"
//*****************************************************************************
// ç\ë¢ëÃíËã`
//*****************************************************************************
struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

	void Interpolate(float t, XMFLOAT4X4& M)const;

	SimpleArray<Keyframe> Keyframes;

};

struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

	void Interpolate(float t, SimpleArray<XMFLOAT4X4>& boneTransforms) const;

	SimpleArray<BoneAnimation> BoneAnimations;
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(char* clipName) const;
	float GetClipEndTime(char* clipName) const;

	void Set(SimpleArray<int>& boneHierarchy,
		SimpleArray<XMFLOAT4X4>& boneOffsets,
		HashMap<char*, AnimationClip, CharPtrHash, CharPtrEquals>& animations);

	void GetFinalTransforms(char* clipName, float timePos,
		SimpleArray<XMFLOAT4X4>& finalTransforms) const;

private:
	SimpleArray<int> mBoneHierarchy;

	SimpleArray<XMFLOAT4X4> mBoneOffsets;

	HashMap<char*, AnimationClip, CharPtrHash, CharPtrEquals> mAnimations;
};
