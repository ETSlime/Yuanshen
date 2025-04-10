//=============================================================================
//
// Sceneˆ— [Scene.h]
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

    // •`‰æ‘ÎÛ‚ÌGameObject‚ğ“o˜^
    void RegisterGameObject(IGameObject* obj)
    {
        m_renderableObjects.push_back(obj);
    }

    // •`‰æ‘ÎÛ‚ÌGameObject‚ğíœ
    void UnregisterGameObject(IGameObject* obj)
    {
        int index = m_renderableObjects.find_index(obj);
        if (index != -1)
            m_renderableObjects.erase(index);
    }

    //•`‰æ‘ÎÛ‚·‚×‚Ä‚ğæ“¾iShadowRenderer“™‚©‚çg—pj
    const SimpleArray<IGameObject*>& GetAllRenderableObjects() const
    {
        return m_renderableObjects;
    }

private:

    SimpleArray<IGameObject*> m_renderableObjects;
};