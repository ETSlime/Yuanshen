#pragma once
//=============================================================================
//
// �\�[�h�g���C�� [SwordTrail.h]
// Author : 
// - ����̓������G�����ʂ�c���G�t�F�N�g��
// - ���Ɛn��̗�����ێ����āA��ԂɌ��z���c��
//
//=============================================================================
#include "Scene/GameObject.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define SWORD_TRAIL_TEXTURE_PATH	"data/TEXTURE/SwordTrail/SwordTrail.png"

//*****************************************************************************
// �\���̒�`
//*****************************************************************************

enum class SwordTrailType
{
	Default,
	Normal,
	Flame,
	Ice,
	Lightning,
};

class SwordTrail
{
public:
	SwordTrail(const GameObject<SkinnedMeshModelInstance>* weapon);
	~SwordTrail();
	void Update(void);
	void Draw(void);

	// hiltTrail, tipTrail �̗������� VFXVertex �𐶐����� vertexBuffer ���X�V
	void GenerateSwordTrailMesh(void);
	// �O�Ղ̌����ڃ^�C�v�i�ʏ�A���A�X�A���c�j��؂�ւ���
	bool SetTrailType(SwordTrailType type);
	// �ő�t���[�����i���𐔁j���w��A�܂�u�c���̒����v�𐧌�ł���
	void SetTrailFrames(int frames);
	// ���݂̕��킩�畿�E�n���3D�ʒu���v�Z���Ď擾
	void GetSwordTrailPoints(XMVECTOR& hiltWorld, XMVECTOR& tipWorld);
private:

	bool LoadTrailTexture(char* path);

	const GameObject<SkinnedMeshModelInstance>* weapon;

	SwordTrailType trailType;

	int maxTrailFrames;

	ID3D11Buffer* vertexBuffer;
	ID3D11ShaderResourceView* swordTrailTexture;
	SimpleArray<XMVECTOR> hiltTrail;
	SimpleArray<XMVECTOR> tipTrail;
	SimpleArray<VFXVertex> swordTrailVertices;

	Renderer& renderer = Renderer::get_instance();
};