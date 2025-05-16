//=============================================================================
//
// ���C�g�ꗗ�̓o�^�E�L���Ǘ�����ђ萔�o�b�t�@���� [light.h]
// Author : 
// �S���C�g�̒ǉ��^�폜�^�X�V�𓝊����A�L����Ԃ̐���ƂƂ��ɁA
// �V�F�[�_�[������ LIGHT_CBUFFER ���\�z�E�񋟂���}�l�[�W���N���X
//
//=============================================================================
#pragma once
#include "Light.h"
#include "DoubleLinkedList.h"

class LightManager : public SingletonBase<LightManager>
{
public:
	LightManager();

	void AddLight(Light* light);
	void RemoveLight(Light* light);
	void SetLightEnable(BOOL flag);
	void Update(void);

	const LIGHT_CBUFFER& GetCBuffer() const { return m_CBuffer; }

	const DoubleLinkedList<Light*>& GetLightList(void) { return m_LightList; }

private:

	bool m_enableLight;
	DoubleLinkedList<Light*> m_LightList;
	LIGHT_CBUFFER m_CBuffer;
	Renderer& m_renderer = Renderer::get_instance();
};