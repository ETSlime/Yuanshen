//=============================================================================
//
// ÉâÉCÉgèàóù [light.h]
// Author : 
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

	bool enableLight;
	DoubleLinkedList<Light*> m_LightList;
	LIGHT_CBUFFER m_CBuffer;
	Renderer& renderer = Renderer::get_instance();
};