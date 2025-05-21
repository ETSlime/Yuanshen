//=============================================================================
//
// �����Ȃ��g���������茩�����Ⴄ�����d������� [DebugBoundingBoxRenderer.h]
// Author : 
// �o�E���f�B���O�{�b�N�X�����C���ŕ`���āA���f���ⓖ���蔻��̊m�F�ɑ劈�􂷂�f�o�b�O��p�����_���[�ł����I
// ImGui�Ƃ��A�����āA���ǂ̎q�����Ă邩��ڂł킩�閂�@�̃T�|�[�g�W�Ȃ�!
//
//=============================================================================
#pragma once
#include "Core/Graphics/Renderer.h"
#include "Collision/AABBUtils.h"
#include "Utility/Debugproc.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define DEBUG_BOUNDING_BOX_NAME_LENGTH 64

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
struct Vertex
{
    XMFLOAT3 position;
};

struct ConstantBuffer
{
    XMMATRIX worldViewProj;
    XMFLOAT4 color;
};

class DebugBoundingBoxRenderer : public IDebugUI
{
public:
    ~DebugBoundingBoxRenderer();

	void Initialize(char* name = nullptr);
	void DrawBox(const BOUNDING_BOX& box, const XMMATRIX& viewProj, const XMFLOAT4& color);

    virtual void RenderImGui(void) override;
    virtual const char* GetPanelName(void) const override { return m_boxName; };

private:
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    ID3D11Buffer* m_constantBuffer;

    XMFLOAT3 m_center = {};
    XMFLOAT3 m_size = {};
    char* m_boxName = nullptr;

    ShaderSet debugShaderSet;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;


    ShaderResourceBinder& m_ShaderResourceBinder = ShaderResourceBinder::get_instance();
};