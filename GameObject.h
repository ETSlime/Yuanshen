#pragma once

//=============================================================================
//
// �G�l�~�[���f������ [GameObject.h]
// Author : 
//
//=============================================================================

#include "main.h"
#include "renderer.h"
#include "model.h"

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

struct ModelInstance
{
	char*				modelPath;
	int					state;
	bool				use;
	bool				load;
	Model*				pModel;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F
	Transform			transform;

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
	virtual void Update();
	virtual void Draw();
	void DrawModelEditor();
	void UpdateModelEditor();

	inline void SetPosition(XMFLOAT3 pos) { instance.transform.pos = pos; }
	inline void SetRotation(XMFLOAT3 rot) { instance.transform.rot = rot; }
	inline void SetScale(XMFLOAT3 scl) { instance.transform.scl = scl; }
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

protected:
	T instance;
};


template <typename T>
GameObject<T>::GameObject()
{
	instance.pModel = nullptr;
	instance.isSelected = FALSE;
	instance.isCursorIn = FALSE;
	instance.editorIdx = -1;
	instance.state = -1;
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

template <typename T>
void GameObject<T>::Instantiate(char* modelPath)
{
	instance.modelPath = modelPath;
	instance.pModel = Model::StoreModel(modelPath);
	instance.load = true;
	instance.use = true;
}

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
	SetCurrentWorldMatrix(&instance.transform.mtxWorld);

	instance.pModel->DrawModel();
}

template<typename T>
inline void GameObject<T>::DrawModelEditor()
{
	if (instance.isCursorIn == TRUE)
	{
		SetFillMode(D3D11_FILL_WIREFRAME);
		instance.pModel->DrawModel();
		SetFillMode(D3D11_FILL_SOLID);
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

