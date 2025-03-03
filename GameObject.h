#pragma once

//=============================================================================
//
// GameObject処理 [GameObject.h]
// Author : 
//
//=============================================================================
#include "renderer.h"
#include "model.h"
#include "SkinnedMeshModel.h"
#include "CollisionManager.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
enum class ActionEnum
{
	NONE,
	ATTACK,
};
//*****************************************************************************
// 構造体定義
//*****************************************************************************

struct Transform
{
	XMMATRIX			mtxWorld;			// ワールドマトリックス
	XMFLOAT3			pos;				// モデルの位置
	XMFLOAT3			rot;				// モデルの向き(回転)
	XMFLOAT3			scl;				// モデルの大きさ(スケール)

	Transform()
	{
		pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		mtxWorld = XMMatrixIdentity();
	}
};

struct Attributes
{
	XMFLOAT4X4		mtxWorld;	// ワールドマトリックス

	bool			load;

	float			spd;		// 移動スピード
	float			dir;		// 向き
	float			size;		// 当たり判定の大きさ
	bool			use;

	float			targetDir;

	bool			stopRun;
	bool			isMoving;
	bool			isAttacking;
	bool			isAttacking2;
	bool			isAttacking3;
	bool			isDashing;
	bool			isJumping;
	bool			isGrounded;
	bool			isMoveBlocked;
	bool			isHit1;
	bool			isHit2;
	int				hitTimer;

	bool			attackWindow2;
	bool			attackWindow3;
	int				attackWinwdowCnt;

	Attributes()
	{
		load = TRUE;
		spd = 0.0f;
		dir = 0.0f;
		size = 0.0f;
		use = TRUE;
		targetDir = 0.0f;
		stopRun = TRUE;
		isMoving = FALSE;
		isAttacking = FALSE;
		isAttacking2 = FALSE;
		isAttacking3 = FALSE;
		isDashing = FALSE;
		isJumping = FALSE;
		isGrounded = FALSE;
		isMoveBlocked = FALSE;
		isHit1 = FALSE;
		isHit2 = FALSE;
		hitTimer = 0;

		attackWindow2 = FALSE;
		attackWindow3 = FALSE;
		attackWinwdowCnt = 0;
	}
};

struct SkinnedMeshModelInstance
{
	char*				modelPath;
	char*				modelName;
	char*				modelFullPath;
	bool				use;
	bool				load;
	bool				renderShadow;
	SkinnedMeshModel*	pModel;
	Transform			transform;

	BOOL			isSelected;
	BOOL			isCursorIn;
	int				editorIdx;
	Attributes		attributes;
	Collider		collider;

	RenderProgressBuffer renderProgress;
};

struct ModelInstance
{
	char*				modelPath;
	int					state;
	bool				use;
	bool				load;
	bool				renderShadow;
	Model*				pModel;				// モデル情報
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// モデルの色
	Transform			transform;

	Attributes		attributes;
	Collider		collider;

	BOOL			isSelected;
	BOOL			isCursorIn;
	int				editorIdx;

	RenderProgressBuffer renderProgress;
};


template<typename T>
class GameObject
{
public:
	GameObject();
	~GameObject();
	void Instantiate(char* modelPath);
	void Instantiate(char* modelPath, char* modelName, ModelType modelType = ModelType::Default, 
		AnimationClipName clipName = AnimationClipName::ANIM_NONE);
	virtual void Update();
	virtual void Draw();
	void DrawModelEditor();
	void UpdateModelEditor();

	inline void SetPosition(XMFLOAT3 pos) { instance.transform.pos = pos; }
	inline void SetRotation(XMFLOAT3 rot) { instance.transform.rot = rot; }
	inline void SetScale(XMFLOAT3 scl) { instance.transform.scl = scl; }
	inline void SetTransform(Transform transform) { instance.transform = transform; }
	inline void SetWorldMatrix(XMMATRIX mtxWorld) { instance.transform.mtxWorld = mtxWorld; }
	inline Transform GetTransform() { return instance.transform; }
	inline XMMATRIX	GetWorldMatrix() { return instance.transform.mtxWorld; }
	inline const T* GetInstance() const { return &instance; }
	inline Model* GetModel() { return instance.pModel; }
	inline SkinnedMeshModel* GetSkinnedMeshModel() { return instance.pModel; }
	inline XMFLOAT4* GetDiffuse() { return instance.diffuse; }
	inline bool GetUse() { return instance.use; }
	inline void SetUse(bool use) { instance.use = use; }
	inline bool GetRenderShadow() { return instance.renderShadow; }
	inline void SetRenderShadow(bool render) { instance.renderShadow = render; }
	inline BOOL GetIsModelSelected() { return instance.isSelected; }
	inline BOOL GetIsCursorIn() { return instance.isCursorIn; }
	inline void SetIsCursorIn(BOOL cursorIn) { instance.isCursorIn = cursorIn; }
	inline int GetEditorIndex() { return instance.editorIdx; }
	inline void SetEditorIndex(int idx) { instance.editorIdx = idx; }
	inline Attributes GetAttributes() { return instance.attributes; }
	inline void UpdateAttributes(Attributes attributes) { instance.attributes = attributes; }
	inline void SetDrawBoundingBox(bool draw) { instance.pModel.SetDrawBoundingBox(draw); }
	inline void SetColliderType(ColliderType type) { instance.collider.type = type; }
	inline void SetColliderOwner(void* owner) { instance.collider.owner = owner; }
	inline void SetColliderEnable(bool enable) { instance.collider.enable = enable; }
	inline void SetColliderBoundingBox(BOUNDING_BOX boundingBox) { instance.collider.aabb = boundingBox; }
	inline const Collider& GetCollider(void) { return instance.collider; }
	inline void SetGrounded(bool grounded) { instance.attributes.isGrounded = grounded; }
	inline void SetMoveBlock(bool blocked) { instance.attributes.isMoveBlocked = blocked; }
	inline void SetIsHit(bool isHit) { instance.attributes.isHit1 = isHit; }
	inline bool GetIsHit(void) { return instance.attributes.isHit1; }
	inline void SetIsHit2(bool isHit) { instance.attributes.isHit2 = isHit; }
	inline bool GetIsHit2(void) { return instance.attributes.isHit2; }
	inline int	GetHitTimer(void) { return instance.attributes.hitTimer; }
	inline void SetHitTimer(int hitTimer) { instance.attributes.hitTimer = hitTimer; }
	virtual AnimationStateMachine* GetStateMachine() { return nullptr; }

	void SetBoundingboxSize(XMFLOAT3 size)
	{
		UINT numBones = instance.pModel.GetNumBones();
		for (int i = 0; i < numBones; i++)
		{
			instance.pModel.SetBoundingBoxSize(size, i);
		}
	}

	Renderer& renderer = Renderer::get_instance();

protected:
	T instance;
};

class ISkinnedMeshModelChar
{
public:
	virtual void PlayWalkAnim() {};
	virtual void PlayRunAnim() {};
	virtual void PlayIdleAnim() {};
	virtual void PlayStandingAnim() {};
	virtual void PlayAttackAnim() {};
	virtual void PlayHitAnim() {};
	virtual void PlayDashAnim() {}
	virtual void PlayJumpAnim() {}

	virtual bool CanWalk() const = 0;
	virtual bool CanStopWalking() const = 0;
	virtual bool CanAttack() const = 0;
	virtual bool CanAttack2() const  { return false; }
	virtual bool CanAttack3() const  { return false; }
	virtual bool CanRun()	const = 0;
	virtual bool CanHit()	const = 0;
	virtual bool CanHit2()	const { return false; }
	virtual bool CanJump()	const { return false; }
	virtual void OnAttackAnimationEnd() {};
	virtual void OnHitAnimationEnd() {};
	virtual void OnDashAnimationEnd() {}
	virtual void OnJumpAnimationEnd() {}
	virtual bool ExecuteAction(ActionEnum action) { return true; }
	bool AlwaysTrue() const { return true; }
};


template <typename T>
GameObject<T>::GameObject()
{
	instance.pModel = nullptr;
	instance.isSelected = FALSE;
	instance.isCursorIn = FALSE;
	instance.editorIdx = -1;
	instance.renderShadow = true;
	instance.renderProgress.progress = 0.0f;
	instance.renderProgress.isRandomFade = TRUE;
}

template <typename T>
GameObject<T>::~GameObject()
{
	if (instance.pModel)
	{
		instance.load = false;
		MODEL_POOL* modelPool = Model::GetModel(instance.modelPath);
		if (modelPool)
		{
			modelPool->count--;
			if (modelPool->count == 0)
			{
				delete modelPool->pModel;
				Model::RemoveModel(instance.modelPath);
			}
		}
	}
}

template <>
GameObject<SkinnedMeshModelInstance>::~GameObject();

template <typename T>
void GameObject<T>::Instantiate(char* modelPath)
{
	instance.modelPath = modelPath;
	instance.pModel = Model::StoreModel(modelPath);
	instance.load = true;
	instance.use = true;
}

template <>
void GameObject<SkinnedMeshModelInstance>::Instantiate(char* modelPath, char* modelName, 
	ModelType modelType, AnimationClipName clipName);

template<>
void GameObject< SkinnedMeshModelInstance>::Update();

template <typename T>
void GameObject<T>::Update()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(instance.transform.scl.x, instance.transform.scl.y, instance.transform.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(instance.transform.rot.x, instance.transform.rot.y + XM_PI, instance.transform.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	instance.transform.mtxWorld = mtxWorld;

	XMFLOAT3 worldPos1, worldPos2;

	XMVECTOR localAABBMax = XMVectorSet(
		instance.pModel->GetBoundingBox().maxPoint.x,
		instance.pModel->GetBoundingBox().maxPoint.y,
		instance.pModel->GetBoundingBox().maxPoint.z,
		1.0f
	);

	XMVECTOR worldPosMax = XMVector3Transform(localAABBMax, mtxWorld);
	XMStoreFloat3(&worldPos1, worldPosMax);

	XMVECTOR localAABBMin = XMVectorSet(
		instance.pModel->GetBoundingBox().minPoint.x,
		instance.pModel->GetBoundingBox().minPoint.y,
		instance.pModel->GetBoundingBox().minPoint.z,
		1.0f
	);

	XMVECTOR worldPosMin = XMVector3Transform(localAABBMin, mtxWorld);
	XMStoreFloat3(&worldPos2, worldPosMin);

	instance.collider.aabb.maxPoint = XMFLOAT3(
		max(worldPos1.x, worldPos2.x),
		max(worldPos1.y, worldPos2.y),
		max(worldPos1.z, worldPos2.z)
	);

	instance.collider.aabb.minPoint = XMFLOAT3(
		min(worldPos1.x, worldPos2.x),
		min(worldPos1.y, worldPos2.y),
		min(worldPos1.z, worldPos2.z)
	);
}


template <typename T>
void GameObject<T>::Draw()
{
	// ワールドマトリックスの設定
	renderer.SetCurrentWorldMatrix(&instance.transform.mtxWorld);

	instance.pModel->DrawModel();
}


template<typename T>
inline void GameObject<T>::DrawModelEditor()
{
	if (instance.isCursorIn == TRUE)
	{
		renderer.SetFillMode(D3D11_FILL_WIREFRAME);
		instance.pModel->DrawModel();
		renderer.SetFillMode(D3D11_FILL_SOLID);
	}
	else
	{
		instance.pModel->DrawModel();
	}
}

template<typename T>
inline void GameObject<T>::UpdateModelEditor()
{
	if (instance.isCursorIn == FALSE) return;

	if (IsMouseLeftTriggered())
	{
		instance.isSelected = instance.isSelected == TRUE ? FALSE : TRUE;
		if (instance.isSelected == TRUE)
			MapEditor::get_instance().SetCurSelectedModelIdx(instance.editorIdx);
		else
			MapEditor::get_instance().ResetCurSelectedModelIdx();
	}
}

