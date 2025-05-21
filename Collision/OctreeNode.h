//=============================================================================
//
// AABB �ɂ��O�p�`��ԕ����\���i�����؁j [OctreeNode.h]
// Author : 
// �O�p�`�W���� AABB �P�ʂōċA�I�ɋ�ԕ������A
// �����ȗ̈�N�G���ƌ���������\�ɂ���f�[�^�\����񋟂���
// 
//=============================================================================
#pragma once
#include "main.h"
#include "Utility/SimpleArray.h"
#include "Collision/AABBUtils.h"
//*****************************************************************************
// �\���̒�`
//*****************************************************************************

struct Triangle
{
    XMFLOAT3 v0, v1, v2;
    BOUNDING_BOX aabb;
    XMFLOAT3 normal;

    // AABB���v�Z
    Triangle(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c, bool alwaysFaceUp = false) : v0(a), v1(b), v2(c)
    {
        aabb.minPoint.x = min(min(v0.x, v1.x), v2.x);
        aabb.minPoint.y = min(min(v0.y, v1.y), v2.y);
        aabb.minPoint.z = min(min(v0.z, v1.z), v2.z);
        aabb.maxPoint.x = max(max(v0.x, v1.x), v2.x);
        aabb.maxPoint.y = max(max(v0.y, v1.y), v2.y);
        aabb.maxPoint.z = max(max(v0.z, v1.z), v2.z);

        // �@�����v�Z����i���_���W��������ς𗘗p���Đ��K������j
        XMVECTOR v0Vec = XMLoadFloat3(&v0);
        XMVECTOR v1Vec = XMLoadFloat3(&v1);
        XMVECTOR v2Vec = XMLoadFloat3(&v2);
        XMVECTOR edge1 = XMVectorSubtract(v1Vec, v0Vec);
        XMVECTOR edge2 = XMVectorSubtract(v2Vec, v0Vec);
        XMVECTOR normVec = XMVector3Cross(edge1, edge2);
        normVec = XMVector3Normalize(normVec);

        // �@����y�������m�F���A���̏ꍇ�͔��]
        if (XMVectorGetY(normVec) < 0.0f && alwaysFaceUp)
        {
            normVec = XMVectorNegate(normVec);  // �@�����]
        }
        XMStoreFloat3(&normal, normVec);
    }

    // �R���X�g���N�^�F�O�p�`�̖@���𒼐ڎw��ł���悤�ɂ���
    Triangle(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c, const XMFLOAT3& norm)
        : v0(a), v1(b), v2(c), normal(norm)
    {
        aabb.minPoint.x = min(min(v0.x, v1.x), v2.x);
        aabb.minPoint.y = min(min(v0.y, v1.y), v2.y);
        aabb.minPoint.z = min(min(v0.z, v1.z), v2.z);
        aabb.maxPoint.x = max(max(v0.x, v1.x), v2.x);
        aabb.maxPoint.y = max(max(v0.y, v1.y), v2.y);
        aabb.maxPoint.z = max(max(v0.z, v1.z), v2.z);
    }
};

// �����؃m�[�h�̒�`�B�e�m�[�h�͎���� AABB ���E�������ATriangle* ���i�[����B
class OctreeNode 
{
public:
    static const int MAX_TRIANGLES = 10; // 1�m�[�h������̍ő�O�p�`��
    BOUNDING_BOX boundary; // ���̃m�[�h���J�o�[����̈�i���[���h���W�n�j
    SimpleArray<Triangle*> triangles;   // �m�[�h�Ɋi�[�����O�p�`�|�C���^�̃��X�g
    OctreeNode* children[8] = { nullptr }; // 8�̎q�m�[�h�ւ̃|�C���^
    int depth;
    int maxDepth;

    OctreeNode(const BOUNDING_BOX& boundary, int depth = 0, int maxDepth = 8)
        : boundary(boundary), depth(depth), maxDepth(maxDepth) {}

    ~OctreeNode() 
    {
        for (int i = 0; i < 8; i++) 
        {
            delete children[i];
        }
    }

    // ���݂̃m�[�h��8�̎q�m�[�h�ɕ�������֐�
    void subdivide() 
    {
        XMFLOAT3 pMin = boundary.minPoint;
        XMFLOAT3 pMax = boundary.maxPoint;

        // ���E�̒��S�_���v�Z����
        XMFLOAT3 center;
        center.x = (pMin.x + pMax.x) * 0.5f;
        center.y = (pMin.y + pMax.y) * 0.5f;
        center.z = (pMin.z + pMax.z) * 0.5f;

        // 8�̎q�m�[�h�̋��E�����ꂼ��v�Z����
        // �q�m�[�h 0: (min, center)
        children[0] = new OctreeNode(BOUNDING_BOX(pMin, center), depth + 1, maxDepth);
        // �q�m�[�h 1: ( (center.x, min.y, min.z), (max.x, center.y, center.z) )
        children[1] = new OctreeNode(BOUNDING_BOX(XMFLOAT3(center.x, pMin.y, pMin.z), XMFLOAT3(pMax.x, center.y, center.z)), depth + 1, maxDepth);
        // �q�m�[�h 2: ( (min.x, center.y, min.z), (center.x, max.y, center.z) )
        children[2] = new OctreeNode(BOUNDING_BOX(XMFLOAT3(pMin.x, center.y, pMin.z), XMFLOAT3(center.x, pMax.y, center.z)), depth + 1, maxDepth);
        // �q�m�[�h 3: ( (center.x, center.y, min.z), (max.x, max.y, center.z) )
        children[3] = new OctreeNode(BOUNDING_BOX(XMFLOAT3(center.x, center.y, pMin.z), XMFLOAT3(pMax.x, pMax.y, center.z)), depth + 1, maxDepth);
        // �q�m�[�h 4: ( (min.x, min.y, center.z), (center.x, center.y, max.z) )
        children[4] = new OctreeNode(BOUNDING_BOX(XMFLOAT3(pMin.x, pMin.y, center.z), XMFLOAT3(center.x, center.y, pMax.z)), depth + 1, maxDepth);
        // �q�m�[�h 5: ( (center.x, min.y, center.z), (max.x, center.y, max.z) )
        children[5] = new OctreeNode(BOUNDING_BOX(XMFLOAT3(center.x, pMin.y, center.z), XMFLOAT3(pMax.x, center.y, pMax.z)), depth + 1, maxDepth);
        // �q�m�[�h 6: ( (min.x, center.y, center.z), (center.x, max.y, max.z) )
        children[6] = new OctreeNode(BOUNDING_BOX(XMFLOAT3(pMin.x, center.y, center.z), XMFLOAT3(center.x, pMax.y, pMax.z)), depth + 1, maxDepth);
        // �q�m�[�h 7: ( (center.x, center.y, center.z), (max.x, max.y, max.z) )
        children[7] = new OctreeNode(BOUNDING_BOX(center, pMax), depth + 1, maxDepth);
    }

    // �O�p�`�����̔����؂ɑ}������֐�
    bool insert(Triangle* tri) 
    {
        // �O�p�`�� AABB �����̃m�[�h�̋��E�ƌ������Ȃ���Α}�����Ȃ�
        if (!boundary.intersects(tri->aabb))
            return false;

        // ���݂̃m�[�h���t�ŁA�e�ʂ����邩�܂��͍ő�[���ɒB���Ă���ꍇ�͂����Ɋi�[����
        if (triangles.getSize() < MAX_TRIANGLES || depth == maxDepth) 
        {
            triangles.push_back(tri);
            return true;
        }

        // �q�m�[�h�ɕ�������Ă��Ȃ���΁A��������
        if (children[0] == nullptr) 
        {
            subdivide();
        }

        // �q�m�[�h�ɑ}�������݂�
        bool inserted = false;
        for (int i = 0; i < 8; i++) 
        {
            if (children[i]->insert(tri))
                inserted = true;
        }

        // �����q�m�[�h�̂ǂ��ɂ����S�Ɏ��܂�Ȃ��ꍇ�́A���݂̃m�[�h�ɕێ�����
        if (!inserted)
            triangles.push_back(tri);
        return true;
    }

    // �w�肵���͈� (AABB) �ƌ�������O�p�`���N�G������֐�
    void queryRange(const BOUNDING_BOX& range, SimpleArray<Triangle*>& result) const
    {
        // ���݂̃m�[�h�Ɣ͈͂��������Ȃ���΁A�����������^�[��
        if (!boundary.intersects(range))
            return;

        // ���݂̃m�[�h�Ɋi�[���ꂽ�e�O�p�`�ɂ��āAAABB �̌������m�F����
        int size = triangles.getSize();
        for (int i = 0; i < size; i++)
        {
            if (triangles[i]->aabb.intersects(range))
                result.push_back(triangles[i]);
        }

        // �q�m�[�h�����݂���ꍇ�A�ċA�I�ɃN�G�������s����
        if (children[0] != nullptr) 
        {
            for (int i = 0; i < 8; i++) 
            {
                children[i]->queryRange(range, result);
            }
        }
    }
};