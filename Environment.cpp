//=============================================================================
//
// 環境の描画 [Environment.cpp]
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
    time = 0.0f; // 累積時間

    Initialize();
    CompileShaders();
}

Environment::Environment(EnvironmentConfig config) :
    m_device(Renderer::get_instance().GetDevice()),
    m_context(Renderer::get_instance().GetDeviceContext())
{
	time = 0.0f; // 累積時間

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
    // サンプラーステート作成
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    m_device->CreateSamplerState(&samplerDesc, &samplerState);

    // シャドウ用サンプラー (比較サンプラー)
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

    // ノイズテクスチャ生成
    if (!GenerateNoiseTexture(&noiseTextureSRV))
        return false;

    // 定数バッファ作成
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
    time++; // 時間更新

    // 定数バッファマッピング
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    PerFrameBuffer* bufferData = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);

    bufferData->Time = time * 0.0005f; // 時間設定
    bufferData->WindDirection = CalculateDynamicWindDirection(time); // 動的風向き計算
    bufferData->WindStrength = 0.2f + 0.05f * sinf(time * 0.5f); // 風強さ変動

    m_context->Unmap(perFrameBuffer, 0); // バッファアンマップ

    // ビルボードオブジェクトの回転行列計算
    for (UINT i = 0; i < environmentObjects.getSize(); i++)
    {
        EnvironmentObject* obj = environmentObjects[i];
        if (obj->attributes.billboard)
        {
            Camera& camera = Camera::get_instance();
            XMMATRIX mtxView = XMLoadFloat4x4(&camera.GetViewMatrix());

            // 正方行列（直交行列）を転置行列させて逆行列を作ってる版(速い)
            XMMATRIX billboardRotation = XMMatrixIdentity();
            billboardRotation.r[0] = XMVectorSet(mtxView.r[0].m128_f32[0], mtxView.r[1].m128_f32[0], mtxView.r[2].m128_f32[0], 0.0f);
            billboardRotation.r[1] = XMVectorSet(mtxView.r[0].m128_f32[1], mtxView.r[1].m128_f32[1], mtxView.r[2].m128_f32[1], 0.0f);
            billboardRotation.r[2] = XMVectorSet(mtxView.r[0].m128_f32[2], mtxView.r[1].m128_f32[2], mtxView.r[2].m128_f32[2], 0.0f);

            // インスタンスデータの回転行列更新
            for (int j = 0; j < obj->instanceCount; j++)
            {
                XMVECTOR quat = XMQuaternionRotationMatrix(
                    XMMatrixRotationQuaternion(XMLoadFloat4(&obj->instanceData[j].initialBillboardRot)) * billboardRotation);

                XMStoreFloat4(&obj->instanceData[j].Rotation, quat);
            }

            // インスタンスバッファマッピング
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
    // 影描画モードの場合、影を生成しないオブジェクトは描画しない
    if (!obj->attributes.castShadow &&
        Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE_SHADOW)
        return;

    UINT strides[2] = { sizeof(InstanceVertex), sizeof(InstanceData) }; // 頂点 & インスタンスストライド
    UINT offsets[2] = { 0, 0 }; // オフセット初期化
    ID3D11Buffer* buffers[2] = { obj->vertexBuffer, obj->instanceBuffer }; // バッファ配列

    m_context->IASetInputLayout(instanceInputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // 頂点バッファとインスタンスバッファ設定
    m_context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    // インデックスバッファ設定
    m_context->IASetIndexBuffer(obj->indexBuffer, DXGI_FORMAT_R32_UINT, 0); 
    
    m_context->VSSetConstantBuffers(12, 1, &perFrameBuffer);

    if (Renderer::get_instance().GetRenderMode() == RenderMode::INSTANCE)
    {
        m_context->VSSetShader(obj->vertexShader, nullptr, 0);

        // 風に影響される場合、ノイズテクスチャとピクセルシェーダーを設定
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
        // インスタンス化描画実行
        m_context->DrawIndexedInstanced(obj->indexCount, obj->instanceCount, 0, 0, 0);
    }
    else
    {
        const SUBSET* subset = obj->model->GetModel()->GetSubset();
        unsigned int subsetNum = obj->model->GetModel()->GetSubNum();

        for (unsigned int i = 0; i < subsetNum; i++)
        {
            // マテリアル設定
            if (subset[i].Material.MaterialData.LoadMaterial)
                Renderer::get_instance().SetMaterial(subset[i].Material.MaterialData);

            // テクスチャ設定
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

            // インスタンス化描画実行
            m_context->DrawIndexedInstanced(subset[i].IndexNum, obj->instanceCount, subset[i].StartIndex, 0, 0);
        }
    }
}

XMFLOAT3 Environment::CalculateDynamicWindDirection(float time)
{
    float angle = sinf(time * 0.3f) * XM_PIDIV4; // 時間に基づき風向き周期的変化
    return XMFLOAT3(cosf(angle), 0.0f, sinf(angle));
}

bool Environment::GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV)
{
    const int width = 256, height = 256; // ノイズテクスチャサイズ
    unsigned char noiseData[width * height];

    // 簡易ノイズ生成 (サイン波で擬似ノイズ生成)
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            float nx = static_cast<float>(x) / width;
            float ny = static_cast<float>(y) / height;
            float value = 0.5f * (sinf(nx * 10.0f) + cosf(ny * 10.0f)); // サイン波とコサイン波でノイズ生成
            noiseData[y * width + x] = static_cast<unsigned char>((value + 1.0f) * 127.5f); // 0~255 にマッピング
        }
    }

    // テクスチャ記述子設定
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8_UNORM;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.SampleDesc.Count = 1;

    // 初期データ設定
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = noiseData;
    initData.SysMemPitch = width;

    // テクスチャ生成
    ID3D11Texture2D* noiseTexture = nullptr;
    HRESULT hr = m_device->CreateTexture2D(&texDesc, &initData, &noiseTexture);
    if (FAILED(hr))
        return false;

    // シェーダーリソースビュー作成
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
    return powf(normalizedHeight, 0.01f); // 高さの影響をより強調
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
    //obj->model->SetIsInstanced(true); // インスタンス化フラグを立てる

    InstanceVertex* VertexArray = new InstanceVertex[modelData->VertexNum];

    for (unsigned int i = 0; i < modelData->VertexNum; i++)
    {
        VertexArray[i].Position = modelData->VertexArray[i].Position;
        VertexArray[i].Normal = modelData->VertexArray[i].Normal;
        VertexArray[i].Tangent = modelData->VertexArray[i].Tangent;
        VertexArray[i].TexCoord = modelData->VertexArray[i].TexCoord;

        // 風に影響される場合、重みを計算
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
    // 頂点バッファ生成
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

    // インデックスバッファ生成
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

    // シェーダーコンパイル
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
        // 頂点データ
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   1, DXGI_FORMAT_R32_FLOAT,       0, 32,  D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Weight
        { "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },

        // インスタンスデータ
        { "POSITION",   1, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,   D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // OffsetPosition
        { "TEXCOORD",   2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }, // Rotation (Quaternion)
        { "TEXCOORD",   3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },  // initialBillboardRot (初期ビルボード回転)
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
    // 環境オブジェクト検索
    EnvironmentObject* obj = nullptr;
    for (UINT i = 0; i < environmentObjects.getSize(); ++i)
	{
        // 環境オブジェクトが見つかった場合、オブジェクトを設定
		if (environmentObjects[i]->type == type)
		{
			obj = environmentObjects[i];
            obj->attributes.use = true;
			break;
		}
	}

    // 環境オブジェクトが見つからない場合、新規作成
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
        // クラスターの中心を地形全体からランダムに選択
        XMFLOAT3 clusterCenter = 
        {
            GetRandFloat(octree->boundary.minPoint.x, octree->boundary.maxPoint.x),
            0,
            GetRandFloat(octree->boundary.minPoint.z, octree->boundary.maxPoint.z)
        };

        // クラスターのサイズをランダム化 (半径範囲を設定)
        float clusterRadius = GetRandFloat(3500.0f, 13000.0f);
        int numInstance = static_cast<int>(clusterRadius * GetRandFloat(0.015f, 0.025f)); // クラスターサイズに応じた密度

        // クラスター範囲を定義し、八分木で検索
        BOUNDING_BOX clusterBox
        (
            XMFLOAT3(clusterCenter.x - clusterRadius, octree->boundary.minPoint.y, clusterCenter.z - clusterRadius),
            XMFLOAT3(clusterCenter.x + clusterRadius, octree->boundary.maxPoint.y, clusterCenter.z + clusterRadius)
        );
        SimpleArray<Triangle*> nearbyTriangles;
        octree->queryRange(clusterBox, nearbyTriangles);

        if (nearbyTriangles.getSize() == 0) continue; // クラスター内に三角形がない場合スキップ

        for (int j = 0; j < numInstance; ++j)
        {
            // クラスター範囲内のランダムな点を生成
            float randomaAngle = GetRandFloat(0.0f, XM_2PI);
            float randomRadius = GetRandFloat(0.0f, clusterRadius);
            XMFLOAT3 pos = 
            {
                clusterCenter.x + cosf(randomaAngle) * randomRadius,
                octree->boundary.maxPoint.y, // 高い位置からレイを下向きに投射
                clusterCenter.z + sinf(randomaAngle) * randomRadius
            };
            BOUNDING_BOX instanceBoundingBox;
            instanceBoundingBox.maxPoint = XMFLOAT3(pos.x + 0.1f, octree->boundary.maxPoint.y, pos.z + 0.1f);
            instanceBoundingBox.minPoint = XMFLOAT3(pos.x - 0.1f, octree->boundary.minPoint.y, pos.z - 0.1f);
            // 八分木で取得した三角形の中から最も高いものを選択
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

            // オブジェクトの Y 座標を最高点に調整
            pos.y = maxHeight;

            XMFLOAT4 initRotation;
            if (!GetInitRotation(obj, initRotation, bestTriangle))
                continue;

            XMFLOAT4 initialBillboardRot = XMFLOAT4(0, 0, 0, 1);
            if (obj->attributes.billboard)
            {
                // ビルボードオブジェクトの場合、初期回転を保存
                initialBillboardRot = initRotation;
            }
            else
            {
                // ビルボードオブジェクトでない場合、ランダムな Y 軸回転を追加
                float randomYAngle = GetRandFloat(0.0f, XM_2PI);
                XMMATRIX randomYRotation = XMMatrixRotationY(randomYAngle);

                XMMATRIX finalRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&initRotation)) * randomYRotation;
                XMVECTOR quaternion = XMQuaternionRotationMatrix(finalRotation);
                XMStoreFloat4(&initRotation, quaternion);
            }

            // オブジェクトのスケール (ランダム化)
            float scl = GetRandomScale(type);

            // インスタンスデータを追加
            instanceDataArray.push_back(
                InstanceData(
                    pos,                            // オブジェクトの位置
                    initRotation,                   // オブジェクトの回転 (法線方向に基づく)
                    initialBillboardRot,            // ビルボードオブジェクトの初期回転
                    scl,					        // オブジェクトのスケール
                    static_cast<float>(obj->type)   // オブジェクトの種類
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
            octree->boundary.maxPoint.y, // 高い位置からレイを下向きに投射
            params.transformArray[i].posZ
        };

        BOUNDING_BOX instanceBoundingBox;
        instanceBoundingBox.maxPoint = XMFLOAT3(pos.x + 0.1f, octree->boundary.maxPoint.y, pos.z + 0.1f);
        instanceBoundingBox.minPoint = XMFLOAT3(pos.x - 0.1f, octree->boundary.minPoint.y, pos.z - 0.1f);

        SimpleArray<Triangle*> nearbyTriangles;
        octree->queryRange(instanceBoundingBox, nearbyTriangles);

        // 八分木で取得した三角形の中から最も高いものを選択
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

        // オブジェクトの Y 座標を最高点に調整
        pos.y = maxHeight;// +(instanceBoundingBox.maxPoint.y - instanceBoundingBox.minPoint.y);

        XMFLOAT4 initRotation;
        if (!GetInitRotation(obj, initRotation, bestTriangle))
            continue;

        XMFLOAT4 initialBillboardRot = XMFLOAT4(0, 0, 0, 1);
        if (obj->attributes.billboard)
        {
            // ビルボードオブジェクトの場合、初期回転を保存
            initialBillboardRot = initRotation;
        }
        else if (params.randomRotY)
        {
            // ビルボードオブジェクトでない場合、ランダムな Y 軸回転を追加
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
            // オブジェクトのスケール (ランダム化)
            scl = GetRandomScale(obj->type);
        else
            scl = params.transformArray[i].scl;

        // インスタンスデータを追加
        instanceDataArray.push_back(
            InstanceData(
                pos,                            // オブジェクトの位置
                initRotation,                   // オブジェクトの回転 (法線方向に基づく)
                initialBillboardRot,            // ビルボードオブジェクトの初期回転
                scl,					        // オブジェクトのスケール
                static_cast<float>(obj->type)   // オブジェクトの種類
            ));
    }

    SAFE_DELETE(octree);

    if (!CreateInstanceBuffer(obj, instanceDataArray))
        return false;

    return true;
}

bool Environment::GetInitRotation(EnvironmentObject* obj, XMFLOAT4& rotation, const Triangle* triangle)
{
    // 地形に適応されている場合、地形に合わせて回転
    if (obj->attributes.adaptedToTerrain)
    {
        // 法線から回転クォータニオンを計算
        XMVECTOR normalVec = XMLoadFloat3(&triangle->normal);

        // デフォルトの上方向ベクトル (Y 軸)
        XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);

        // 法線がすでに上方向を向いている場合、回転を適用しない
        float dotProduct = XMVectorGetX(XMVector3Dot(upVec, normalVec));
        float angle = acosf(dotProduct); // ラジアン単位の角度

        // もし角度が最大許容値を超えた場合、生成しない
        if (angle > obj->attributes.maxSlopeAngle)
            return false;

        // 法線がすでに上方向を向いている場合、回転を適用しない
        if (XMVector3Equal(normalVec, upVec))
        {
            rotation = XMFLOAT4(0, 0, 0, 1);
        }
        else
        {
            // 回転軸を計算（上方向ベクトルと法線の外積）
            XMVECTOR rotationAxis = XMVector3Normalize(XMVector3Cross(upVec, normalVec));

            // 回転角を計算（上方向ベクトルと法線の間の角度）
            float rotationAngle = acosf(XMVectorGetX(XMVector3Dot(upVec, normalVec)));

            // クォータニオンを生成
            XMVECTOR quaternion = XMQuaternionRotationAxis(rotationAxis, rotationAngle);

            XMStoreFloat4(&rotation, XMVector4Normalize(quaternion));
        }
    }
    else
    {
        rotation = XMFLOAT4(0, 0, 0, 1); // 回転なし
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

    // バッファ記述子設定
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // 頻繁な更新が必要なため DYNAMIC 使用
    bufferDesc.ByteWidth = sizeof(InstanceData) * obj->instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // CPU アクセス可能

    // 初期データ設定
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = obj->instanceData;

    // インスタンスバッファ作成
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &obj->instanceBuffer);
    if (FAILED(hr))
        return false;

    return true;
}

XMVECTOR Environment::AddRotationToQuaternion(const XMVECTOR& baseRotation, RotationAxis axis, float angle)
{
    // 元のクォータニオンを正規化
    XMVECTOR normalizedBase = XMQuaternionNormalize(baseRotation);

    // 回転軸ベクトルを決定
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
        // デフォルトはY軸
        axisVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        break;
    }

    // 回転軸と角度から追加の回転クォータニオンを作成
    XMVECTOR additionalRotation = XMQuaternionRotationAxis(axisVector, angle);

    // 回転を合成（順番に注意：先にbase、その後追加）
    XMVECTOR result = XMQuaternionMultiply(additionalRotation, normalizedBase);

    // 結果を正規化して返す
    return XMQuaternionNormalize(result);
}
