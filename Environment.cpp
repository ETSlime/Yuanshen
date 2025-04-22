//=============================================================================
//
// ���̕`�� [Environment.cpp]
// Author : 
//
//=============================================================================
#include "Environment.h"
#include "camera.h"
#include "TextureMgr.h"
#include "ShadowMeshCollector.hpp"

Environment::Environment() : 
    m_device(Renderer::get_instance().GetDevice()), 
    m_context(Renderer::get_instance().GetDeviceContext())
{
    time = 0.0f; // �ݐώ���

    Initialize();
    CompileShaders();
}

Environment::Environment(EnvironmentConfig config) :
    m_device(Renderer::get_instance().GetDevice()),
    m_context(Renderer::get_instance().GetDeviceContext())
{
	time = 0.0f; // �ݐώ���

	Initialize();
	CompileShaders();

	if (config.loadGrass)
		InitializeEnvironmentObj(EnvironmentObjectType::Grass_1);
	if (config.loadBush)
		InitializeEnvironmentObj(EnvironmentObjectType::Bush_1);
	if (config.loadFlower)
		InitializeEnvironmentObj(EnvironmentObjectType::Flower_1);
	if (config.loadTree)
		InitializeEnvironmentObj(EnvironmentObjectType::Tree_1);
	if (config.loadRock)
		InitializeEnvironmentObj(EnvironmentObjectType::Rock);
}

bool Environment::Initialize(void)
{
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

    return true;
}

EnvironmentObject* Environment::InitializeEnvironmentObj(EnvironmentObjectType type)
{
    EnvironmentObject* obj;

    switch (type)
    {
    case EnvironmentObjectType::Grass_1:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = true;
        obj->attributes.castShadow = false;
        obj->attributes.collision = false;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = false;
        obj->attributes.maxSlopeAngle = GRASS_MAX_SLOPE_ANGLE;
        obj->modelPath = GRASS_1_MODEL_PATH;
        obj->texturePath = GRASS_1_TEX_PATH;
        break;
    case EnvironmentObjectType::Grass_2:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = true;
        obj->attributes.collision = false;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = true;
        obj->attributes.maxSlopeAngle = GRASS_MAX_SLOPE_ANGLE;
        obj->modelPath = GRASS_2_MODEL_PATH;
        obj->texturePath = GRASS_2_TEX_PATH;
        break;
    case EnvironmentObjectType::Clover_1:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = false;
        obj->attributes.collision = false;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = false;
        obj->attributes.maxSlopeAngle = CLOVER_MAX_SLOPE_ANGLE;
        obj->modelPath = CLOVER_1_MODEL_PATH;
        obj->texturePath = CLOVER_1_TEX_PATH;
        break;
    case EnvironmentObjectType::Bush_1:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = true;
        obj->attributes.collision = false;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = false;
        obj->attributes.maxSlopeAngle = BUSH_MAX_SLOPE_ANGLE;
        obj->modelPath = BUSH_1_MODEL_PATH;
        break;
    case EnvironmentObjectType::Bush_2:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = true;
        obj->attributes.collision = true;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = false;
        obj->attributes.maxSlopeAngle = BUSH_MAX_SLOPE_ANGLE;
        obj->modelPath = BUSH_2_MODEL_PATH;
        break;
    case EnvironmentObjectType::Flower_1:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = false;
        obj->attributes.collision = false;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = true;
        obj->attributes.maxSlopeAngle = FLOWER_MAX_SLOPE_ANGLE;
        obj->modelPath = FLOWER_1_MODEL_PATH;
        obj->texturePath = FLOWER_1_TEX_PATH;
        break;
    case EnvironmentObjectType::Shrubbery_1:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = true;
        obj->attributes.collision = false;
        obj->attributes.adaptedToTerrain = true;
        obj->attributes.billboard = true;
        obj->attributes.maxSlopeAngle = SHRUBBERY_MAX_SLOPE_ANGLE;
        obj->modelPath = SHRUBBERY_1_MODEL_PATH;
        obj->texturePath = SHRUBBERY_1_TEX_PATH;
        break;
    case EnvironmentObjectType::Tree_1:
        obj = new EnvironmentObject();
        obj->type = type;
        obj->attributes.load = true;
        obj->attributes.affectedByWind = false;
        obj->attributes.castShadow = true;
        obj->attributes.collision = true;
        obj->attributes.adaptedToTerrain = false;
        obj->attributes.billboard = true;
        obj->attributes.maxSlopeAngle = TREE_MAX_SLOPE_ANGLE;
        obj->modelPath = TREE_1_MODEL_PATH;
        obj->texturePath = TREE_1_TEX_PATH;
        break;
    default:
        return nullptr;
    }

    if (!LoadEnvironmentObj(obj))
    {
        SAFE_DELETE(obj);
        return nullptr;
    }

    environmentObjects.push_back(obj);

    return obj;
}

Environment::~Environment()
{
    for (UINT i = 0; i < environmentObjects.getSize(); i++)
	{
		SAFE_DELETE(environmentObjects[i]);
	}

    SafeRelease(&perFrameBuffer);
    SafeRelease(&shadowMapSRV);
    SafeRelease(&noiseTextureSRV);

    SafeRelease(&instanceInputLayout);
    SafeRelease(&grassVertexShader);
    SafeRelease(&grassPixelShader);
    SafeRelease(&treeVertexShader);
    SafeRelease(&treePixelShader);
    SafeRelease(&shadowVertexShader);

    SafeRelease(&samplerState);
    SafeRelease(&shadowSamplerState);
}

void Environment::Update(void)
{
    time++; // ���ԍX�V

    // �萔�o�b�t�@�}�b�s���O
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    PerFrameBuffer* bufferData = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);

    bufferData->Time = time * 0.0005f; // ���Ԑݒ�
    bufferData->WindDirection = CalculateDynamicWindDirection(time); // ���I�������v�Z
    bufferData->WindStrength = 0.2f + 0.05f * sinf(time * 0.5f); // �������ϓ�

    m_context->Unmap(perFrameBuffer, 0); // �o�b�t�@�A���}�b�v

    // �r���{�[�h�I�u�W�F�N�g�̉�]�s��v�Z
    for (UINT i = 0; i < environmentObjects.getSize(); i++)
    {
        EnvironmentObject* obj = environmentObjects[i];
        if (obj->attributes.billboard)
        {
            Camera& camera = Camera::get_instance();
            XMMATRIX mtxView = XMLoadFloat4x4(&camera.GetViewMatrix());

            // �����s��i�����s��j��]�u�s�񂳂��ċt�s�������Ă��(����)
            XMMATRIX billboardRotation = XMMatrixIdentity();
            billboardRotation.r[0] = XMVectorSet(mtxView.r[0].m128_f32[0], mtxView.r[1].m128_f32[0], mtxView.r[2].m128_f32[0], 0.0f);
            billboardRotation.r[1] = XMVectorSet(mtxView.r[0].m128_f32[1], mtxView.r[1].m128_f32[1], mtxView.r[2].m128_f32[1], 0.0f);
            billboardRotation.r[2] = XMVectorSet(mtxView.r[0].m128_f32[2], mtxView.r[1].m128_f32[2], mtxView.r[2].m128_f32[2], 0.0f);

            // �C���X�^���X�f�[�^�̉�]�s��X�V
            for (int j = 0; j < obj->instanceCount; j++)
            {
                XMVECTOR quat = XMQuaternionRotationMatrix(
                    XMMatrixRotationQuaternion(XMLoadFloat4(&obj->instanceData[j].initialBillboardRot)) * billboardRotation);

                XMStoreFloat4(&obj->instanceData[j].Rotation, quat);
            }

            // �C���X�^���X�o�b�t�@�}�b�s���O
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = m_context->Map(obj->instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

            if (SUCCEEDED(hr))
            {
                memcpy(mappedResource.pData, obj->instanceData, sizeof(InstanceData) * obj->instanceCount);
            }

            m_context->Unmap(obj->instanceBuffer, 0);
        }
    }


}

void Environment::Draw(void)
{
    for (UINT i = 0; i < environmentObjects.getSize(); i++)
	{
		EnvironmentObject* obj = environmentObjects[i];
		if (obj->attributes.use)
		{
            Renderer::get_instance().SetCurrentWorldMatrix(&obj->transform.mtxWorld);

            RenderEnvironmentObj(obj);
		}
	}

}

void Environment::RenderEnvironmentObj(EnvironmentObject* obj)
{
    // �e�`�惂�[�h�̏ꍇ�A�e�𐶐����Ȃ��I�u�W�F�N�g�͕`�悵�Ȃ�
    if (!obj->attributes.castShadow &&
        Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE_SHADOW)
        return;

    UINT strides[2] = { sizeof(InstanceVertex), sizeof(InstanceData) }; // ���_ & �C���X�^���X�X�g���C�h
    UINT offsets[2] = { 0, 0 }; // �I�t�Z�b�g������
    ID3D11Buffer* buffers[2] = { obj->vertexBuffer, obj->instanceBuffer }; // �o�b�t�@�z��

    m_context->IASetInputLayout(instanceInputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ���_�o�b�t�@�ƃC���X�^���X�o�b�t�@�ݒ�
    m_context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    // �C���f�b�N�X�o�b�t�@�ݒ�
    m_context->IASetIndexBuffer(obj->indexBuffer, DXGI_FORMAT_R32_UINT, 0); 
    
    m_context->VSSetConstantBuffers(12, 1, &perFrameBuffer);

    if (Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE)
    {
        m_context->VSSetShader(obj->vertexShader, nullptr, 0);

        // ���ɉe�������ꍇ�A�m�C�Y�e�N�X�`���ƃs�N�Z���V�F�[�_�[��ݒ�
        if (obj->attributes.affectedByWind)
            m_context->VSSetShaderResources(7, 1, &noiseTextureSRV);

        m_context->PSSetShader(obj->pixelShader, nullptr, 0);
        m_context->PSSetSamplers(0, 1, &samplerState);

    }
    else if (Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE_SHADOW)
    {
        m_context->VSSetShader(shadowVertexShader, nullptr, 0);
        m_context->PSSetShader(nullptr, nullptr, 0);
        m_context->PSSetSamplers(1, 1, &shadowSamplerState);
    }

    if (obj->externTexPath)
    {
        MATERIAL material;
        ZeroMemory(&material, sizeof(material));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        Renderer::get_instance().SetMaterial(material);
        m_context->PSSetShaderResources(0, 1, &obj->externDiffuseTextureSRV);
        // �C���X�^���X���`����s
        m_context->DrawIndexedInstanced(obj->indexCount, obj->instanceCount, 0, 0, 0);
    }
    else
    {
        const SUBSET* subset = obj->model->GetModel()->GetSubset();
        unsigned int subsetNum = obj->model->GetModel()->GetSubNum();

        for (unsigned int i = 0; i < subsetNum; i++)
        {
            // �}�e���A���ݒ�
            if (subset[i].Material.MaterialData.LoadMaterial)
                Renderer::get_instance().SetMaterial(subset[i].Material.MaterialData);

            // �e�N�X�`���ݒ�
            if (subset[i].Material.MaterialData.noTexSampling == 0)
            {
                m_context->PSSetShaderResources(0, 1, &subset[i].diffuseTexture);
            }
            if (subset[i].Material.MaterialData.normalMapSampling == 1)
            {
                m_context->PSSetShaderResources(9, 1, &subset[i].normalTexture);
            }
            if (subset[i].Material.MaterialData.bumpMapSampling == 1)
            {
                m_context->PSSetShaderResources(10, 1, &subset[i].bumpTexture);
            }
            if (subset[i].Material.MaterialData.opacityMapSampling == 1)
            {
                m_context->PSSetShaderResources(11, 1, &subset[i].opacityTexture);
            }
            if (subset[i].Material.MaterialData.reflectMapSampling == 1)
            {
                m_context->PSSetShaderResources(12, 1, &subset[i].reflectTexture);
            }
            if (subset[i].Material.MaterialData.translucencyMapSampling == 1)
            {
                m_context->PSSetShaderResources(13, 1, &subset[i].translucencyTexture);
            }

            // �C���X�^���X���`����s
            m_context->DrawIndexedInstanced(subset[i].IndexNum, obj->instanceCount, subset[i].StartIndex, 0, 0);
        }
    }
}

XMFLOAT3 Environment::CalculateDynamicWindDirection(float time)
{
    float angle = sinf(time * 0.3f) * XM_PIDIV4; // ���ԂɊ�Â������������I�ω�
    return XMFLOAT3(cosf(angle), 0.0f, sinf(angle));
}

bool Environment::GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV)
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

float Environment::CalculateWeight(float vertexY, float minY, float maxY)
{
    if (maxY - minY == 0.0f) return 0.0f;
    float normalizedHeight = (vertexY - minY) / (maxY - minY);
    return powf(normalizedHeight, 0.01f); // �����̉e������苭��
}

float Environment::RayIntersectTriangle(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, const Triangle* tri)
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

bool Environment::LoadEnvironmentObj(EnvironmentObject* obj)
{
    obj->model = new GameObject<ModelInstance>();
    obj->model->Instantiate(obj->modelPath);

    const SUBSET* subset = obj->model->GetModel()->GetSubset();
    unsigned int subsetNum = obj->model->GetModel()->GetSubNum();
    if (subsetNum > 0)
    {
        if (subset[0].diffuseTexture == nullptr && obj->texturePath)
        {
            obj->externTexPath = true;
            obj->externDiffuseTextureSRV = TextureMgr::get_instance().CreateTexture(obj->texturePath);
        }
    }

    const MODEL_DATA* modelData = obj->model->GetModel()->GetModelData();
    BOUNDING_BOX boudningBox = obj->model->GetModel()->GetBoundingBox();
    //obj->model->SetIsInstanced(true); // �C���X�^���X���t���O�𗧂Ă�

    InstanceVertex* VertexArray = new InstanceVertex[modelData->VertexNum];

    for (unsigned int i = 0; i < modelData->VertexNum; i++)
    {
        VertexArray[i].Position = modelData->VertexArray[i].Position;
        VertexArray[i].Normal = modelData->VertexArray[i].Normal;
        VertexArray[i].Tangent = modelData->VertexArray[i].Tangent;
        VertexArray[i].TexCoord = modelData->VertexArray[i].TexCoord;

        // ���ɉe�������ꍇ�A�d�݂��v�Z
        if (obj->attributes.affectedByWind)
        {
            VertexArray[i].Weight = CalculateWeight(
                VertexArray[i].Position.y, boudningBox.minPoint.y, boudningBox.maxPoint.y);
        }
        else
            VertexArray[i].Weight = 0.0f;

    }

    obj->indexCount = modelData->IndexNum;

    HRESULT hr = S_OK;
    // ���_�o�b�t�@����
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(InstanceVertex) * modelData->VertexNum;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = VertexArray;

    hr = m_device->CreateBuffer(&bd, &sd, &obj->vertexBuffer);
    if (FAILED(hr))
        return false;

    // �C���f�b�N�X�o�b�t�@����
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned int) * modelData->IndexNum;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = modelData->IndexArray;

    hr = m_device->CreateBuffer(&bd, &sd, &obj->indexBuffer);
    if (FAILED(hr))
        return false;

    SAFE_DELETE_ARRAY(VertexArray);

    LoadShaders(obj);

    return true;
}

void Environment::LoadShaders(EnvironmentObject* obj)
{
	if (obj->attributes.affectedByWind)
	{
		obj->vertexShader = grassVertexShader;
		obj->pixelShader = grassPixelShader;
	}
	else
	{
		obj->vertexShader = treeVertexShader;
        obj->pixelShader = treePixelShader;
	}
}

void Environment::CompileShaders(void)
{
    HRESULT hr = S_OK;

    ID3DBlob* pVSBlob, * pPSBlob;
    ID3DBlob* pErrorBlob = NULL;
    UINT compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;

    // �V�F�[�_�[�R���p�C��
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

    m_device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &grassVertexShader);
    m_device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &grassPixelShader);

    ID3DBlob* pVSBlob2, *pPSBlob2;

    hr = D3DX11CompileFromFile("Tree.hlsl", NULL, NULL, "VS", "vs_4_0", compileFlags, 0, NULL, &pVSBlob2, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
    }

    hr = D3DX11CompileFromFile("Tree.hlsl", NULL, NULL, "PS", "ps_4_0", 0, 0, NULL, &pPSBlob2, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
    }

    m_device->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), nullptr, &treeVertexShader);
    m_device->CreatePixelShader(pPSBlob2->GetBufferPointer(), pPSBlob2->GetBufferSize(), nullptr, &treePixelShader);

    ID3DBlob* pVSBlob3;
    hr = D3DX11CompileFromFile("Tree.hlsl", NULL, NULL, "VSShadow", "vs_4_0", 0, 0, NULL, &pVSBlob3, &pErrorBlob, NULL);
    if (FAILED(hr))
    {
        MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VSShadow", MB_OK | MB_ICONERROR);
    }
    
    m_device->CreateVertexShader(pVSBlob3->GetBufferPointer(), pVSBlob3->GetBufferSize(), nullptr, &shadowVertexShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = 
    {
        // ���_�f�[�^
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   1, DXGI_FORMAT_R32_FLOAT,       0, 32,  D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Weight
        { "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },

        // �C���X�^���X�f�[�^
        { "POSITION",   1, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,   D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // OffsetPosition
        { "TEXCOORD",   2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // Rotation (Quaternion)
        { "TEXCOORD",   3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // initialBillboardRot (�����r���{�[�h��])
        { "TEXCOORD",   4, DXGI_FORMAT_R32_FLOAT,          1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // Scale
        { "TEXCOORD",   5, DXGI_FORMAT_R32_FLOAT,          1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // Type
    };

    m_device->CreateInputLayout(
        layoutDesc,
        ARRAYSIZE(layoutDesc),
        pVSBlob2->GetBufferPointer(),
        pVSBlob2->GetBufferSize(),
        &instanceInputLayout);

    pVSBlob->Release();
    pVSBlob2->Release();
    pVSBlob3->Release();
    pPSBlob->Release();
    pPSBlob2->Release();
}

OctreeNode* Environment::GenerateOctree(const SimpleArray<Triangle*>& triangles, BOUNDING_BOX boundingBox)
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

bool Environment::GenerateRandomInstances(EnvironmentObjectType type, const SkinnedMeshModel* fieldModel, int clusterCount)
{
    // ���I�u�W�F�N�g����
    EnvironmentObject* obj = nullptr;
    for (UINT i = 0; i < environmentObjects.getSize(); ++i)
	{
        // ���I�u�W�F�N�g�����������ꍇ�A�I�u�W�F�N�g��ݒ�
		if (environmentObjects[i]->type == type)
		{
			obj = environmentObjects[i];
            obj->attributes.use = true;
			break;
		}
	}

    // ���I�u�W�F�N�g��������Ȃ��ꍇ�A�V�K�쐬
    if (obj == nullptr)
    {
        obj = InitializeEnvironmentObj(type);
        if (obj == nullptr)
			return false;

        obj->attributes.use = true;
    }
    
    SimpleArray<InstanceData> instanceDataArray;

    const SimpleArray<Triangle*>& triangles = fieldModel->GetTriangles();
    BOUNDING_BOX boundingBox = fieldModel->GetBoundingBox();
    OctreeNode* octree = GenerateOctree(triangles, boundingBox);

    for (int i = 0; i < clusterCount; ++i)
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
        int numInstance = static_cast<int>(clusterRadius * GetRandFloat(0.015f, 0.025f)); // �N���X�^�[�T�C�Y�ɉ��������x

        // �N���X�^�[�͈͂��`���A�����؂Ō���
        BOUNDING_BOX clusterBox
        (
            XMFLOAT3(clusterCenter.x - clusterRadius, octree->boundary.minPoint.y, clusterCenter.z - clusterRadius),
            XMFLOAT3(clusterCenter.x + clusterRadius, octree->boundary.maxPoint.y, clusterCenter.z + clusterRadius)
        );
        SimpleArray<Triangle*> nearbyTriangles;
        octree->queryRange(clusterBox, nearbyTriangles);

        if (nearbyTriangles.getSize() == 0) continue; // �N���X�^�[���ɎO�p�`���Ȃ��ꍇ�X�L�b�v

        for (int j = 0; j < numInstance; ++j)
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
            BOUNDING_BOX instanceBoundingBox;
            instanceBoundingBox.maxPoint = XMFLOAT3(pos.x + 0.1f, octree->boundary.maxPoint.y, pos.z + 0.1f);
            instanceBoundingBox.minPoint = XMFLOAT3(pos.x - 0.1f, octree->boundary.minPoint.y, pos.z - 0.1f);
            // �����؂Ŏ擾�����O�p�`�̒�����ł��������̂�I��
            const Triangle* bestTriangle = nullptr;
            XMFLOAT3 rayDir = { 0.0f, -1.0f, 0.0f };
            float maxHeight = -FLT_MAX;
            for (UINT k = 0; k < nearbyTriangles.getSize(); ++k)
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

            // �I�u�W�F�N�g�� Y ���W���ō��_�ɒ���
            pos.y = maxHeight;

            XMFLOAT4 initRotation;
            if (!GetInitRotation(obj, initRotation, bestTriangle))
                continue;

            XMFLOAT4 initialBillboardRot = XMFLOAT4(0, 0, 0, 1);
            if (obj->attributes.billboard)
            {
                // �r���{�[�h�I�u�W�F�N�g�̏ꍇ�A������]��ۑ�
                initialBillboardRot = initRotation;
            }
            else
            {
                // �r���{�[�h�I�u�W�F�N�g�łȂ��ꍇ�A�����_���� Y ����]��ǉ�
                float randomYAngle = GetRandFloat(0.0f, XM_2PI);
                XMMATRIX randomYRotation = XMMatrixRotationY(randomYAngle);

                XMMATRIX finalRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&initRotation)) * randomYRotation;
                XMVECTOR quaternion = XMQuaternionRotationMatrix(finalRotation);
                XMStoreFloat4(&initRotation, quaternion);
            }

            // �I�u�W�F�N�g�̃X�P�[�� (�����_����)
            float scl = GetRandomScale(type);

            // �C���X�^���X�f�[�^��ǉ�
            instanceDataArray.push_back(
                InstanceData(
                    pos,                            // �I�u�W�F�N�g�̈ʒu
                    initRotation,                   // �I�u�W�F�N�g�̉�] (�@�������Ɋ�Â�)
                    initialBillboardRot,            // �r���{�[�h�I�u�W�F�N�g�̏�����]
                    scl,					        // �I�u�W�F�N�g�̃X�P�[��
                    static_cast<float>(obj->type)   // �I�u�W�F�N�g�̎��
                ));
        }
    }

    SAFE_DELETE(octree);

    if (!CreateInstanceBuffer(obj, instanceDataArray))
        return false;

    return true;

}

bool Environment::GenerateInstanceByParams(const InstanceParams& params, const SkinnedMeshModel* fieldModel)
{
    EnvironmentObject* obj = InitializeEnvironmentObj(params.type);
    if (obj == nullptr)
        return false;
    obj->attributes.use = true;

    SimpleArray<InstanceData> instanceDataArray;

    const SimpleArray<Triangle*>& triangles = fieldModel->GetTriangles();
    BOUNDING_BOX boundingBox = fieldModel->GetBoundingBox();
    OctreeNode* octree = GenerateOctree(triangles, boundingBox);

    UINT numInstance = params.transformArray.getSize();

    for (UINT i = 0; i < numInstance; i++)
    {
        XMFLOAT3 pos =
        {
            params.transformArray[i].posX,
            octree->boundary.maxPoint.y, // �����ʒu���烌�C���������ɓ���
            params.transformArray[i].posZ
        };

        BOUNDING_BOX instanceBoundingBox;
        instanceBoundingBox.maxPoint = XMFLOAT3(pos.x + 0.1f, octree->boundary.maxPoint.y, pos.z + 0.1f);
        instanceBoundingBox.minPoint = XMFLOAT3(pos.x - 0.1f, octree->boundary.minPoint.y, pos.z - 0.1f);

        SimpleArray<Triangle*> nearbyTriangles;
        octree->queryRange(instanceBoundingBox, nearbyTriangles);

        // �����؂Ŏ擾�����O�p�`�̒�����ł��������̂�I��
        const Triangle* bestTriangle = nullptr;
        XMFLOAT3 rayDir = { 0.0f, -1.0f, 0.0f };
        float maxHeight = -FLT_MAX;
        for (UINT k = 0; k < nearbyTriangles.getSize(); ++k)
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

        // �I�u�W�F�N�g�� Y ���W���ō��_�ɒ���
        pos.y = maxHeight;// +(instanceBoundingBox.maxPoint.y - instanceBoundingBox.minPoint.y);

        XMFLOAT4 initRotation;
        if (!GetInitRotation(obj, initRotation, bestTriangle))
            continue;

        XMFLOAT4 initialBillboardRot = XMFLOAT4(0, 0, 0, 1);
        if (obj->attributes.billboard)
        {
            // �r���{�[�h�I�u�W�F�N�g�̏ꍇ�A������]��ۑ�
            initialBillboardRot = initRotation;
        }
        else if (params.randomRotY)
        {
            // �r���{�[�h�I�u�W�F�N�g�łȂ��ꍇ�A�����_���� Y ����]��ǉ�
            float randomYAngle = GetRandFloat(0.0f, XM_2PI);
            XMMATRIX randomYRotation = XMMatrixRotationY(randomYAngle);

            XMMATRIX finalRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&initRotation)) * randomYRotation;
            XMVECTOR quaternion = XMQuaternionRotationMatrix(finalRotation);
            XMStoreFloat4(&initRotation, quaternion);
        }
        else
        {
            float rotationY = params.transformArray[i].rotY;
            XMVECTOR quaternion = AddRotationToQuaternion(XMLoadFloat4(&initRotation), AXIS_Y, rotationY);
            XMStoreFloat4(&initRotation, quaternion);
        }

        float scl; 
        if (params.randomScl)
            // �I�u�W�F�N�g�̃X�P�[�� (�����_����)
            scl = GetRandomScale(obj->type);
        else
            scl = params.transformArray[i].scl;

        // �C���X�^���X�f�[�^��ǉ�
        instanceDataArray.push_back(
            InstanceData(
                pos,                            // �I�u�W�F�N�g�̈ʒu
                initRotation,                   // �I�u�W�F�N�g�̉�] (�@�������Ɋ�Â�)
                initialBillboardRot,            // �r���{�[�h�I�u�W�F�N�g�̏�����]
                scl,					        // �I�u�W�F�N�g�̃X�P�[��
                static_cast<float>(obj->type)   // �I�u�W�F�N�g�̎��
            ));
    }

    SAFE_DELETE(octree);

    if (!CreateInstanceBuffer(obj, instanceDataArray))
        return false;

    return true;
}

bool Environment::GetInitRotation(EnvironmentObject* obj, XMFLOAT4& rotation, const Triangle* triangle)
{
    // �n�`�ɓK������Ă���ꍇ�A�n�`�ɍ��킹�ĉ�]
    if (obj->attributes.adaptedToTerrain)
    {
        // �@�������]�N�H�[�^�j�I�����v�Z
        XMVECTOR normalVec = XMLoadFloat3(&triangle->normal);

        // �f�t�H���g�̏�����x�N�g�� (Y ��)
        XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);

        // �@�������łɏ�����������Ă���ꍇ�A��]��K�p���Ȃ�
        float dotProduct = XMVectorGetX(XMVector3Dot(upVec, normalVec));
        float angle = acosf(dotProduct); // ���W�A���P�ʂ̊p�x

        // �����p�x���ő勖�e�l�𒴂����ꍇ�A�������Ȃ�
        if (angle > obj->attributes.maxSlopeAngle)
            return false;

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

            XMStoreFloat4(&rotation, XMVector4Normalize(quaternion));
        }
    }
    else
    {
        rotation = XMFLOAT4(0, 0, 0, 1); // ��]�Ȃ�
    }

    return true;
}

float Environment::GetRandomScale(EnvironmentObjectType type)
{
    float scl;

    switch (type)
    {
    case EnvironmentObjectType::Grass_1:
        scl = GetRandFloat(20, 40);
        return scl;
    case EnvironmentObjectType::Grass_2:
        scl = GetRandFloat(0.1f, 0.2f);
        return scl;
    case EnvironmentObjectType::Clover_1:
        scl = GetRandFloat(0.1f, 0.2f);
        return scl;
    case EnvironmentObjectType::Bush_1:
        scl = GetRandFloat(0.5, 1.0f);
        return scl;
    case EnvironmentObjectType::Flower_1:
        scl = GetRandFloat(0.1f, 0.2f);
        return scl;
    case EnvironmentObjectType::Shrubbery_1:
        scl = GetRandFloat(0.1f, 0.2f);
        return scl;
    case EnvironmentObjectType::Tree_1:
        scl = GetRandFloat(0.1f, 0.2f);
        return scl;
    default:
        scl = 1.0f;
        return scl;
    }
}

bool Environment::CreateInstanceBuffer(EnvironmentObject* obj, const SimpleArray<InstanceData>& instanceDataArray)
{
    obj->instanceCount = instanceDataArray.getSize();
    obj->instanceData = new InstanceData[obj->instanceCount];
    for (int i = 0; i < obj->instanceCount; i++)
        obj->instanceData[i] = instanceDataArray[i];

    // �o�b�t�@�L�q�q�ݒ�
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // �p�ɂȍX�V���K�v�Ȃ��� DYNAMIC �g�p
    bufferDesc.ByteWidth = sizeof(InstanceData) * obj->instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU �A�N�Z�X�\

    // �����f�[�^�ݒ�
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = obj->instanceData;

    // �C���X�^���X�o�b�t�@�쐬
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &obj->instanceBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

XMVECTOR Environment::AddRotationToQuaternion(const XMVECTOR& baseRotation, RotationAxis axis, float angle)
{
    // ���̃N�H�[�^�j�I���𐳋K��
    XMVECTOR normalizedBase = XMQuaternionNormalize(baseRotation);

    // ��]���x�N�g��������
    XMVECTOR axisVector;
    switch (axis)
    {
    case AXIS_X:
        axisVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        break;
    case AXIS_Y:
        axisVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        break;
    case AXIS_Z:
        axisVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        break;
    default:
        // �f�t�H���g��Y��
        axisVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        break;
    }

    // ��]���Ɗp�x����ǉ��̉�]�N�H�[�^�j�I�����쐬
    XMVECTOR additionalRotation = XMQuaternionRotationAxis(axisVector, angle);

    // ��]�������i���Ԃɒ��ӁF���base�A���̌�ǉ��j
    XMVECTOR result = XMQuaternionMultiply(additionalRotation, normalizedBase);

    // ���ʂ𐳋K�����ĕԂ�
    return XMQuaternionNormalize(result);
}
