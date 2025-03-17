//=============================================================================
//
// �����蔻�菈�� [CollisionManager.h]
// Author : 
//
//=============================================================================
#pragma once
#include "OctreeNode.h"
#include "SimpleArray.h"
#include "SingletonBase.h"
#include "model.h"
//*****************************************************************************
// �}�N����`
//*****************************************************************************

// �Փˊ�̎�ނ�\���񋓌^
enum class ColliderType 
{
    DEFAULT,
    PLAYER,
    ENEMY,
    WALL,
    TREE,
    ITEM,
    STATIC_OBJECT,
    TELEPORTER,
    PLAYER_ATTACK,
    ENEMY_ATTACK,
};

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
class Collider 
{
public:
    BOUNDING_BOX bbox; // ���[���h���W�ł̏Փ˔���p
    ColliderType type; // �Փˊ�̎��
    void* owner;
    bool enable;

    Collider()
    {
        bbox = BOUNDING_BOX();
        type = ColliderType::DEFAULT;
        enable = false;
        owner = nullptr;
    }
};

// �Փ˃C�x���g�\���́F�Փˎ��ɔ��s�����C�x���g���b�Z�[�W
struct CollisionEvent 
{
    const Collider* colliderA;
    const Collider* colliderB;
};

// �R�[���o�b�N�֐�
typedef void (*CollisionListener)(const CollisionEvent& event, void* context);

// �Փ˃C�x���g���X�i�[�̃G���g���\����
struct CollisionListenerEntry 
{
    CollisionListener listener; // �R�[���o�b�N�֐��|�C���^
    void* context;              // �C�ӂ̃R���e�L�X�g���i�Ăяo�����ŗ��p�\�j
};

// EventBus �N���X�F�C�x���g���w�ǁE���s����V���v���ȃ��b�Z�[�W�V�X�e��
class EventBus 
{
public:
    // �Փ˃C�x���g���X�i�[�̃��X�g��ێ�����
    SimpleArray<CollisionListenerEntry> collisionListeners;

    // �Փ˃C�x���g�̍w�ǂ�o�^����i�֐��|�C���^�ƃR���e�L�X�g��o�^�j
    void subscribeCollision(CollisionListener listener, void* context) 
    {
        CollisionListenerEntry entry;
        entry.listener = listener;
        entry.context = context;
        collisionListeners.push_back(entry);
    }

    // �Փ˃C�x���g�𔭍s����i�o�^���ꂽ�S���X�i�[�ɒʒm����j
    void publishCollision(const CollisionEvent& event) 
    {
        for (UINT i = 0; i < collisionListeners.getSize(); ++i) 
        {
            collisionListeners[i].listener(event, collisionListeners[i].context);
        }
    }
};

// CollisionManager �N���X�F�ÓI�I�u�W�F�N�g�i�����؁j�Ɠ��I�I�u�W�F�N�g�̏ՓˊǗ�
class CollisionManager : public SingletonBase<CollisionManager>
{
public:
    // �ÓI�I�u�W�F�N�g�i��F�n�ʃ��f���j�̔�����
    OctreeNode* staticOctree = nullptr;

    // ���I�I�u�W�F�N�g�i�v���C���[�A�G�A���̑��j�̏Փˑ̃��X�g
    SimpleArray<const Collider*> dynamicColliders;

    // �C�x���g�o�X�̃C���X�^���X
    EventBus eventBus;

    void InitOctree(const BOUNDING_BOX& boundingBox);

    // ���I�Փˑ̂̓o�^
    void RegisterDynamicCollider(const Collider* collider)
    {
        dynamicColliders.push_back(collider);
    }

    // ���I�Փˑ̃��X�g�̃N���A
    void ClearDynamicColliders() 
    {
        dynamicColliders.clear();
    }

    void Update();

private:
    bool IsSelfCollision(const Collider* collider1, const Collider* collider2);

    //// �����Փ˔���֐��iCollider �� Triangle �Ԃ̏Փ˔���j
    //bool PreciseCollision(Collider* col, Triangle* tri);

    //// �����Փ˔���֐��i���I Collider ���m�̏Փ˔���j
    //bool PreciseCollision(Collider* col1, Collider* col2);
};


