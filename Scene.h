//=============================================================================
//
// Scene���� [Scene.h]
// Author :
//
//=============================================================================
#pragma once

#include "main.h"
#include "SingletonBase.h"

class IGameObject;

class Scene : public SingletonBase<Scene>
{
public:

    // �`��Ώۂ�GameObject��o�^
    void RegisterGameObject(IGameObject* obj)
    {
        m_renderableObjects.push_back(obj);
    }

    // �`��Ώۂ�GameObject���폜
    void UnregisterGameObject(IGameObject* obj)
    {
        int index = m_renderableObjects.find_index(obj);
        if (index != -1)
            m_renderableObjects.erase(index);
    }

    //�`��Ώۂ��ׂĂ��擾�iShadowRenderer������g�p�j
    const SimpleArray<IGameObject*>& GetAllRenderableObjects() const
    {
        return m_renderableObjects;
    }

private:

    SimpleArray<IGameObject*> m_renderableObjects;
};