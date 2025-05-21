#pragma once
//=============================================================================
//
// �C�ӃN���X�ɑ΂��ăV���O���g���@�\��񋟂���e���v���[�g��� [SingletonBase.h]
// Author : 
// �p����ɐÓI�B��C���X�^���X������񋟂��A�R�s�[�E������֎~���邱�ƂŁA
// �O���[�o���ň�ѐ��̂��郊�\�[�X�Ǘ����\�ɂ���
//
//=============================================================================
#include <iostream>

template <typename T>
class SingletonBase
{
public:

    virtual ~SingletonBase() {
    }

    SingletonBase(const SingletonBase&) = delete;

    SingletonBase& operator=(const SingletonBase&) = delete;

    static T& get_instance() {
        static T instance;

        return instance;
    }

protected:

    
    SingletonBase() {
    }
};
