//=============================================================================
//
// ShadowMeshCollector���� [ShadowMeshCollector.hpp]
// Author :
//
//=============================================================================
#pragma once
#include "ShadowMeshCollector.h"
#include "GameObject.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define DEFINE_SHADOW_COLLECT_EMPTY(MODEL, RENDERDATA) \
template<> \
inline void CollectShadowMeshFromModel<MODEL>(const MODEL&, SimpleArray<RENDERDATA>&) {}

// �� �e���v���[�g�{��
template<typename TModel, typename RenderData>
inline void CollectShadowMeshFromModel(const TModel&, SimpleArray<RenderData>&)
{
    static_assert(sizeof(TModel) == 0, "Unsupported model instance");
}

// ================================
// �L���ȓ����i���ꂼ��Ή�����g�����j
// ================================

// StaticModel �� StaticRenderData
template<>
inline void CollectShadowMeshFromModel<ModelInstance>(
    const ModelInstance& modelInstance,
    SimpleArray<StaticRenderData>& out)
{
    if (!modelInstance.pModel) return;

    const auto& parts = modelInstance.pModel->GetMeshParts();
    for (const auto& part : parts)
    {
        StaticRenderData data = {};
        data.vertexBuffer = part.VertexBuffer;
        data.indexBuffer = part.IndexBuffer;
        data.stride = sizeof(VERTEX_3D);
        data.indexCount = part.IndexNum;
        data.startIndexLocation = part.StartIndex;
        data.indexFormat = DXGI_FORMAT_R32_UINT;
        data.worldMatrix = modelInstance.transform.mtxWorld;
        out.push_back(std::move(data));
    }
}

// StaticModel�i�C���X�^���V���O�j�� InstancedRenderData
template<>
inline void CollectShadowMeshFromModel<ModelInstance>(
    const ModelInstance& modelInstance,
    SimpleArray<InstancedRenderData>& out)
{
    if (!modelInstance.pModel || !modelInstance.isInstanced) return;

    const auto& parts = modelInstance.pModel->GetMeshParts();
    for (const auto& part : parts)
    {
        InstancedRenderData data = {};
        data.vertexBuffer = part.VertexBuffer;
        data.indexBuffer = part.IndexBuffer;
        data.stride = sizeof(VERTEX_3D);
        data.indexCount = part.IndexNum;
        data.startIndexLocation = part.StartIndex;
        data.indexFormat = DXGI_FORMAT_R32_UINT;
        data.worldMatrix = modelInstance.transform.mtxWorld;
        data.instanceBuffer = modelInstance.instanceBuffer;
        data.instanceCount = modelInstance.instanceCount;
        out.push_back(std::move(data));
    }
}

// SkinnedModel �� SkinnedRenderData
template<>
inline void CollectShadowMeshFromModel<SkinnedMeshModelInstance>(
    const SkinnedMeshModelInstance& modelInstance,
    SimpleArray<SkinnedRenderData>& out)
{
    if (!modelInstance.pModel) return;

    const auto& parts = modelInstance.pModel->GetMeshParts();
    const auto* boneMatrices = modelInstance.pModel->GetFinalBoneMatrices();

    for (const auto& part : parts)
    {
        SkinnedRenderData data = {};
        data.vertexBuffer = part.VertexBuffer;
        data.indexBuffer = part.IndexBuffer;
        data.stride = sizeof(SKINNED_VERTEX_3D);
        data.indexCount = part.IndexNum;
        data.indexFormat = DXGI_FORMAT_R32_UINT;
        data.worldMatrix = modelInstance.transform.mtxWorld;
        data.pBoneMatrices = boneMatrices;
        out.push_back(std::move(data));
    }
}

// ================================
// �����ȑg�ݍ��킹 �� �����
// ================================
// StaticModel �� SkinnedRenderData�i�����j
DEFINE_SHADOW_COLLECT_EMPTY(ModelInstance, SkinnedRenderData);

// SkinnedModel �� StaticRenderData�i�����j
DEFINE_SHADOW_COLLECT_EMPTY(SkinnedMeshModelInstance, StaticRenderData);

// SkinnedModel �� InstancedRenderData�i�����j
DEFINE_SHADOW_COLLECT_EMPTY(SkinnedMeshModelInstance, InstancedRenderData);