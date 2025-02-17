#pragma once

//=============================================================================
//
// GameObject���� [GameObject.h]
// Author : 
//
//=============================================================================

#include "main.h"
#include "renderer.h"
#include "model.h"
#include "SkinnedMeshModel.h"

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct Transform
{
	XMMATRIX			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

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
	XMFLOAT4X4		mtxWorld;	// ���[���h�}�g���b�N�X

	bool			load;

	float			spd;		// �ړ��X�s�[�h
	float			dir;		// ����
	float			size;		// �����蔻��̑傫��
	bool			use;

	float			targetDir;

	bool			stopRun;
	bool			isMoving;
	bool			isAttacking;

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
	}
};

struct SkinnedMeshModelInstance
{
	char*				modelPath;
	char*				modelName;
	bool				use;
	bool				load;
	SkinnedMeshModel*	pModel;
	Transform			transform;

	BOOL			isSelected;
	BOOL			isCursorIn;
	int				editorIdx;
	Attributes		attributes;
};

struct ModelInstance
{
	char*				modelPath;
	int					state;
	bool				use;
	bool				load;
	Model*				pModel;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F
	Transform			transform;

	Attributes		attributes;

	BOOL			isSelected;
	BOOL			isCursorIn;
	int				editorIdx;
};

template<typename T>
class GameObject
{
public:
	GameObject();
	~GameObject();
	void Instantiate(char* modelPath);
	void Instantiate(char* modelPath, char* modelName, ModelType modelType, 
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
	inline Model* GetModel() { return instance.pModel; }
	inline XMFLOAT4* GetDiffuse() { return instance.diffuse; }
	inline bool GetUse() { return instance.use; }
	inline void SetUse(bool use) { instance.use = use; }
	inline BOOL GetIsModelSelected() { return instance.isSelected; }
	inline BOOL GetIsCursorIn() { return instance.isCursorIn; }
	inline void SetIsCursorIn(BOOL cursorIn) { instance.isCursorIn = cursorIn; }
	inline int GetEditorIndex() { return instance.editorIdx; }
	inline void SetEditorIndex(int idx) { instance.editorIdx = idx; }
	inline Attributes GetAttributes() { return instance.attributes; }
	inline void UpdateAttributes(Attributes attributes) { instance.attributes = attributes; }
	virtual AnimationStateMachine* GetStateMachine() { return nullptr; }

	Renderer& renderer = Renderer::get_instance();

protected:
	T instance;
};

class ISkinnedMeshModelChar
{
public:
	virtual void PlayWalkAnim() = 0;
	virtual void PlayRunAnim() = 0;
	virtual void PlayJumpAnim() = 0;
	virtual void PlayIdleAnim() = 0;

	virtual bool CanWalk() const = 0;
	virtual bool CanStopWalking() const = 0;
	virtual bool CanAttack() const = 0;
	virtual bool CanRun()	const = 0;
	virtual void OnAttackAnimationEnd() = 0;
	bool AlwaysTrue() const { return true; }
};


template <typename T>
GameObject<T>::GameObject()
{
	instance.pModel = nullptr;
	instance.isSelected = FALSE;
	instance.isCursorIn = FALSE;
	instance.editorIdx = -1;


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

template <typename T>
void GameObject<T>::Update()
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(instance.transform.scl.x, instance.transform.scl.y, instance.transform.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(instance.transform.rot.x, instance.transform.rot.y + XM_PI, instance.transform.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(instance.transform.pos.x, instance.transform.pos.y, instance.transform.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	instance.transform.mtxWorld = mtxWorld;
}


template <typename T>
void GameObject<T>::Draw()
{
	// ���[���h�}�g���b�N�X�̐ݒ�
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

