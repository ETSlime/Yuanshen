//=============================================================================
//
// �G�l�~�[���f������ [Ground.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "model.h"
#include "input.h"
#include "debugproc.h"
#include "ground.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_GROUND		"data/MODEL/tree.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(5.0f)						// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// ��]��

#define GROUND_SHADOW_SIZE	(0.4f)						// �e�̑傫��
#define GROUND_OFFSET_Y		(7.0f)						// �G�l�~�[�̑��������킹��


//=============================================================================
// ����������
//=============================================================================
Ground::Ground()
{
	for (int i = 0; i < MAX_GROUND; i++)
	{
		GameObject<ModelInstance>* GO = new GameObject<ModelInstance>();
		GO->Instantiate(MODEL_GROUND);

		GO->SetPosition(XMFLOAT3(-50.0f + i * 130.0f, -70.0f, -1720.0f - i * 130.0f));
		GO->SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
		GO->SetScale(XMFLOAT3(12.5f, 12.5f, 12.5f));

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		Model* pModel = GO->GetModel();
		pModel->GetModelDiffuse(&GO->GetDiffuse()[0]);

		groundGO.push_back(GO);

	}
}

//=============================================================================
// �I������
//=============================================================================
Ground::~Ground()
{
	int size = groundGO.getSize();
	for (int i = 0; i < size; i++)
	{
		if (groundGO[i])
			delete groundGO[i];
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void Ground::Update(void)
{
	int size = groundGO.getSize();

	for (int i = 0; i < size; i++)
	{
		if (groundGO[i]->GetUse() == false) continue;

		groundGO[i]->Update();
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void Ground::Draw(void)
{
	int size = groundGO.getSize();

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < size; i++)
	{
		if (groundGO[i]->GetUse() == false) continue;

		groundGO[i]->Draw();
	}

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}
