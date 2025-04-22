//=============================================================================
//
// UIElementèàóù [UIElement.h]
// Author : 
//
//=============================================================================
#pragma once
#include "Renderer.h"

class UIElement
{
public:
    virtual void Update(void) = 0;
    virtual void Draw(void) = 0;

    virtual void OnHover(void) {}
    virtual void OnUnhover(void) {}

    void SetZOrder(int z) { m_zOrder = z; }
    int  GetZOrder(void) const { return m_zOrder; }

    bool IsMouseOver(float mouseX, float mouseY) const { return false; }

    virtual bool NeedsOverlay() const { return HasCustomOverlay(); }
    virtual bool HasCustomOverlay(void) const { return m_customOverlay; }
    virtual void DrawCustomOverlay(void) const {}

protected:
    XMFLOAT2 m_position;
    XMFLOAT2 m_size;
    int m_zOrder = 0;
    bool m_isHovering = false;
    bool m_customOverlay = false;
};
