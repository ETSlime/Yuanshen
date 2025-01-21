#include "SkinnedData.h"

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f){}

Keyframe::~Keyframe()
{
}

float BoneAnimation::GetStartTime()const
{
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	return Keyframes.back().TimePos;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	if (t <= Keyframes.front().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if (t >= Keyframes.back().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for (UINT i = 0; i < Keyframes.getSize() - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime()const
{
	float t = FLT_MAX;
	for (UINT i = 0; i < BoneAnimations.getSize(); ++i)
	{
		t = min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	float t = 0.0f;
	for (UINT i = 0; i < BoneAnimations.getSize(); ++i)
	{
		t = max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, SimpleArray<XMFLOAT4X4>& boneTransforms) const
{
	for (UINT i = 0; i < BoneAnimations.getSize(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(char* clipName) const
{
	auto clip = mAnimations.search(clipName);
	return clip->GetClipStartTime();
}

float SkinnedData::GetClipEndTime(char* clipName) const
{
	auto clip = mAnimations.search(clipName);
	return clip->GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
	return mBoneHierarchy.getSize();
}

void SkinnedData::Set(SimpleArray<int>& boneHierarchy,
	SimpleArray<XMFLOAT4X4>& boneOffsets,
	HashMap<char*, AnimationClip, CharPtrHash, CharPtrEquals>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets = boneOffsets;
	mAnimations = animations;
}

void SkinnedData::GetFinalTransforms(char* clipName, float timePos,
	SimpleArray<XMFLOAT4X4>& finalTransforms) const
{
	UINT numBones = mBoneOffsets.getSize();

	SimpleArray<XMFLOAT4X4> toParentTransforms(numBones);

	// Interpolate all the bones of this clip at the given time instance.
	auto clip = mAnimations.search(clipName);
	clip->Interpolate(timePos, toParentTransforms);

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	SimpleArray<XMFLOAT4X4> toRootTransforms(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// Premultiply by the bone offset transform to get the final transform.
	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}