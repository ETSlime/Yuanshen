//=============================================================================
//
// 草の描画 [Grass.cpp]
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
    time = 0.0f; // 累積時間
    transform.scl = XMFLOAT3(550.0f, 550.0f, 550.0f);
    XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;
    
    // ワールドマトリックスの初期化
    mtxWorld = XMMatrixIdentity();

    // スケールを反映
    mtxScl = XMMatrixScaling(transform.scl.x, transform.scl.y, transform.scl.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

    // 回転を反映
    mtxRot = XMMatrixRotationRollPitchYaw(transform.rot.x, transform.rot.y + XM_PI, transform.rot.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

    // 移動を反映
    mtxTranslate = XMMatrixTranslation(transform.pos.x, transform.pos.y, transform.pos.z);
    mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

    LoadModel();
    LoadShaders();

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
    time++;; // 時間更新

    Renderer::get_instance().SetCurrentWorldMatrix(&transform.mtxWorld);

    // 定数バッファマッピング
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(perFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    PerFrameBuffer* bufferData = reinterpret_cast<PerFrameBuffer*>(mappedResource.pData);

    bufferData->Time = time; // 時間設定
    bufferData->WindDirection = CalculateDynamicWindDirection(time); // 動的風向き計算
    bufferData->WindStrength = 0.2f + 0.05f * sinf(time * 0.5f); // 風強さ変動

    m_context->Unmap(perFrameBuffer, 0); // バッファアンマップ

    // シャドウデプスパス実行
    RenderShadowPass();
}

void Grass::RenderShadowPass(void)
{

    UINT strides[2] = { sizeof(GrassVertex), sizeof(InstanceData) }; // 頂点 & インスタンスストライド
    UINT offsets[2] = { 0, 0 }; // オフセット初期化
    ID3D11Buffer* buffers[2] = { vertexBuffer, instanceBuffer }; // バッファ配列

    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // 頂点バッファとインスタンスバッファ設定
    m_context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    m_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0); // インデックスバッファ設定

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


    // インスタンス化描画実行
    m_context->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}

XMFLOAT3 Grass::CalculateDynamicWindDirection(float time)
{
    float angle = sinf(time * 0.3f) * XM_PIDIV4; // 時間に基づき風向き周期的変化
    return XMFLOAT3(cosf(angle), 0.0f, sinf(angle));
}

bool Grass::GenerateNoiseTexture(ID3D11ShaderResourceView** noiseTextureSRV)
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

    // 頂点バッファ生成
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(GrassVertex) * modelData->VertexNum;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = VertexArray;

    m_device->CreateBuffer(&bd, &sd, &vertexBuffer);


    // インデックスバッファ生成
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
        // 頂点データ
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,   D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",   1, DXGI_FORMAT_R32_FLOAT,       0, 32,  D3D11_INPUT_PER_VERTEX_DATA, 0 }, // Weight

        // インスタンスデータ
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
            GetRand(-100, 100) * 0.5f,  // X座標
            0.0f,                        // Y座標固定 (地面)
            GetRand(-100, 100) * 0.5f    // Z座標
        ); // ランダム配置
        instanceData[i].Scale = 0.5f + (rand() % 100) / 100.0f; // スケール: 0.5 ~ 1.5
    }

    // バッファ記述子設定
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // 頻繁な更新が必要なため DYNAMIC 使用
    bufferDesc.ByteWidth = sizeof(InstanceData) * instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // 初期データ設定
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = instanceData;

    // インスタンスバッファ作成
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
        // クラスターの中心を地形全体からランダムに選択
        XMFLOAT3 clusterCenter = 
        {
            GetRandFloat(octree->boundary.minPoint.x, octree->boundary.maxPoint.x),
            0,
            GetRandFloat(octree->boundary.minPoint.z, octree->boundary.maxPoint.z)
        };

        // クラスターのサイズをランダム化 (半径範囲を設定)
        float clusterRadius = GetRandFloat(3500.0f, 13000.0f);
        int grassCount = static_cast<int>(clusterRadius * GetRandFloat(0.015f, 0.025f)); // クラスターサイズに応じた密度

        // クラスター範囲を定義し、八分木で検索
        BOUNDING_BOX clusterBox
        (
            XMFLOAT3(clusterCenter.x - clusterRadius, octree->boundary.minPoint.y, clusterCenter.z - clusterRadius),
            XMFLOAT3(clusterCenter.x + clusterRadius, octree->boundary.maxPoint.y, clusterCenter.z + clusterRadius)
        );
        SimpleArray<Triangle*> nearbyTriangles;
        octree->queryRange(clusterBox, nearbyTriangles);

        if (nearbyTriangles.getSize() == 0) continue; // クラスター内に三角形がない場合スキップ

        for (int j = 0; j < grassCount; ++j) 
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
            BOUNDING_BOX grassBoundingBox;
            grassBoundingBox.maxPoint = XMFLOAT3(pos.x + 0.1f, octree->boundary.maxPoint.y, pos.z + 0.1f);
            grassBoundingBox.minPoint = XMFLOAT3(pos.x - 0.1f, octree->boundary.minPoint.y, pos.z - 0.1f);
            // 八分木で取得した三角形の中から最も高いものを選択
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

            // 草の Y 座標を最高点に調整
            pos.y = maxHeight;

            // 法線から回転クォータニオンを計算
            XMFLOAT4 rotation;
            XMVECTOR normalVec = XMLoadFloat3(&bestTriangle->normal);

            // デフォルトの上方向ベクトル (Y 軸)
            XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);

            // 法線がすでに上方向を向いている場合、回転を適用しない
            float dotProduct = XMVectorGetX(XMVector3Dot(upVec, normalVec));
            float angle = acosf(dotProduct); // ラジアン単位の角度

            constexpr float maxSlopeAngle = XMConvertToRadians(60.0f); // 最大許容角度 (60°)

            // もし角度が最大許容値を超えた場合、草を生成しない
            if (angle > maxSlopeAngle)
                continue;

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

                XMStoreFloat4(&rotation, quaternion);
            }

            // インスタンスデータを追加
            grassInstanceDataArray.push_back(
                InstanceData(
                    pos,                 // 草の位置
                    rotation,            // 草の回転 (法線方向に基づく)
                    GetRandFloat(20, 40) // 草のスケール (ランダム化)
                ));
        }
        
    }

    instanceCount = grassInstanceDataArray.getSize();
    instanceData = new InstanceData[instanceCount];
    for (int i = 0; i < instanceCount; i++)
        instanceData[i] = grassInstanceDataArray[i];

    // バッファ記述子設定
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT; // 頻繁な更新が必要なため DYNAMIC 使用
    bufferDesc.ByteWidth = sizeof(InstanceData) * instanceCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    // 初期データ設定
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = instanceData;

    // インスタンスバッファ作成
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &instanceBuffer);
    if (FAILED(hr))
        return false;

    return true;

}
