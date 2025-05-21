//=============================================================================
//
// �������E�_�����Ȃǃ��C�g���̍X�V����ъǗ� [Light.h]
// Author : 
// �V�[�������C�g�̎�ށE�L����ԁE�����^�ʒu�𐧌䂵�A
// �g�����X�t�H�[���Ƃ̘A�g����уV�F�[�_�[�A�g�p�� LightData ���\�z����
//
//=============================================================================
#pragma once
#include "Scene/GameObject.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
enum class LIGHT_TYPE : int
{
	NONE = 0,			//���C�g����
	DIRECTIONAL,	//�f�B���N�V���i�����C�g
	POINT,			//�|�C���g���C�g

	LIGHT_TYPE_NUM
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
class Light
{
public:
	Light();
	virtual ~Light() = default;

	virtual void Update(void) = 0;
	virtual const LightData& GetLightData() const { return m_LightData; }
	virtual void BindToTransform(const Transform* pTransform);
	void SetEnable(bool flag) { m_LightData.Enable = flag; }
	LIGHT_TYPE GetType(void) const;

protected:
	LightData m_LightData;
	const Transform* ownerTransform;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight();
	void SetTimeBasedRotation(bool enable);
	XMFLOAT3 GetDirection(void) const { return m_LightData.Direction; }
	virtual void Update(void) override;

private:
	bool m_TimeBased = false;
	float m_Time = 0.0f;
};


