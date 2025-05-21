//=============================================================================
//
// ���̕`�� [Environment.cpp]
// Author : 
//
//=============================================================================
#include "Scene/Environment.h"
#include "Core/Camera.h"
#include "Core/TextureMgr.h"
#include "Core/Graphics/ShadowMeshCollector.hpp"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define DRAW_BOUNDING_BOX       1

Environment::Environment() : 
    m_device(Renderer::get_instance().GetDevice()), 
    m_context(Renderer::get_instance().GetDeviceContext())
{
    m_time = 0.0f; // �ݐώ���

    Initialize();
}

Environment::Environment(EnvironmentConfig config) :
    m_device(Renderer::get_instance().GetDevice()),
    m_context(Renderer::get_instance().GetDeviceContext())
{
    m_time = 0.0f; // �ݐώ���

	Initialize();

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
#ifdef _DEBUG
    DebugProc::get_instance().Register(this);
#endif // DEBUG

    // �m�C�Y�e�N�X�`������
    if (!GenerateNoiseTexture(&noiseTextureSRV))
        return false;

    // �萔�o�b�t�@�쐬
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(PerFrameBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_perFrameBuffer);
    if (FAILED(hr))
        return false;

	// �V�F�[�_�[�̓ǂݍ���
	if (!LoadAllShaders())
		return false;

#if DRAW_BOUNDING_BOX
    m_debugBoundingBoxRenderer.Initialize();
#endif

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
		obj->attributes.isDynamic = false;
        obj->attributes.maxSlopeAngle = GRASS_MAX_SLOPE_ANGLE;
        obj->modelPath = GRASS_1_MODEL_PATH;
        obj->extDiffuseTexPath = GRASS_1_TEX_PATH;
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
        obj->attributes.isDynamic = false;
        obj->attributes.maxSlopeAngle = GRASS_MAX_SLOPE_ANGLE;
        obj->modelPath = GRASS_2_MODEL_PATH;
        obj->extDiffuseTexPath = GRASS_2_TEX_PATH;
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
        obj->attributes.isDynamic = false;
        obj->attributes.maxSlopeAngle = CLOVER_MAX_SLOPE_ANGLE;
        obj->modelPath = CLOVER_1_MODEL_PATH;
        obj->extDiffuseTexPath = CLOVER_1_TEX_PATH;
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
        obj->attributes.isDynamic = false;
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
        obj->attributes.isDynamic = false;
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
        obj->attributes.isDynamic = false;
        obj->attributes.maxSlopeAngle = FLOWER_MAX_SLOPE_ANGLE;
        obj->modelPath = FLOWER_1_MODEL_PATH;
        obj->extDiffuseTexPath = FLOWER_1_TEX_PATH;
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
        obj->attributes.isDynamic = false;
        obj->attributes.maxSlopeAngle = SHRUBBERY_MAX_SLOPE_ANGLE;
        obj->modelPath = SHRUBBERY_1_MODEL_PATH;
        obj->extDiffuseTexPath = SHRUBBERY_1_TEX_PATH;
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
        obj->attributes.isDynamic = false;
        obj->attributes.maxSlopeAngle = TREE_MAX_SLOPE_ANGLE;
        obj->modelPath = TREE_1_MODEL_PATH;
        obj->extDiffuseTexPath = TREE_1_TEX_PATH;
        break;
    default:
        return nullptr;
    }

    if (!LoadEnvironmentObj(obj))
    {
        SAFE_DELETE(obj);
        return nullptr;
    }

    m_environmentObjects.push_back(obj);

    return obj;
}

Environment::~Environment()
{
    for (UINT i = 0; i < m_environmentObjects.getSize(); i++)
	{
		SAFE_DELETE(m_environmentObjects[i]);
	}

    SafeRelease(&m_perFrameBuffer);
    SafeRelease(&noiseTextureSRV);
}

void Environment::Update(void)
{
    m_time++; // ���ԍX�V

    // �萔�o�b�t�@�}�b�s���O
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(m_perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    PerFrameBuffer* bufferData = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);

    bufferData->Time = m_time * 0.0005f; // ���Ԑݒ�
    bufferData->WindDirection = CalculateDynamicWindDirection(m_time); // ���I�������v�Z
    bufferData->WindStrength = 0.2f + 0.05f * sinf(m_time * 0.5f); // �������ϓ�

    m_context->Unmap(m_perFrameBuffer, 0); // �o�b�t�@�A���}�b�v

    // �r���{�[�h�I�u�W�F�N�g�̉�]�s��v�Z
    for (UINT i = 0; i < m_environmentObjects.getSize(); i++)
    {
        EnvironmentObject* obj = m_environmentObjects[i];
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
            for (UINT j = 0; j < obj->instanceCount; j++)
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

		// ���I�I�u�W�F�N�g�̃o�E���f�B���O�{�b�N�X�X�V
        if (obj->attributes.isDynamic)
            UpdateBoundingBox(obj);
    }
}

void Environment::Draw(void)
{
	if (m_shaderHotReload)
		LoadAllShaders();

    for (UINT i = 0; i < m_environmentObjects.getSize(); i++)
	{
		EnvironmentObject* obj = m_environmentObjects[i];
		if (obj->attributes.use)
		{
            Renderer::get_instance().SetCurrentWorldMatrix(&obj->transform.mtxWorld);

            RenderEnvironmentObj(obj);
		}
	}

}

void Environment::RenderEnvironmentObj(EnvironmentObject* obj)
{
    if (m_shaderHotReload)
        LoadShadersForEachObj(obj);

    UINT strides[2] = { sizeof(InstanceVertex), sizeof(InstanceData) }; // ���_ & �C���X�^���X�X�g���C�h
    UINT offsets[2] = { 0, 0 }; // �I�t�Z�b�g������
    ID3D11Buffer* buffers[2] = { obj->vertexBuffer, obj->instanceBuffer }; // �o�b�t�@�z��

    m_context->IASetInputLayout(m_treeShaderSet.inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ���_�o�b�t�@�ƃC���X�^���X�o�b�t�@�ݒ�
    m_context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    // �C���f�b�N�X�o�b�t�@�ݒ�
    m_context->IASetIndexBuffer(obj->indexBuffer, DXGI_FORMAT_R32_UINT, 0); 
    m_ShaderResourceBinder.BindConstantBuffer(ShaderStage::VS, SLOT_CB_INSTANCED_DATA, m_perFrameBuffer);

    m_context->VSSetShader(obj->vertexShader, nullptr, 0);

    // ���ɉe�������ꍇ�A�m�C�Y�e�N�X�`���ƃs�N�Z���V�F�[�_�[��ݒ�
    if (obj->attributes.affectedByWind)
        m_ShaderResourceBinder.BindShaderResource(ShaderStage::VS, SLOT_TEX_NOISE, noiseTextureSRV);

    m_context->PSSetShader(obj->pixelShader, nullptr, 0);

    if (obj->extDiffuseTexPath)
    {
        MATERIAL material;
        ZeroMemory(&material, sizeof(material));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        Renderer::get_instance().SetMaterial(material);
        m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, obj->externDiffuseTextureSRV);
        // �C���X�^���X���`����s
        m_context->DrawIndexedInstanced(obj->indexCount, obj->instanceCount, 0, 0, 0);
    }
    else
    {
        const SUBSET* subset = obj->modelGO->GetModel()->GetSubset();
        unsigned int subsetNum = obj->modelGO->GetModel()->GetSubNum();

        for (unsigned int i = 0; i < subsetNum; i++)
        {
            // �}�e���A���ݒ�
            if (subset[i].Material.MaterialData.LoadMaterial)
                Renderer::get_instance().SetMaterial(subset[i].Material.MaterialData);

            // �e�N�X�`���ݒ�
            if (subset[i].Material.MaterialData.noTexSampling == 0)
            {
                m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, subset[i].diffuseTexture);
            }
            if (subset[i].Material.MaterialData.normalMapSampling == 1)
            {
                m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_NORMAL, subset[i].normalTexture);
            }
            if (subset[i].Material.MaterialData.bumpMapSampling == 1)
            {
                m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_BUMP, subset[i].bumpTexture);
            }
            if (subset[i].Material.MaterialData.opacityMapSampling == 1)
            {
                m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_OPACITY, subset[i].opacityTexture);
            }
            if (subset[i].Material.MaterialData.reflectMapSampling == 1)
            {
                m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_REFLECT, subset[i].reflectTexture);
            }
            if (subset[i].Material.MaterialData.translucencyMapSampling == 1)
            {
                m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_TRANSLUCENCY, subset[i].translucencyTexture);
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
    obj->modelGO = new GameObject<ModelInstance>();
    obj->modelGO->Instantiate(obj->modelPath);
	obj->modelGO->SetModelType(ModelType::Instanced);

    const SUBSET* subset = obj->modelGO->GetModel()->GetSubset();
    unsigned int subsetNum = obj->modelGO->GetModel()->GetSubNum();

	obj->boundingBoxLocal = obj->modelGO->GetModel()->GetBoundingBox();

    obj->useExtDiffuseTex = true;
    for (unsigned int i = 0; i < subsetNum; i++)
	{
        // �e�N�X�`���̗L�����m�F
		if (subset[i].diffuseTexture != nullptr)
		{
			obj->useExtDiffuseTex = false;
		}

        if (subset[i].opacityTexture != nullptr)
            obj->modelGO->SetEnableAlphaTest(true);
	}

    // �e�N�X�`���̓ǂݍ���
    if (obj->useExtDiffuseTex && obj->extDiffuseTexPath)
	{
		obj->externDiffuseTextureSRV = TextureMgr::get_instance().CreateTexture(obj->extDiffuseTexPath);
		if (obj->externDiffuseTextureSRV == nullptr)
			return false;
	}

    const MODEL_DATA* modelData = obj->modelGO->GetModel()->GetModelData();
    BOUNDING_BOX boudningBox = obj->modelGO->GetModel()->GetBoundingBox();
    obj->modelGO->SetIsInstanced(true); // �C���X�^���X���t���O�𗧂Ă�

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

    obj->modelGO->SetVertexBuffer(obj->vertexBuffer);
    obj->modelGO->SetIndexBuffer(obj->indexBuffer);

    LoadShadersForEachObj(obj);

    return true;
}

void Environment::LoadShadersForEachObj(EnvironmentObject* obj)
{
	if (obj->attributes.affectedByWind)
	{
        obj->vertexShader = m_grassShaderSet.vs;
        obj->pixelShader = m_grassShaderSet.ps;
	}
	else
	{
        obj->vertexShader = m_treeShaderSet.vs;
        obj->pixelShader = m_treeShaderSet.ps;
	}
}

bool Environment::LoadAllShaders(void)
{
	bool loadSuccessful = true;

    loadSuccessful &= ShaderManager::get_instance().HasShaderSet(ShaderSetID::Instanced_Grass);
    if (loadSuccessful)
        m_grassShaderSet = ShaderManager::get_instance().GetShaderSet(ShaderSetID::Instanced_Grass);
    loadSuccessful &= ShaderManager::get_instance().HasShaderSet(ShaderSetID::Instanced_Tree);
    if (loadSuccessful)
        m_treeShaderSet = ShaderManager::get_instance().GetShaderSet(ShaderSetID::Instanced_Tree);

    return loadSuccessful;
}

OctreeNode* Environment::GenerateOctree(const SimpleArray<Triangle*>* triangles, BOUNDING_BOX boundingBox)
{
    if (triangles == nullptr || triangles->getSize() == 0)
		return nullptr;

    OctreeNode* octree = new OctreeNode(boundingBox);

    int numTriangles = triangles->getSize();
    for (int i = 0; i < numTriangles; i++)
    {
        if (!octree->insert((*triangles)[i]))
            return nullptr;
    }

    return octree;
}

void Environment::RenderDebugInfo(void)
{
#if DRAW_BOUNDING_BOX
    if (m_drawBoundingBox)
    {
        for (UINT i = 0; i < m_environmentObjects.getSize(); i++)
        {
            EnvironmentObject* obj = m_environmentObjects[i];

            SimpleArray<Collider>* colliders = obj->modelGO->GetInstancedColliderArray();
            if (!colliders) continue;

            for (UINT j = 0; j < colliders->getSize(); j++)
            {
                const BOUNDING_BOX& box = (*colliders)[j].aabb;

                XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

                m_debugBoundingBoxRenderer.DrawBox(box, m_camera.GetViewProjMtx(), color);
            }
        }
    }
#endif
}

bool Environment::GenerateRandomInstances(EnvironmentObjectType type, const Model* fieldModel, int clusterCount)
{
    // ���I�u�W�F�N�g����
    EnvironmentObject* obj = nullptr;
    for (UINT i = 0; i < m_environmentObjects.getSize(); ++i)
	{
        // ���I�u�W�F�N�g�����������ꍇ�A�I�u�W�F�N�g��ݒ�
		if (m_environmentObjects[i]->type == type)
		{
			obj = m_environmentObjects[i];
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

    const SimpleArray<Triangle*>* triangles = fieldModel->GetTriangles();
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

    obj->modelGO->SetInstanceCollision(obj->attributes.collision); // �Փ˔���
    obj->modelGO->SetInstanceCount(obj->instanceCount); // �C���X�^���X����ݒ�
    obj->modelGO->SetInstanceData(obj->instanceData); // �C���X�^���X�f�[�^��ݒ�
    obj->modelGO->InitializeInstancedArray(); // �C���X�^���X���z���������

    // �o�E���f�B���O�{�b�N�X�X�V
    UpdateBoundingBox(obj);

    return true;

}

bool Environment::GenerateInstanceByParams(const InstanceParams& params, const SkinnedMeshModel* fieldModel, BOUNDING_BOX fieldBBox)
{
    EnvironmentObject* obj = InitializeEnvironmentObj(params.type);
    if (obj == nullptr)
        return false;
    obj->attributes.use = true;

    SimpleArray<InstanceData> instanceDataArray;

    const SimpleArray<Triangle*>* triangles = fieldModel->GetTriangles();
    OctreeNode* octree = GenerateOctree(triangles, fieldBBox);

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

	obj->modelGO->SetInstanceCollision(obj->attributes.collision); // �Փ˔���
    obj->modelGO->SetInstanceCount(obj->instanceCount); // �C���X�^���X����ݒ�
    obj->modelGO->SetInstanceData(obj->instanceData); // �C���X�^���X�f�[�^��ݒ�
	obj->modelGO->InitializeInstancedArray(); // �C���X�^���X���z���������

    // �o�E���f�B���O�{�b�N�X�X�V
    UpdateBoundingBox(obj);

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
    for (UINT i = 0; i < obj->instanceCount; i++)
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

	// �C���X�^���X�o�b�t�@�̃V���h�E�R�s�[���쐬
    hr = m_device->CreateBuffer(&bufferDesc, nullptr, &obj->instanceBufferShadow);
    if (FAILED(hr))
        return false;

    obj->modelGO->SetInstanceBuffer(obj->instanceBufferShadow); // ���f���ɃC���X�^���X�o�b�t�@��ݒ�

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

void Environment::UpdateBoundingBox(EnvironmentObject* obj)
{
    SimpleArray<Collider>* colliders = obj->modelGO->GetInstancedColliderArray();
    //colliders->clear(); // �O�̃f�[�^���N���A�i�ė��p�j
    //colliders->reserve(obj->instanceCount); // �ő�e�ʊm��

	BOUNDING_BOX boundingBoxLocal = obj->boundingBoxLocal;

    for (UINT i = 0; i < obj->instanceCount; i++)
    {
        const InstanceData& inst = obj->instanceData[i];

        // ���[�J��AABB��8�̃R�[�i�[���擾
        XMVECTOR localCorners[8];
        boundingBoxLocal.GetCorners(localCorners);

        // ��]�E�X�P�[�����O�E�I�t�Z�b�g�}�g���b�N�X���\�z
        XMMATRIX scaleMat = XMMatrixScaling(inst.Scale, inst.Scale, inst.Scale);
        XMMATRIX rotMat = XMMatrixRotationQuaternion(XMLoadFloat4(&inst.Rotation));
        XMMATRIX transMat = XMMatrixTranslation(inst.OffsetPosition.x, inst.OffsetPosition.y, inst.OffsetPosition.z);

        XMMATRIX worldMat = scaleMat * rotMat * transMat;

        // ���[���hAABB���\�z
        BOUNDING_BOX worldAABB;
        worldAABB.minPoint = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
        worldAABB.maxPoint = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (int j = 0; j < 8; ++j)
        {
            XMVECTOR v = XMVector3Transform(localCorners[j], worldMat);
            XMFLOAT3 pos;
            XMStoreFloat3(&pos, v);

            worldAABB.minPoint.x = min(worldAABB.minPoint.x, pos.x);
            worldAABB.minPoint.y = min(worldAABB.minPoint.y, pos.y);
            worldAABB.minPoint.z = min(worldAABB.minPoint.z, pos.z);

            worldAABB.maxPoint.x = max(worldAABB.maxPoint.x, pos.x);
            worldAABB.maxPoint.y = max(worldAABB.maxPoint.y, pos.y);
            worldAABB.maxPoint.z = max(worldAABB.maxPoint.z, pos.z);
        }

		// �R���C�_�[���X�V
        (*colliders)[i].aabb = worldAABB;
    }
}
