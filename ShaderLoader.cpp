//=============================================================================
//
// ShaderLoader���� [ShaderLoader.cpp]
// Author : 
//
//=============================================================================
#include "ShaderLoader.h"
#include "SimpleArray.h"


bool ShaderLoader::LoadShaderSet(ID3D11Device* device,
    const char* vsPath, const char* vsEntry,
    const char* psPath, const char* psEntry,
    const char* gsPath, const char* gsEntry,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    UINT numElements,
    ShaderSet& outShaderSet)
{
    ID3DBlob* vsBlob = nullptr;
    if (!CompileAndCreateVertexShader(device, vsPath, vsEntry, &outShaderSet.vs, &vsBlob))
        return false;

    if (layoutDesc)
    {
        device->CreateInputLayout(layoutDesc, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &outShaderSet.inputLayout);
    }
    else
    {
        if (!CreateInputLayoutFromReflection(device, vsBlob, &outShaderSet.inputLayout))
            return false;
    }

    vsBlob->Release();

    if (!CompileAndCreatePixelShader(device, psPath, psEntry, &outShaderSet.ps))
        return false;

    if (gsPath && !CompileAndCreateGeometryShader(device, gsPath, gsEntry, &outShaderSet.gs))
        return false;

    return true;
}


bool ShaderLoader::LoadShaderSetWithoutLayoutCreation(ID3D11Device* device,
    const char* vsPath, const char* vsEntry, 
    const char* psPath, const char* psEntry,
    const char* gsPath, const char* gsEntry,
    ShaderSet& outShaderSet)
{
    ID3DBlob* vsBlob = nullptr;
    if (!CompileAndCreateVertexShader(device, vsPath, vsEntry, &outShaderSet.vs, &vsBlob))
        return false;

    vsBlob->Release();

    if (!CompileAndCreatePixelShader(device, psPath, psEntry, &outShaderSet.ps))
        return false;

    if (gsPath && !CompileAndCreateGeometryShader(device, gsPath, gsEntry, &outShaderSet.gs))
        return false;

    return true;
}

bool ShaderLoader::LoadShadowShaderSet(ID3D11Device* device,
    const char* vsPath, const char* psPath, const char* psAlphaPath,
    const char* vsEntry, const char* psEntry, const char* psAlphaEntry,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    UINT numElements,
    ShadowShaderSet& outShaderSet)
{
    ID3DBlob* vsBlob = nullptr;
    if (!CompileAndCreateVertexShader(device, vsPath, vsEntry, &outShaderSet.vs, &vsBlob))
        return false;

    if (layoutDesc)
        device->CreateInputLayout(layoutDesc, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &outShaderSet.inputLayout);
    else
        CreateInputLayoutFromReflection(device, vsBlob, &outShaderSet.inputLayout);

    vsBlob->Release();

    if (psPath && !CompileAndCreatePixelShader(device, psPath, psEntry, &outShaderSet.ps))
        return false;

    if (psAlphaPath && !CompileAndCreatePixelShader(device, psAlphaPath, psAlphaEntry, &outShaderSet.alphaPs))
        return false;

    return true;
}


bool ShaderLoader::LoadShadowShaderSetWithoutLayoutCreation(ID3D11Device* device,
    const char* vsPath, const char* psPath, const char* psAlphaPath,
    const char* vsEntry, const char* psEntry, const char* psAlphaEntry,
    ShadowShaderSet& outShaderSet)
{
    ID3DBlob* vsBlob = nullptr;
    if (!CompileAndCreateVertexShader(device, vsPath, vsEntry, &outShaderSet.vs, &vsBlob))
        return false;

    vsBlob->Release();

    if (psPath && !CompileAndCreatePixelShader(device, psPath, psEntry, &outShaderSet.ps))
        return false;

    if (psAlphaPath && !CompileAndCreatePixelShader(device, psAlphaPath, psAlphaEntry, &outShaderSet.alphaPs))
        return false;

    return true;
}

bool ShaderLoader::LoadComputerShaderSet(ID3D11Device* device, ComputeShaderSet& outShaderSet)
{
    if (outShaderSet.path && !CompileAndCreateComputeShader(device, outShaderSet.path, outShaderSet.entry, &outShaderSet.cs))
        return false;

    return true;
}

bool ShaderLoader::CompileShaderFromFileLegacy(const char* fileName, const char* entryPoint, const char* target, ID3DBlob** blobOut)
{
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif

    if (entryPoint == nullptr)
        return false;

    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DX11CompileFromFile(
        fileName,
        nullptr, nullptr,
        entryPoint, target,
        shaderFlags, 0,
        nullptr,
        blobOut, &errorBlob, nullptr);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return false;
    }

    return true;
}

bool ShaderLoader::CompileShaderFromFile(const char* fileName, const char* entryPoint, const char* target, ID3DBlob** blobOut, ID3DBlob** errorBlob)
{
    // �t�@�C�����o�C�i���ŊJ��
    FILE* file = nullptr;
    if (fopen_s(&file, fileName, "rb") != 0 || file == nullptr)
        return false;

    // �t�@�C���T�C�Y���擾
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size <= 0)
    {
        fclose(file);
        return false;
    }

    // �������m��
    char* buffer = new char[size];
    if (fread(buffer, 1, size, file) != size)
    {
        delete[] buffer;
        fclose(file);
        return false;
    }

    fclose(file);

    // �t���O�ݒ�i�f�o�b�O���͍œK���I�t�j
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // D3DCompile ���g�p���ăV�F�[�_�[���R���p�C��
    HRESULT hr = D3DCompile(
        buffer, size,
        fileName,                           // �\�[�X���i�x��/�G���[�\���p�j
        nullptr,                            // defines
        nullptr,
        entryPoint, target,
        flags, 0,
        blobOut, errorBlob
    );

    delete[] buffer;
    return SUCCEEDED(hr);
}

bool ShaderLoader::LoadEmptyPixelShader(ID3D11Device* device, ID3D11PixelShader** outPixelShader, const char* fileName, const char* entryPoint)
{
    if (!device || !outPixelShader) return false;

    ID3DBlob* blob = nullptr;
    HRESULT hr = CompileShaderFromFile(fileName, entryPoint, SHADER_MODEL_PS, &blob);
    if (FAILED(hr) || !blob) 
        return false;

    hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outPixelShader);
    blob->Release();

    return SUCCEEDED(hr);
}

bool ShaderLoader::CreateInputLayoutFromReflection(ID3D11Device* device, ID3DBlob* vsBlob, ID3D11InputLayout** outLayout)
{
    // �V�F�[�_�[���t���N�V�����̃C���^�[�t�F�[�X���擾
    ID3D11ShaderReflection* reflector = nullptr;
    HRESULT hr = D3DReflect(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);
    if (FAILED(hr)) return false;

    // �V�F�[�_�[�̏ڍ׏����擾
    D3D11_SHADER_DESC shaderDesc;
    reflector->GetDesc(&shaderDesc);

    // ���̓��C�A�E�g�L�q�q�̔z����쐬�i���̓p�����[�^�[���ɉ����Ċm�ہj
    D3D11_INPUT_ELEMENT_DESC* layoutDescs = new D3D11_INPUT_ELEMENT_DESC[shaderDesc.InputParameters];

    // �e���̓p�����[�^�[�����
    for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
    {
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
        reflector->GetInputParameterDesc(i, &paramDesc);

        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

        // Auto match input type
        // ���̓f�[�^�̌^�ɉ�����DXGI�t�H�[�}�b�g������
        switch (paramDesc.ComponentType)
        {
        case D3D_REGISTER_COMPONENT_FLOAT32:
            if (paramDesc.Mask == 1)
                format = DXGI_FORMAT_R32_FLOAT;
            else if (paramDesc.Mask <= 3)
                format = DXGI_FORMAT_R32G32_FLOAT;
            else if (paramDesc.Mask <= 7)
                format = DXGI_FORMAT_R32G32B32_FLOAT;
            else if (paramDesc.Mask <= 15)
                format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            break;

        case D3D_REGISTER_COMPONENT_SINT32:
            if (paramDesc.Mask == 1)
                format = DXGI_FORMAT_R32_SINT;
            else if (paramDesc.Mask <= 3)
                format = DXGI_FORMAT_R32G32_SINT;
            else if (paramDesc.Mask <= 7)
                format = DXGI_FORMAT_R32G32B32_SINT;
            else if (paramDesc.Mask <= 15)
                format = DXGI_FORMAT_R32G32B32A32_SINT;
            break;

        case D3D_REGISTER_COMPONENT_UINT32:
            if (paramDesc.Mask == 1)
                format = DXGI_FORMAT_R32_UINT;
            else if (paramDesc.Mask <= 3)
                format = DXGI_FORMAT_R32G32_UINT;
            else if (paramDesc.Mask <= 7)
                format = DXGI_FORMAT_R32G32B32_UINT;
            else if (paramDesc.Mask <= 15)
                format = DXGI_FORMAT_R32G32B32A32_UINT;
            break;
        }

        // �s���ȃt�H�[�}�b�g���w�肳�ꂽ�ꍇ�͏����𒆎~
        if (format == DXGI_FORMAT_UNKNOWN)
        {
            delete[] layoutDescs;
            return false;
        }

        // ���͗v�f�̐ݒ�
        D3D11_INPUT_ELEMENT_DESC inputDesc = {};
        inputDesc.SemanticName = paramDesc.SemanticName;            // �Z�}���e�B�b�N��
        inputDesc.SemanticIndex = paramDesc.SemanticIndex;          // �Z�}���e�B�b�N�C���f�b�N�X
        inputDesc.Format = format;                                  // �^�ɉ�����DXGI�t�H�[�}�b�g
        inputDesc.InputSlot = 0;                                    // ���̓X���b�g�i�ʏ�0�j
        inputDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // �����ŃI�t�Z�b�g�v�Z
        inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;     // ���_���Ƃ̃f�[�^
        inputDesc.InstanceDataStepRate = 0;                         // �C���X�^���V���O���g�p

        // ���̓��C�A�E�g�L�q�q�ɒǉ�
        layoutDescs[i] = inputDesc;
    }

    // ���̓��C�A�E�g�𐶐�
    hr = device->CreateInputLayout(layoutDescs, shaderDesc.InputParameters,
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), outLayout);

    // �ꎞ��������������A���t���N�V�������I��
    delete[] layoutDescs;
    reflector->Release();

    return SUCCEEDED(hr);
}

bool ShaderLoader::CompileAndCreateVertexShader(
    ID3D11Device* device, 
    const char* path, 
    const char* entry, 
    ID3D11VertexShader** outVS, 
    ID3DBlob** outVSBlob)
{
    if (!CompileShaderFromFile(path, entry, SHADER_MODEL_VS, outVSBlob))
        return false;

    ID3D11VertexShader* newVS = nullptr;
    HRESULT hr = device->CreateVertexShader(
        (*outVSBlob)->GetBufferPointer(), 
        (*outVSBlob)->GetBufferSize(), 
        nullptr, 
        &newVS);

    if (SUCCEEDED(hr))
    {
        SafeRelease(outVS); // �Â� VS �����
        *outVS = newVS;
        return true;
    }

    SafeRelease(&newVS); // ��������ǎg���Ȃ������ꍇ�����
    return false;
}

bool ShaderLoader::CompileAndCreatePixelShader(
    ID3D11Device* device,
    const char* path,
    const char* entry,
    ID3D11PixelShader** outPS)
{
    ID3DBlob* psBlob = nullptr;
    if (!CompileShaderFromFile(path, entry, SHADER_MODEL_PS, &psBlob))
        return false;

    ID3D11PixelShader* newPS = nullptr;
    HRESULT hr = device->CreatePixelShader(
        psBlob->GetBufferPointer(), 
        psBlob->GetBufferSize(), 
        nullptr, 
        &newPS);

    psBlob->Release();

    if (SUCCEEDED(hr))
    {
        SafeRelease(outPS); // �Â� PS �����
        *outPS = newPS;
        return true;
    }

    SafeRelease(&newPS);
    return false;
}

bool ShaderLoader::CompileAndCreateGeometryShader(
    ID3D11Device* device, 
    const char* path, 
    const char* entry, 
    ID3D11GeometryShader** outGS)
{
    ID3DBlob* gsBlob = nullptr;

    // HLSL �t�@�C�����R���p�C���iGS ���f���Łj
    if (!CompileShaderFromFile(path, entry, SHADER_MODEL_GS, &gsBlob))
        return false;

    ID3D11GeometryShader* newGS = nullptr;
    HRESULT hr = device->CreateGeometryShader(
        gsBlob->GetBufferPointer(),
        gsBlob->GetBufferSize(),
        nullptr,
        &newGS);

    gsBlob->Release();

    if (SUCCEEDED(hr))
    {
        SafeRelease(outGS); // �Â� GS �����
        *outGS = newGS;
        return true;
    }

    SafeRelease(&newGS);
    return false;
}

bool ShaderLoader::CompileAndCreateComputeShader(
    ID3D11Device* device, 
    const char* path, 
    const char* entry, 
    ID3D11ComputeShader** outCS)
{
    ID3DBlob* blob = nullptr;

    // .hlsl ���� CS ���R���p�C��
    if (!CompileShaderFromFile(path, entry, SHADER_MODEL_CS, &blob))
        return false;

    // �R���s���[�g�V�F�[�_�[�̍쐬
    ID3D11ComputeShader* newCS = nullptr;
    HRESULT hr = device->CreateComputeShader(
        blob->GetBufferPointer(), 
        blob->GetBufferSize(), 
        nullptr, 
        &newCS);

    blob->Release();

    if (SUCCEEDED(hr))
    {
        SafeRelease(outCS);
        *outCS = newCS;
        return true;
    }

    SafeRelease(&newCS);
    return false;
}
