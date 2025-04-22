//=============================================================================
//
// ShadowMeshCollector処理 [ShadowMeshCollector.h]
// Author :
//
//=============================================================================
#pragma once

#include "SimpleArray.h"
#include "main.h"
#include "Renderer.h"

class ShadowMeshCollector
{
public:
    void Clear()
    {
        m_staticMeshes.clear();
        m_instancedMeshes.clear();
        m_skinnedMeshes.clear();
    }

    void Reserve(UINT size)
    {
        m_staticMeshes.reserve(size);
        m_instancedMeshes.reserve(size);
        m_skinnedMeshes.reserve(size);
    }

    /*ShadowMeshCollector::Collect 関数のテンプレート分岐について

    この関数は、1つのモデル型（TModel）に対して、
    3種類の異なる描画データ（RenderData）を収集しようとする処理になっています：
    - StaticRenderData（静的モデル用）
    - InstancedRenderData（インスタンス化されたモデル用）
    - SkinnedRenderData（ボーンアニメーションモデル用）

    一見すると「3回全部 CollectShadowMeshFromModel を呼んでいるので、
    間違って全部実行されてしまうのでは？」と思うかもしれませんが、
    実際には「テンプレート特殊化」による**タイプ分岐（タイプディスパッチ）**が行われています。

    CollectShadowMeshFromModel<TModel>(..., StaticRenderData)
    CollectShadowMeshFromModel<TModel>(..., InstancedRenderData)
    CollectShadowMeshFromModel<TModel>(..., SkinnedRenderData)
    これらは全て別の関数として扱われ、TModel によって有効な1つのみがマッチします。

    たとえば TModel = StaticModelHolder の場合：
    - StaticRenderData 版 → 特殊化されて実行される
    - InstancedRenderData 版 → モデルがインスタンス化されていれば実行される（なければ空）
    - SkinnedRenderData 版 → 存在しなければコンパイル時に noop 関数が使われる

    よって、このような設計により、
    「Collect 関数一つであらゆるモデルの ShadowMesh をタイプ別に自動収集」することができます*/	
    template<typename TModel>
    void Collect(const TModel& model)
    {
        CollectShadowMeshFromModel<TModel>(model, m_staticMeshes);

        CollectShadowMeshFromModel<TModel>(model, m_instancedMeshes);

        CollectShadowMeshFromModel<TModel>(model, m_skinnedMeshes);
    }

    const SimpleArray<StaticRenderData>& GetStaticMeshes() const { return m_staticMeshes; }
    const SimpleArray<InstancedRenderData>& GetInstancedMeshes() const { return m_instancedMeshes; }
    const SimpleArray<SkinnedRenderData>& GetSkinnedMeshes() const { return m_skinnedMeshes; }

private:

    SimpleArray<StaticRenderData>     m_staticMeshes;
    SimpleArray<InstancedRenderData>  m_instancedMeshes;
    SimpleArray<SkinnedRenderData>    m_skinnedMeshes;
};

template<typename TModel, typename RenderData>
void CollectShadowMeshFromModel(const TModel& model, SimpleArray<RenderData>& out);


