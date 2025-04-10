//=============================================================================
//
// ShaderReloader���� [ShaderReloader.cpp]
// Author : 
//
//=============================================================================
#include "ShaderReloader.h"

void ShaderReloader::WatchFile(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA attr;

    // �t�@�C���������擾
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &attr))
    {
        // �ŏI�X�V�������L�^
        m_fileTimeMap[path] = attr.ftLastWriteTime;
    }
}

bool ShaderReloader::HasFileChanged(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA attr;

    // �t�@�C���������擾�ł��Ȃ���ΕύX�Ȃ�
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr))
    {
        return false;
    }

    FILETIME currentTime = attr.ftLastWriteTime;

    auto it = m_fileTimeMap.find(path);

    // ����A�N�Z�X�̏ꍇ�͓o�^�����s���A�ύX�Ȃ��Ɣ��f
    if (it == m_fileTimeMap.end())
    {
        m_fileTimeMap[path] = currentTime;
        return false;
    }

    // �O��̋L�^�Ɣ�r���Ĉ���Ă���ΕύX�Ƃ݂Ȃ�
    if (CompareFileTime(&currentTime, &(*it).value) != 0)
    {
        (*it).value = currentTime;
        return true;
    }

    return false;
}
