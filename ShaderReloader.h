//=============================================================================
//
// ShaderReloader���� [ShaderReloader.h]
// Author : 
//
//=============================================================================
#pragma once
#include "HashMap.h"

class ShaderReloader
{
public:
    // �t�@�C�����Ď��Ώۂɒǉ�����
    void WatchFile(const char* path);

    // �t�@�C�����ύX���ꂽ�����`�F�b�N����
    bool HasFileChanged(const char* path);

private:
    // �t�@�C���p�X�ƍŏI�X�V�����̃}�b�v
    HashMap<const char*, FILETIME, CharPtrHash, CharPtrEquals> m_fileTimeMap;
};
