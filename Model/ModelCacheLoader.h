//=============================================================================
//
// ���f���L���b�V���̓ǂݏ��� [ModelCacheLoader.h]
// Author :
// ���f���f�[�^�� .bin �`���ŕۑ����ǂݍ��݂��āA�������[�h����������֗��N���X�ł���
// �t�@�C����������S�Ƀ��b�v���āA�ǂݍ��ݒ��ɃG���[���Ȃ��悤�Ɏ���Ă���邦�炢�q�Ȃ̂ł�
//
//=============================================================================
#pragma once
#include "main.h"
#include "Core/Graphics/Renderer.h"
#include "Utility/SimpleArray.h"

struct MODEL_DATA;

class ModelCacheLoader 
{
public:

    static inline bool SafeReadFile(HANDLE hFile, void* dest, DWORD size) 
    {
        DWORD bytesRead;
        return ReadFile(hFile, dest, size, &bytesRead, nullptr);
    }

    // �������ݗp�̈��S���b�p�[�֐�
    static bool SafeWriteFile(HANDLE hFile, const void* src, DWORD size) 
    {
        DWORD bytesWritten = 0;
        // WriteFile �����s�����ꍇ��A�������񂾃o�C�g������v���Ȃ��ꍇ�� false ��Ԃ�
        return WriteFile(hFile, src, size, &bytesWritten, nullptr) && bytesWritten == size;
    }

    static bool LoadFromCache(const char* cachePath, MODEL_DATA* modelData);
    static bool SaveToCache(const char* cachePath, const MODEL_DATA* modelData);

    // .obj �p�X���� .bin �p�X�ɕϊ��ichar* ���g���j
    static void ConvertToBinPath(const char* objPath, char* outBinPath, size_t maxLength);
};