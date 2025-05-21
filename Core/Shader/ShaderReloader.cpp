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
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &attr))
    {
        uint64_t hash = CalculateFileHash(path);
        m_fileStateMap[path] = { attr.ftLastWriteTime, hash };
    }
}

bool ShaderReloader::HasFileChanged(const char* path)
{
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr)) return false;

    FILETIME newTime = attr.ftLastWriteTime;
    uint64_t newHash = CalculateFileHash(path);

    auto& state = m_fileStateMap[path];
    if (CompareFileTime(&newTime, &state.lastWriteTime) != 0 || newHash != state.contentHash)
    {
        state.lastWriteTime = newTime;
        state.contentHash = newHash;
        return true;
    }
    return false;
}

//bool ShaderReloader::HasFileChanged(const char* path)
//{
//    FILETIME newTime;
//    uint64_t newHash;
//
//    // �t�@�C���������擾�ł��Ȃ���ΕύX�Ȃ�
//    if (!GetFileHashAndTime(path, newTime, newHash))
//    {
//        return false;
//    }
//
//    auto it = m_fileStateMap.find(path);
//
//    // ����A�N�Z�X�̏ꍇ�͓o�^�����s���A�ύX�Ȃ��Ɣ��f
//    if (it == m_fileStateMap.end())
//    {
//        m_fileStateMap[path] = { newTime, newHash };
//        return false;
//    }
//
//    // �O��̋L�^�Ɣ�r���Ĉ���Ă���ΕύX�Ƃ݂Ȃ�
//    if (CompareFileTime(&newTime, &it->value.lastWriteTime) != 0)
//    {
//        // ���e���ς���Ă����ꍇ�̂� true ��Ԃ�
//        if (newHash != it->value.contentHash)
//        {
//            it->value.lastWriteTime = newTime;
//            it->value.contentHash = newHash;
//            return true; // ���ۂɒ��g���ς�����I
//        }
//
//        // ���������ς���������e�͓��� �� �X�V���Ȃ�
//        it->value.lastWriteTime = newTime; // �ێ�����
//
//        return false;
//    }
//
//    // N �t���[�����Ƃ� fallback �n�b�V���`�F�b�N
//    if (m_frameCount == 0)
//    {
//        if (newHash != it->value.contentHash)
//        {
//            it->value.contentHash = newHash;
//            return true; // �����������ł����g���ς�����I
//        }
//    }
//
//    return false;
//}

//void ShaderReloader::AdvanceFrame(void)
//{
//    m_frameCount++;
//
//    // �w��̃t���[�����ɒB�����烊�Z�b�g�i���̃t���[�����`�F�b�N�ΏۂɂȂ�j
//    if (m_frameCount >= kHashFallbackInterval)
//    {
//        m_frameCount = 0;
//    }
//}

bool ShaderReloader::HasShaderContentChanged(const char* path, const char* entry, const void* blob, size_t size)
{
    ShaderSourceKey key = { path, entry };
    uint64_t newHash = CalculateBlobHash(blob, size);
    uint64_t& oldHash = m_blobHashMap[key];

    if (oldHash != newHash)
    {
        oldHash = newHash;
        return true; // �{���ɕω�����
    }
    return false; // blob �͓���
}

//bool ShaderReloader::GetFileHashAndTime(const char* path, FILETIME& outTime, uint64_t& outHash)
//{
//    WIN32_FILE_ATTRIBUTE_DATA attr;
//
//    // �t�@�C���������擾
//    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr))
//        return false;
//
//    // �ŏI�X�V�������L�^
//    outTime = attr.ftLastWriteTime;
//    outHash = CalculateFileHash(path);
//    return true;
//}

uint64_t ShaderReloader::CalculateBlobHash(const void* data, size_t size)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < size; ++i)
    {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

uint64_t ShaderReloader::CalculateFileHash(const char* path)
{
    FILE* file = nullptr;
    fopen_s(&file, path, "rb");
    if (!file) return 0;

    const size_t bufferSize = 4096;
    uint8_t buffer[bufferSize];
    uint64_t hash = 0xcbf29ce484222325ULL; // FNV-1a offset basis

    while (!feof(file))
    {
        size_t read = fread(buffer, 1, bufferSize, file);
        for (size_t i = 0; i < read; ++i)
        {
            hash ^= buffer[i];
            hash *= 1099511628211ULL; // FNV-1a prime
        }
    }

    fclose(file);
    return hash;
}
