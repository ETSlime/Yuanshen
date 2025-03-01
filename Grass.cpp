//=============================================================================
//
// ���̕`�� [Grass.cpp]
// Author : 
//
//=============================================================================
#include "Grass.h"
#include "camera.h"
#include "TextureMgr.h"

Grass::Grass() : 
    m_device(Renderer::get_instance().GetDevice()), 
    m_context(Renderer::get_instance().GetDeviceContext())
{
    Initialize();
}

bool Grass::Initialize(void)
{
    time = 0.0f; // �ݐώ���
    transform.scl = XMFLOAT3(550.0f, 550.0f, 550.0f);
    XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;
    
    // ���[���h�}�g���b�N�X�̏�����
    mtxWorld = XMMatrixIdentity();

    // �X�P�[���𔽉f
    mtxScl = XMMatrixScaling(transform.scl.x, transform.scl.y, transform.scl.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

    // ��]�𔽉f
    mtxRot = XMMatrixRotationRollPitchYaw(transform.rot.x, transform.rot.y + XM_PI, transform.rot.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

    // �ړ��𔽉f
    mtxTranslate = XMMatrixTranslation(transform.pos.x, transform.pos.y, transform.pos.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

    LoadModel();
    LoadShaders();

    // �T���v���[�X�e�[�g�쐬
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    m_device->CreateSamplerState(&samplerDesc, &samplerState);

    // �V���h�E�p�T���v���[ (��r�T���v���[)
    D3D11_SAMPLER_DESC shadowSamplerDesc = {};
    shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamplerDesc.BorderColor[0] = 1.0f;
    shadowSamplerDesc.BorderColor[1] = 1.0f;
    shadowSamplerDesc.BorderColor[2] = 1.0f;
    shadowSamplerDesc.BorderColor[3] = 1.0f;
    shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    m_device->CreateSamplerState(&shadowSamplerDesc, &shadowSamplerState);

    // �m�C�Y�e�N�X�`������
    if (!GenerateNoiseTexture(&noiseTextureSRV))
        return false;

    // �萔�o�b�t�@�쐬
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &perFrameBuffer);
    if (FAILED(hr))
        return false;
}

Grass::~Grass()
{
    SafeRelease(&vertexBuffer);
    SafeRelease(&indexBuffer);
    SafeRelease(&instanceBuffer);
    SafeRelease(&perFrameBuffer);
    SafeRelease(&diffuseTextureSRV);
    SafeRelease(&shadowMapSRV);
    SafeRelease(&noiseTextureSRV);

    SafeRelease(&m_inputLayout);
    SafeRelease(&m_vertexShader);
    SafeRelease(&m_pixelShader);
    SafeRelease(&m_shadowVertexShader);

    SafeRelease(&samplerState);
    SafeRelease(&shadowSamplerState);
}

void Grass::Update(void)
{

}

void Grass::Draw(void)
{
    time++;; // ���ԍX�V

    Renderer::get_instance().SetCurrentWorldMatrix(&transform.mtxWorld);

    // �萔�o�b�t�@�}�b�s���O
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    PerFrameBuffer* bufferData = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);

    bufferData->Time = time; // ���Ԑݒ�
    bufferData->WindDirection = CalculateDynamicWindDirection(time); // ���I�������v�Z
    bufferData->WindStrength = 0.2f + 0.05f * sinf(time * 0.5f); // �������ϓ�

    m_context->Unmap(perFrameBuffer, 0); // �o�b�t�@�A���}�b�v

    // �V���h�E�f�v�X�p�X���s
    RenderShadowPass();
}

void Grass::RenderShadowPass(void)
{

    UINT strides[2] = { sizeof(GrassVertex), sizeof(InstanceData) }; // ���_ & �C���X�^���X�X�g���C�h
    UINT offsets[2] = { 0, 0 }; // �I�t�Z�b�g������
    ID3D11Buffer* buffers[2] = { vertexBuffer, instanceBuffer }; // �o�b�t�@�z��

    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ���_�o�b�t�@�ƃC���X�^���X�o�b�t�@�ݒ�
    m_context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    m_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0); // �C���f�b�N�X�o�b�t�@�ݒ�

    if (Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE)
    {
        m_context->VSSetShader(m_vertexShader, nullptr, 0);
        m_context->VSSetShaderResources(0, 1, &noiseTextureSRV);
        m_context->PSSetShader(m_pixelShader, nullptr, 0);
        m_context->PSSetShaderResources(1, 1, &diffuseTextureSRV);
        m_context->PSSetSamplers(0, 1, &samplerState);
    }
    else if (Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE_SHADOW)
    {
        m_context->VSSetShader(m_shadowVertexShader, nullptr, 0);
        m_context->PSSetSamplers(1, 1, &shadowSamplerState);
    }

    m_context->VSSetConstantBuffers(12, 1, &perFrameBuffer);


    // �C���X�^���X���`����s
    m_context->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}

XMFLOAT3 Grass::CalculateDynamicWindDirection(float time)
{
    float angle = sinf(time * 0.3f) * XM_PIDIV4; // ���ԂɊ�Â������������I�ω�
    return XMFLOAT3(cosf(angle), 0.0f, sinf(angle));
}

bool Grass::GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV)
{
    const int width = 256, height = 256; // �m�C�Y�e�N�X�`���T�C�Y
    unsigned char noiseData[width * height];

    // �ȈՃm�C�Y���� (�T�C���g�ŋ[���m�C�Y����)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float nx = static_cast<float>(x) / width;
            float ny = static_cast<float>(y) / height;
            float value = 0.5f * (sinf(nx * 10.0f) + cosf(ny * 10.0f)); // �T�C���g�ƃR�T�C���g�Ńm�C�Y����
            noiseData[y * width + x] = static_cast<unsigned char>((value + 1.0f) * 127.5f); // 0~255 �Ƀ}�b�s���O
        }
    }

    // �e�N�X�`���L�q�q�ݒ�
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8_UNORM;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.SampleDesc.Count = 1;

    // �����f�[�^�ݒ�
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = noiseData;
    initData.SysMemPitch = width;

    // �e�N�X�`������
    ID3D11Texture2D* noiseTexture = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&texDesc, &initData, &noiseTexture);
    if (FAILED(hr))
        return false;

    // �V�F�[�_�[���\�[�X�r���[�쐬
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = m_device->CreateShaderResourceView(noiseTexture, &srvDesc, noiseTextureSRV);
    if (FAILED(hr))
        return false;

    noiseTexture->Release();

    return true;
}

float Grass::CalculateWeight(float vertexY, float minY, float maxY)
{
    if (maxY - minY == 0.0f) return 0.0f;
    float linearWeight = (vertexY - minY) / (maxY - minY);
    return sqrtf(linearWeight);
}

float Grass::RayIntersectTriangle(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, const Triangle* tri)
{
    XMVECTOR v0 = XMLoadFloat3(&tri->v0);
    XMVECTOR v1 = XMLoadFloat3(&tri->v1);
    XMVECTOR v2 = XMLoadFloat3(&tri->v2);
    XMVECTOR origin = XMLoadFloat3(&rayOrigin);
    XMVECTOR dir = XMLoadFloat3(&rayDir);

    XMVECTOR edge1 = XMVectorSubtract(v1, v0);
    XMVECTOR edge2 = XMVectorSubtract(v2, v0);
    XMVECTOR pvec = XMVector3Cross(dir, edge2);
    float det = XMVectorGetX(XMVector3Dot(edge1, pvec));

    if (fabs(det) < 1e-6f) return -FLT_MAX;
    float invDet = 1.0f / det;
    XMVECTOR tvec = XMVectorSubtract(origin, v0);
    float u = XMVectorGetX(XMVector3Dot(tvec, pvec)) * invDet;
    if (u < 0.0f || u > 1.0f) return -FLT_MAX;

    XMVECTOR qvec = XMVector3Cross(tvec, edge1);
    float v = XMVectorGetX(XMVector3Dot(dir, qvec)) * invDet;
    if (v < 0.0f || u + v > 1.0f) return -FLT_MAX;

    return XMVectorGetX(XMVector3Dot(edge2, qvec)) * invDet;
}

void Grass::LoadModel(void)
{
    model = new GameObject<ModelInstance>();
    model->Instantiate(GRASS_MODEL_PATH);

    const SUBSET* subset = model->GetModel()->GetSubset();
    unsigned int subsetNum = model->GetModel()->GetSubNum();
    if (subsetNum > 0)
        diffuseTextureSRV = subset[0].Texture;
    if (diffuseTextureSRV == nullptr)
        diffuseTextureSRV = TextureMgr::get_instance().CreateTexture(GRASS_TEX_PATH);

    const MODEL_DATA* modelData = model->GetModel()->GetModelData();
    BOUNDING_BOX boudningBox = model->GetModel()->GetBoundingBox();

    GrassVertex* VertexArray = new GrassVertex[modelData->VertexNum];

    for (int i = 0; i < modelData->VertexNum; i++)
    {
        VertexArray[i].Position = modelData->VertexArray[i].Position;
        VertexArray[i].Normal = modelData->VertexArray[i].Normal;
        VertexArray[i].TexCoord = modelData->VertexArray[i].TexCoord;
        VertexArray[i].Weight = CalculateWeight(
            VertexArray[i].Position.y, boudningBox.minPoint.y, boudningBox.maxPoint.y);
    }

    indexCount = modelData->IndexNum;

    // ���_�o�b�t�@����
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(GrassVertex) * modelData->VertexNum;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = VertexArray;

    m_device->CreateBuffer(&bd, &sd, &vertexBuffer);


    // �C���f�b�N�X�o�b�t�@����
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned int) * indexCount;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = modelData->IndexArray;

    m_device->CreateBuffer(&bd, &sd, &indexBuffer);

}

void Grass::LoadShaders(void)
{
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlob, * pPSBlob;
    ID3DBlob* pErrorBlob = NULL;
    UINT compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;

    hr = D3DX11CompileFromFile("Grass.hlsl", NULL, NULL, "VS", "vs_4_0", compileFlags, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
    }

    hr = D3DX11CompileFromFile("Grass.hlsl", NULL, NULL, "PS", "ps_4_0", 0, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
    }

    ID3DBlob* pVSBlob2;
    hr = D3DX11CompileFromFile("Grass.hlsl", NULL, NULL, "VSShadow", "vs_4_0", 0, 0, NULL, &pVSBlob2, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VSShadow", MB_OK | MB_ICONERROR);
    }
    

    m_device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_vertexShader);
    m_device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pixelShader);
    m_device->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), nullptr, &m_shadowVertexShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = 
    {
        // ���_�f�[�^
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   1, DXGI_FORMAT_R32_FLOAT,       0, 32,  D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Weight

        // �C���X�^���X�f�[�^
        { "POSITION",   1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,   D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // OffsetPosition
        { "TEXCOORD",   2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // Rotation (Quaternion)
        { "TEXCOORD",   3, DXGI_FORMAT_R32_FLOAT,       1, 28,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }  // Scale
    };

    m_device->CreateInputLayout(
        layoutDesc,
        ARRAYSIZE(layoutDesc),
        pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(),
        &m_inputLayout);

    pVSBlob->Release();
    pVSBlob2->Release();
    pPSBlob->Release();
}

OctreeNode* Grass::GenerateOctree(const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox)
{
    OctreeNode* octree = new OctreeNode(boundingBox);

    int numTriangles = triangles.getSize();
    for (int i = 0; i < numTriangles; i++)
    {
        if (!octree->insert(triangles[i]))
            return nullptr;
    }

    return octree;
}

bool Grass::CreateInstanceBuffer(void)
{
    InstanceData* instanceData = new InstanceData[instanceCount];

    for (int i = 0; i < instanceCount; ++i) 
    {
        instanceData[i].OffsetPosition = XMFLOAT3(
            GetRand(-100, 100) * 0.5f,  // X���W
            0.0f,                        // Y���W�Œ� (�n��)
            GetRand(-100, 100) * 0.5f    // Z���W
        ); // �����_���z�u
        instanceData[i].Scale = 0.5f + (rand() % 100) / 100.0f; // �X�P�[��: 0.5 ~ 1.5
    }

    // �o�b�t�@�L�q�q�ݒ�
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // �p�ɂȍX�V���K�v�Ȃ��� DYNAMIC �g�p
    bufferDesc.ByteWidth = sizeof(InstanceData) * instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // �����f�[�^�ݒ�
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = instanceData;

    // �C���X�^���X�o�b�t�@�쐬
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &instanceBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

bool Grass::GenerateGrassInstances(const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox, int grassClusterCount)
{
    SimpleArray<InstanceData> grassInstanceDataArray;

    OctreeNode* octree = GenerateOctree(triangles, boundingBox);

    for (int i = 0; i < grassClusterCount; ++i) 
    {
        // �N���X�^�[�̒��S��n�`�S�̂��烉���_���ɑI��
        XMFLOAT3 clusterCenter = 
        {
            GetRandFloat(octree->boundary.minPoint.x, octree->boundary.maxPoint.x),
            0,
            GetRandFloat(octree->boundary.minPoint.z, octree->boundary.maxPoint.z)
        };

        // �N���X�^�[�̃T�C�Y�������_���� (���a�͈͂�ݒ�)
        float clusterRadius = GetRandFloat(3500.0f, 13000.0f);
        int grassCount = static_cast<int>(clusterRadius * GetRandFloat(0.015f, 0.025f)); // �N���X�^�[�T�C�Y�ɉ��������x

        // �N���X�^�[�͈͂��`���A�����؂Ō���
        BOUNDING_BOX clusterBox
        (
            XMFLOAT3(clusterCenter.x - clusterRadius, octree->boundary.minPoint.y, clusterCenter.z - clusterRadius),
            XMFLOAT3(clusterCenter.x + clusterRadius, octree->boundary.maxPoint.y, clusterCenter.z + clusterRadius)
        );
        SimpleArray<Triangle*> nearbyTriangles;
        octree->queryRange(clusterBox, nearbyTriangles);

        if (nearbyTriangles.getSize() == 0) continue; // �N���X�^�[���ɎO�p�`���Ȃ��ꍇ�X�L�b�v

        for (int j = 0; j < grassCount; ++j) 
        {
            // �N���X�^�[�͈͓��̃����_���ȓ_�𐶐�
            float randomaAngle = GetRandFloat(0.0f, XM_2PI);
            float randomRadius = GetRandFloat(0.0f, clusterRadius);
            XMFLOAT3 pos = 
            {
                clusterCenter.x + cosf(randomaAngle) * randomRadius,
                octree->boundary.maxPoint.y, // �����ʒu���烌�C���������ɓ���
                clusterCenter.z + sinf(randomaAngle) * randomRadius
            };
            BOUNDING_BOX grassBoundingBox;
            grassBoundingBox.maxPoint = XMFLOAT3(pos.x + 0.1f, octree->boundary.maxPoint.y, pos.z + 0.1f);
            grassBoundingBox.minPoint = XMFLOAT3(pos.x - 0.1f, octree->boundary.minPoint.y, pos.z - 0.1f);
            // �����؂Ŏ擾�����O�p�`�̒�����ł��������̂�I��
            const Triangle* bestTriangle = nullptr;
            XMFLOAT3 rayDir = { 0.0f, -1.0f, 0.0f };
            float maxHeight = -FLT_MAX;
            for (int k = 0; k < nearbyTriangles.getSize(); ++k) 
            {
                const Triangle* tri = nearbyTriangles[k];
                float t = RayIntersectTriangle(pos, rayDir, tri);
                if (t > 0 && (pos.y - t) > maxHeight)
                {
                    maxHeight = pos.y - t;
                    bestTriangle = tri;
                }
            }
            if (!bestTriangle) continue;

            // ���� Y ���W���ō��_�ɒ���
            pos.y = maxHeight;

            // �@�������]�N�H�[�^�j�I�����v�Z
            XMFLOAT4 rotation;
            XMVECTOR normalVec = XMLoadFloat3(&bestTriangle->normal);

            // �f�t�H���g�̏�����x�N�g�� (Y ��)
            XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);

            // �@�������łɏ�����������Ă���ꍇ�A��]��K�p���Ȃ�
            float dotProduct = XMVectorGetX(XMVector3Dot(upVec, normalVec));
            float angle = acosf(dotProduct); // ���W�A���P�ʂ̊p�x

            constexpr float maxSlopeAngle = XMConvertToRadians(60.0f); // �ő勖�e�p�x (60��)

            // �����p�x���ő勖�e�l�𒴂����ꍇ�A���𐶐����Ȃ�
            if (angle > maxSlopeAngle)
                continue;

            // �@�������łɏ�����������Ă���ꍇ�A��]��K�p���Ȃ�
            if (XMVector3Equal(normalVec, upVec))
            {
                rotation = XMFLOAT4(0, 0, 0, 1);
            }
            else
            {
                // ��]�����v�Z�i������x�N�g���Ɩ@���̊O�ρj
                XMVECTOR rotationAxis = XMVector3Normalize(XMVector3Cross(upVec, normalVec));

                // ��]�p���v�Z�i������x�N�g���Ɩ@���̊Ԃ̊p�x�j
                float rotationAngle = acosf(XMVectorGetX(XMVector3Dot(upVec, normalVec)));

                // �N�H�[�^�j�I���𐶐�
                XMVECTOR quaternion = XMQuaternionRotationAxis(rotationAxis, rotationAngle);

                XMStoreFloat4(&rotation, quaternion);
            }

            // �C���X�^���X�f�[�^��ǉ�
            grassInstanceDataArray.push_back(
                InstanceData(
                    pos,                 // ���̈ʒu
                    rotation,            // ���̉�] (�@�������Ɋ�Â�)
                    GetRandFloat(20, 40) // ���̃X�P�[�� (�����_����)
                ));
        }
        
    }

    instanceCount = grassInstanceDataArray.getSize();
    instanceData = new InstanceData[instanceCount];
    for (int i = 0; i < instanceCount; i++)
        instanceData[i] = grassInstanceDataArray[i];

    // �o�b�t�@�L�q�q�ݒ�
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT; // �p�ɂȍX�V���K�v�Ȃ��� DYNAMIC �g�p
    bufferDesc.ByteWidth = sizeof(InstanceData) * instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    // �����f�[�^�ݒ�
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = instanceData;

    // �C���X�^���X�o�b�t�@�쐬
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &instanceBuffer);
    if (FAILED(hr))
        return false;

    return true;

}
