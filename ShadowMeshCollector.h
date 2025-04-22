//=============================================================================
//
// ShadowMeshCollector���� [ShadowMeshCollector.h]
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

    /*ShadowMeshCollector::Collect �֐��̃e���v���[�g����ɂ���

    ���̊֐��́A1�̃��f���^�iTModel�j�ɑ΂��āA
    3��ނ̈قȂ�`��f�[�^�iRenderData�j�����W���悤�Ƃ��鏈���ɂȂ��Ă��܂��F
    - StaticRenderData�i�ÓI���f���p�j
    - InstancedRenderData�i�C���X�^���X�����ꂽ���f���p�j
    - SkinnedRenderData�i�{�[���A�j���[�V�������f���p�j

    �ꌩ����Ɓu3��S�� CollectShadowMeshFromModel ���Ă�ł���̂ŁA
    �Ԉ���đS�����s����Ă��܂��̂ł́H�v�Ǝv����������܂��񂪁A
    ���ۂɂ́u�e���v���[�g���ꉻ�v�ɂ��**�^�C�v����i�^�C�v�f�B�X�p�b�`�j**���s���Ă��܂��B

    CollectShadowMeshFromModel<TModel>(..., StaticRenderData)
    CollectShadowMeshFromModel<TModel>(..., InstancedRenderData)
    CollectShadowMeshFromModel<TModel>(..., SkinnedRenderData)
    �����͑S�ĕʂ̊֐��Ƃ��Ĉ����ATModel �ɂ���ėL����1�݂̂��}�b�`���܂��B

    ���Ƃ��� TModel = StaticModelHolder �̏ꍇ�F
    - StaticRenderData �� �� ���ꉻ����Ď��s�����
    - InstancedRenderData �� �� ���f�����C���X�^���X������Ă���Ύ��s�����i�Ȃ���΋�j
    - SkinnedRenderData �� �� ���݂��Ȃ���΃R���p�C������ noop �֐����g����

    ����āA���̂悤�Ȑ݌v�ɂ��A
    �uCollect �֐���ł����郂�f���� ShadowMesh ���^�C�v�ʂɎ������W�v���邱�Ƃ��ł��܂�*/	
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


