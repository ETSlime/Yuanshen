//=============================================================================
//
// �G�l�~�[���f������ [ground.h]
// Author : 
//
//=============================================================================
#pragma once
#include "GameObject.h"
#include "SimpleArray.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define MAX_GROUND		(15)					// �G�l�~�[�̐�
#define	GROUND_SIZE		(5.0f)				// �����蔻��̑傫��
#define ROTATION_SPEED				(0.18f)

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct GROUND
{
	XMFLOAT4X4			mtxWorld;			// ���[���h�}�g���b�N�X
	XMFLOAT3			pos;				// ���f���̈ʒu
	XMFLOAT3			rot;				// ���f���̌���(��])
	XMFLOAT3			scl;				// ���f���̑傫��(�X�P�[��)

	int					state;
	bool				use;
	bool				load;
	Model*				pModel;				// ���f�����
	XMFLOAT4			diffuse[MODEL_MAX_MATERIAL];	// ���f���̐F

	float				spd;				// �ړ��X�s�[�h
	float				size;				// �����蔻��̑傫��
	int					shadowIdx;			// �e�̃C���f�b�N�X�ԍ�

};

class Ground
{
public:
	Ground();
	~Ground();
	void Update(void);
	void Draw(void);
private:
	SimpleArray<GameObject<ModelInstance>*> groundGO;
};