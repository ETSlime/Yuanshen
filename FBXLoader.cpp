#include "FBXLoader.h"
#include "SkinnedMeshModel.h"

void FBXLoader::LoadModel(ID3D11Device* device, TextureMgr& texMgr, SkinnedMeshModel& model, const char* modelFilename, const char* texturePath)
{
	FILE* file;
	file = fopen(modelFilename, "rt");
	if (file == NULL)
	{
		printf("エラー:LoadModel %s \n", modelFilename);
		return;
	}

    char buffer[1024];
    bool loadSuccess = true;
    long int position = 0;
    while (fgets(buffer, sizeof(buffer), file)) 
    {

        if (strstr(buffer, "; Object definitions"))
        {
            loadSuccess = ParseObjectDefinitions(file, model);
        }
        else if (strstr(buffer, "; Object properties"))
        {
            loadSuccess = ParseObjectProperties(file, model);
        }
        else if (strstr(buffer, "; Object connections"))
        {
            loadSuccess = ParseObjectConnections(file, model);
        }

        if (!loadSuccess)
            return;

        position = ftell(file);
        memset(buffer, 0, sizeof(buffer));
    }

    model.mModelHierarchy.resize(model.ModelCount);
    model.mModelGlobalRot.resize(model.ModelCount);
    model.mModelGlobalScl.resize(model.ModelCount);
    model.mModelTranslate.resize(model.ModelCount);
    model.mModelGlobalTrans.resize(model.ModelCount);
    model.mModelLocalTrans.resize(model.ModelCount);

    SimpleArray<FbxNode*> rootNodeChilds;
    for (auto& node : model.fbxNodes)
    {
        if (node.key == 0)
        {
            rootNodeChilds = node.value->childNodes;
            break;
        }
    }
    if (rootNodeChilds.getSize() != 0)
    {
        for (int i = 0; i < rootNodeChilds.getSize(); i++)
        {
            if (rootNodeChilds[i]->nodeType == FbxNodeType::Mesh)
            {
                SimpleArray<FbxNode*> meshNodeChilds = rootNodeChilds[i]->childNodes;
                for (int j = 0; j < meshNodeChilds.getSize(); j++)
                {
                    if (meshNodeChilds[j]->nodeType == FbxNodeType::Mesh)
                    {
                        SimpleArray<FbxNode*> geometryNodeChilds = meshNodeChilds[j]->childNodes;
                        ModelData* modelData = static_cast<ModelData*>(meshNodeChilds[j]->nodeData);
                        for (int k = 0; k < geometryNodeChilds.getSize(); k++)
                        {
                            FbxNode* armature = geometryNodeChilds[k];
                            HandleDeformer(armature, modelData, model, 0, -1);
                        }
                    }
                }
            }
            else if (rootNodeChilds[i]->nodeType == FbxNodeType::None || rootNodeChilds[i]->nodeType == FbxNodeType::LimbNode)
            {
                FbxNode* armature = rootNodeChilds[i];
                model.armatureNode = armature;
                int curIdx = 0;
                int prevIdx = -1;
                model.UpdateLimbGlobalTransform(armature, curIdx, prevIdx, model.animationTime);
            }
        }
    }


    model.Texture = texMgr.CreateTexture(model.textureName);
    model.modelData->IndexNum = model.modelData->mesh.IndicesData.getSize();
    model.modelData->VertexNum = model.modelData->IndexNum;
    model.modelData->VertexArray = new SKINNED_VERTEX_3D[model.modelData->VertexNum];
    model.modelData->IndexArray = new unsigned int[model.modelData->IndexNum];
    
    for (int indexCnt = 0; indexCnt < model.modelData->IndexNum; indexCnt++)
    {
        SKINNED_VERTEX_3D vertex;
        int vertexIndex = model.modelData->mesh.IndicesData[indexCnt].Index;

        if (vertexIndex < 0)
        {
            vertexIndex = -vertexIndex;
            vertexIndex--;
        }

        
        vertex.Position = model.modelData->mesh.VerticesTemp[vertexIndex].Position;
        vertex.Position.z = -vertex.Position.z;
        float BoneIndices[MAX_BONE_INDICES] = { 0.0f, 0.0f , 0.0f , 0.0f };
        float Weights[MAX_BONE_INDICES] = { 1.0f, 0.0f , 0.0f , 0.0f };
        int boneIndicesSize = model.modelData->mesh.VerticesTemp[vertexIndex].BoneIndices.getSize();
        if (boneIndicesSize > MAX_BONE_INDICES)
        {
            int topWeightsIndices[MAX_BONE_INDICES] = { 0 };
            float topWeights[MAX_BONE_INDICES] = { 0 };
            for (int i = 0; i < boneIndicesSize; ++i)
            {
                float currentWeight = model.modelData->mesh.VerticesTemp[vertexIndex].Weights[i];
                int currentIndex = model.modelData->mesh.VerticesTemp[vertexIndex].BoneIndices[i];
                for (int j = 0; j < MAX_BONE_INDICES; ++j)
                {
                    if (currentWeight > topWeights[j]) 
                    {
                        for (int k = MAX_BONE_INDICES - 1; k > j; --k)
                        {
                            topWeights[k] = topWeights[k - 1];
                            topWeightsIndices[k] = topWeightsIndices[k - 1];
                        }
                        topWeights[j] = currentWeight;
                        topWeightsIndices[j] = currentIndex;
                        break;
                    }
                }
            }

            float sumWeights = 0.0f;
            for (int i = 0; i < MAX_BONE_INDICES; i++)
            {
                sumWeights += topWeights[i];
            }
            if (sumWeights > 0) 
            {
                for (int i = 0; i < MAX_BONE_INDICES; i++)
                {
                    topWeights[i] /= sumWeights;
                }
            }

            for (int i = 0; i < MAX_BONE_INDICES; i++)
            {
                BoneIndices[i] = topWeightsIndices[i];
                Weights[i] = topWeights[i];

            }
        }
        else
        {
            for (int i = 0; i < boneIndicesSize; i++)
            {
                BoneIndices[i] = model.modelData->mesh.VerticesTemp[vertexIndex].BoneIndices[i];
                Weights[i] = model.modelData->mesh.VerticesTemp[vertexIndex].Weights[i];

            }
        }

        vertex.BoneIndices = XMFLOAT4(BoneIndices[0], BoneIndices[1], BoneIndices[2], BoneIndices[3]);
        vertex.Weights = XMFLOAT4(Weights[0], Weights[1], Weights[2], Weights[3]);

        
        if (model.normalLoc == Vertex)
            vertex.Normal = model.modelData->mesh.VerticesTemp[vertexIndex].Normal;
        else if (model.normalLoc == Index)
            vertex.Normal = model.modelData->mesh.IndicesData[indexCnt].Normal;

        if (model.texLoc == Vertex)
            vertex.TexCoord = model.modelData->mesh.VerticesTemp[vertexIndex].TexCoord;
        else if (model.texLoc == Index)
        {
            vertex.TexCoord = model.modelData->mesh.IndicesData[indexCnt].TexCoord;

            vertex.TexCoord.y = -vertex.TexCoord.y;
        }
        
        model.modelData->VertexArray[indexCnt] = vertex;
        model.modelData->IndexArray[indexCnt] = indexCnt;
    }

    // 頂点バッファ生成
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(SKINNED_VERTEX_3D) * model.modelData->VertexNum;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.pSysMem = model.modelData->VertexArray;

        renderer.GetDevice()->CreateBuffer(&bd, &sd, &model.VertexBuffer);
    }

    // インデックスバッファ生成
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(unsigned int) * model.modelData->IndexNum;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.pSysMem = model.modelData->IndexArray;

        renderer.GetDevice()->CreateBuffer(&bd, &sd, &model.IndexBuffer);
    }
}

bool FBXLoader::ParseObjectDefinitions(FILE* file, SkinnedMeshModel& model)
{
    char buffer[2048];
    bool inDefinitions = false;
    ObjectType objectType = ObjectType::NONE;
    int modelCount = 0, geometryCount = 0, materialCount = 0, textureCount = 0;
    int videoCount = 0, deformerCount = 0, poseCount = 0, globalSettingsCount = 0;
    long int position = 0;
    int bracketCount = 0;
    while (fgets(buffer, sizeof(buffer), file)) 
    {
        char objectTypeBuffer[64];
        if (inDefinitions)
        {
            if (sscanf(buffer, " ObjectType: \"%[^\"]\"", objectTypeBuffer) == 1)
            {
                if (strcmp(objectTypeBuffer, "Model") == 0)
                    objectType = ObjectType::Model;
                else if (strcmp(objectTypeBuffer, "Material") == 0)
                    objectType = ObjectType::Material;
                else if (strcmp(objectTypeBuffer, "Texture") == 0)
                    objectType = ObjectType::Texture;
                else if (strcmp(objectTypeBuffer, "AnimationStack") == 0)
                    objectType = ObjectType::AnimationStack;
                else if (strcmp(objectTypeBuffer, "AnimationLayer") == 0)
                    objectType = ObjectType::AnimationLayer;
                else if (strcmp(objectTypeBuffer, "Geometry") == 0)
                    objectType = ObjectType::Geometry;
                else if (strcmp(objectTypeBuffer, "Implementation") == 0)
                    objectType = ObjectType::Implementation;
                else if (strcmp(objectTypeBuffer, "BindingTable") == 0)
                    objectType = ObjectType::BindingTable;

                int bracket = 0;
                if (strstr(buffer, "{"))
                    bracket++;
                if (strstr(buffer, "}"))
                    bracket--;

                bool inProperty = false;
                switch (objectType)
                {
                case ObjectType::Model:
                    
                    while (fgets(buffer, sizeof(buffer), file))
                    {
                        //int current = ftell(file);
                        //if (inProperty && !strstr(buffer, "}"))
                        //{
                        //    fseek(file, position, SEEK_SET);
                        //    current = ftell(file);
                        //    fgets(buffer, sizeof(buffer), file);
                        //    current = ftell(file);
                        //    break;
                        //}

                        if (strstr(buffer, "{"))
                            bracket++;
                        if (strstr(buffer, "}"))
                            bracket--;

                        if (bracket == 0)
                            break;

                        if (strstr(buffer, "Properties70:"))
                        {
                            inProperty = true;
                            if (!ParseModelProperty(file, model.globalModelProperty, nullptr))
                                return false;
                            bracket--;
                        }
                        //memset(buffer, 0, sizeof(buffer));
                        //position = ftell(file);
                    }

                    break;
                case ObjectType::Material:
                    while (fgets(buffer, sizeof(buffer), file))
                    {
                        if (strstr(buffer, "{"))
                            bracket++;
                        if (strstr(buffer, "}"))
                            bracket--;

                        if (bracket == 0)
                            break;

                        if (strstr(buffer, "Properties70:"))
                        {
                            inProperty = true;
                            if (!ParseMaterialProperty(file, model.globalMaterial))
                                return false;
                            bracket--;
                        }
                    }
                    break;
                default:
                    if (strstr(buffer, "{"))
                        SkipNode(file);
                    break;
                }
            }
            else if (strstr(buffer, "}"))
            {
                return true;
            }


        }
        else if (strstr(buffer, "Definitions:"))
            inDefinitions = true;

            position = ftell(file);

    }

    return false;
}

bool FBXLoader::ParseObjectProperties(FILE* file, SkinnedMeshModel& model)
{
    char buffer[2048];
    bool inObjects = false;

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (inObjects == false)
        {
            if (strchr(buffer, '{'))
            {
                inObjects = true;
            }
        }

        if (inObjects)
        {
            // Geometryフィールドを解析
            if (strstr(buffer, "NodeAttribute"))
            {
                //FbxNode* node = new FbxNode();
                //if (!CreateFbxNode(buffer, node))
                //{
                //    delete node;
                //    return false;
                //}
                //model.fbxNodes.insert(node->nodeID, node);
                SkipNode(file);
            }
            if (strstr(buffer, "Geometry:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                    return false;

                char geometryNameBuffer[128], geometryTypeBuffer[128];
                GeometryType geometryType;
                if (sscanf(buffer, " Geometry: %*[^,], \"%[^\"]\", \"%[^\"]\"", geometryNameBuffer, geometryTypeBuffer) != 2)
                {
                    delete node;
                    return false;   // Geometryの解析が失敗した
                }

                if (strcmp(geometryTypeBuffer, "Mesh") == 0)
                    geometryType = GeometryType::Mesh;
                else if (strcmp(geometryTypeBuffer, "Shape") == 0)
                {
                    geometryType = GeometryType::Shape;
                    model.modelData->shapeCnt++;
                    model.modelData->shapes.push_back(MeshData());
                }
                else
                {
                    delete node;
                    return false;
                }

                if (!ParseGeometry(file, model, geometryType))
                {
                    delete node;
                    return false;   // Geometryの解析が失敗した
                }

                node->nodeData = static_cast<void*>(model.modelData);

                model.fbxNodes.insert(node->nodeID, node);

            }
            // Modelフィールドを解析
            if (strstr(buffer, "Model:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                

                char modelName[128], modelType[128];
                if (sscanf(buffer, " Model: %*[^,], \"%[^\"]\", \"%[^\"]\"", modelName, modelType) != 2)
                {
                    delete node;
                    return false;   // Modelの解析が失敗した
                }
                if (!ParseModel(file, model, node))
                {
                    delete node;
                    return false;   // Modelの解析が失敗した
                }

                model.fbxNodes.insert(node->nodeID, node);
            }

            // Poseフィールドを解析
            if (strstr(buffer, "Pose:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);
                
                ParseBindPose(file, model, node);
                //SkipNode(file);
            }
            // Materialフィールドを解析
            if (strstr(buffer, "Material:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);

                char materialName[128];
                if (sscanf(buffer, " Material: %*[^,], \"%[^\"]\", \"%[^\"]\"", materialName) != 1)
                {
                    delete node;
                    return false;   // Materialの解析が失敗した
                }
                if (!ParseMaterial(file, model))
                {
                    delete node;
                    return false;   // Materialの解析が失敗した
                }
            }
            if (strstr(buffer, "Deformer:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                    

                if (node->nodeType == FbxNodeType::Skin)
                {
                    Deformer* deformer = new Deformer();
                    deformer->boneIdx = 0;
                    XMStoreFloat4x4(&deformer->Transform, XMMatrixIdentity());
                    XMStoreFloat4x4(&deformer->TransformLink, XMMatrixIdentity());
                    node->nodeData = static_cast<void*>(deformer);
                    SkipNode(file);
                }
                else if (node->nodeType == FbxNodeType::BlendShape
                        || node->nodeType == FbxNodeType::BlendShapeChannel)
                {
                    SkipNode(file);
                }
                else if (!ParseDeformer(file, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);
            }
            if (strstr(buffer, "Video:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);

                SkipNode(file);
            }
            // Textureフィールドを解析
            if (strstr(buffer, "Texture:"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);

                if (model.textureName[0] != '\0')
                    SkipNode(file);
                else
                {
                    char TextureName[128];
                    if (sscanf(buffer, " Texture: %*[^,], \"%[^\"]\", \"%[^\"]\"", TextureName) != 1)
                    {
                        delete node;
                        return false;   // Textureの解析が失敗した
                    }
                    if (!ParseTexture(file, model))
                    {
                        delete node;
                        return false;   // Textureの解析が失敗した
                    }
                }

            }
            if (strstr(buffer, "AnimationStack"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);


                SkipNode(file);
            }
            if (strstr(buffer, "AnimationCurve"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);

                if (!ParseAnimationCurve(file, node))
                {
                    delete node;
                    return false;
                }
            }
            if (strstr(buffer, "AnimationCurveNode"))
            {
                FbxNode* node = new FbxNode();
                if (!CreateFbxNode(buffer, node))
                {
                    delete node;
                    return false;
                }
                model.fbxNodes.insert(node->nodeID, node);

                SkipNode(file);
            }
        }

        if (strstr(buffer, "}"))
        {
            return true;
        }

        memset(buffer, 0, sizeof(buffer));
    }

    return false;
}

bool FBXLoader::ParseGeometry(FILE* file, SkinnedMeshModel& model, GeometryType type)
{
    char buffer[2048];
    int readCount;
    bool readVertice = false;
    long int position = 0;
    // Geometry内部のフィールドを処理する
    while (fgets(buffer, sizeof(buffer), file))
    {
        //if (strstr(buffer, "Properties60:"))
        //{
        //    if (!ParseModelProperty(file))
        //        return false;
        //}
        //else if (strstr(buffer, "Culling:"))
        //{
        //    char culling[32];
        //    
        //    readCount = sscanf(buffer, " Culling: \"%[^\"]\"", culling);
        //    if (readCount != 1)
        //        return false;
        //    if (strcmp(culling, "CullingOff") == 0)
        //        cullMode = CULL_MODE_NONE;
        //}
        if (strstr(buffer, "Vertices:"))
        {
            //fseek(file, position, SEEK_SET);

            if (!ParseVertexData(file, model, type))
                return false;
        }
        else if (strstr(buffer, "PolygonVertexIndex:")
                || strstr(buffer, "Indexes:"))
        {
            //fseek(file, position, SEEK_SET);

            if (!ParseIndexData(file, model, type))
                return false;
        }
        else if (strstr(buffer, "LayerElementNormal:"))
        {
            if (!ParseNormal(file, model, type))
                return false;     
        }
        else if (strstr(buffer, "Normals: "))
        {
            if (!ParseNormalData(file, model, type, MappingInformationType::ByPolygonVertex, ReferenceInformationType::Direct))
                return false;

        }
        else if (strstr(buffer, "LayerElementSmoothing:"))
        {
            while (!strstr(buffer, "}"))
                fgets(buffer, sizeof(buffer), file);
        }
        else if (strstr(buffer, "LayerElementUV:"))
        {
            if (!ParseUV(file, model, type))
                return false;
        }
        else if (strstr(buffer, "LayerElementMaterial:"))
        {
            //fseek(file, position, SEEK_SET);
            SkipNode(file);
        }
        else if (strstr(buffer, "Layer:"))
        {
            //fseek(file, position, SEEK_SET);
            SkipNode(file);
        }
        else if (strstr(buffer, "}"))
        {
            // Geometryセグメントの終わり
            return true;
        }

        memset(buffer, 0, sizeof(buffer));

        position = ftell(file);
    }

    return false;  // ファイルが意外に終了した場合
}

bool FBXLoader::ParseModel(FILE* file, SkinnedMeshModel& model, FbxNode* node)
{
    char buffer[2048];
    int readCount;
    long int position = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "Properties70:"))
        {
            if (!ParseModelProperty(file, model.modelProperty, node))
                return false;
        }
        else if (strstr(buffer, "Culling:"))
        {
            char culling[32];

            readCount = sscanf(buffer, " Culling: \"%[^\"]\"", culling);
            if (readCount != 1)
                return false;
            if (strcmp(culling, "CullingOff") == 0)
                model.cullMode = CULL_MODE_NONE;
        }
        else if (strstr(buffer, "}"))
        {
            // Modelセグメントの終わり
            model.ModelCount++;
            return true;
        }

        memset(buffer, 0, sizeof(buffer));
        position = ftell(file);
    }

    return false;
}

bool FBXLoader::ParseMaterial(FILE* file, SkinnedMeshModel& model)
{
    char buffer[2048];
    int readCount;
    long int position = 0;
    // Material内部のフィールドを処理する
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "Properties70:"))
        {
            if (!ParseMaterialProperty(file, model.material))
                return false;
        }

        if (strstr(buffer, "}"))
            return true;
    }
    return false;
}

bool FBXLoader::ParseDeformer(FILE* file, FbxNode* node)
{
    char buffer[4096];
    char* ptr;
    int index;
    int indexCount = 0;
    float weight;
    int weightCount = 0;
    Deformer* deformer = new Deformer;
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (sscanf(buffer, " Indexes: *%d", &indexCount) == 1)
        {
            int IndexFound = 0;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!IndexFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        deformer->Index.resize(indexCount);
                        ptr += strlen("a:");
                        IndexFound = 1;
                    }
                }

                if (IndexFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                        break;

                    while (sscanf(ptr, "%d", &index) == 1)
                    {
                        deformer->Index.push_back(index);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }
            }
        }
        else if (sscanf(buffer, " Weights: *%d", &weightCount) == 1)
        {
            int WeightFound = 0;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!WeightFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        deformer->Weights.resize(weightCount);
                        ptr += strlen("a:");
                        WeightFound = 1;
                    }
                }

                if (WeightFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                        break;

                    while (sscanf(ptr, "%f", &weight) == 1)
                    {
                        deformer->Weights.push_back(weight);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }
            }
        }
        else if (strstr(buffer, "Transform:"))
        {
            int TransformFound = 0;
            SimpleArray<float> transformArray;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!TransformFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        ptr += strlen("a:");
                        TransformFound = 1;
                    }
                }

                if (TransformFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                        break;

                    while (sscanf(ptr, "%f", &weight) == 1)
                    {
                        transformArray.push_back(weight);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }
            }
            if (transformArray.getSize() != 16)
            {
                delete deformer;
                return false;
            }
                
            int index = 0;
            for (int col = 0; col < 4; col++) 
            {
                for (int row = 0; row < 4; row++) 
                {
                    deformer->Transform.m[col][row] = transformArray[index++];
                }
            }
        }
        else if (strstr(buffer, "TransformLink:"))
        {
            int TransformLinkFound = 0;
            SimpleArray<float> transformArrayLink;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!TransformLinkFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        ptr += strlen("a:");
                        TransformLinkFound = 1;
                    }
                }

                if (TransformLinkFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                        break;

                    while (sscanf(ptr, "%f", &weight) == 1)
                    {
                        transformArrayLink.push_back(weight);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }
            }
            if (transformArrayLink.getSize() != 16)
            {
                delete deformer;
                return false;
            }
            int index = 0;
            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 4; row++)
                {
                    deformer->TransformLink.m[col][row] = transformArrayLink[index++];
                }
            }
        }
        else if (strstr(buffer, "}"))
        {
            if (deformer->Weights.getSize() == weightCount &&
                deformer->Index.getSize() == indexCount)
            {
                node->nodeData = static_cast<void*>(deformer);
            }
            else
            {
                delete deformer;
            }
                
            return true;
        }
            
    }
    return false;
}

bool FBXLoader::ParseMaterialProperty(FILE* file, Material& material)
{
    char buffer[2048];
    char property[128] = { 0 };
    double value;
    int readCount = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        sscanf(buffer, " P: \"%[^\"]\"", property);

        if (strstr(property, "MultiLayer"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &material.MultiLayer);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "EmissiveColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.EmissiveColor.x, &material.EmissiveColor.y, &material.EmissiveColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "EmissiveFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.EmissiveFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "AmbientColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.AmbientColor.x, &material.AmbientColor.y, &material.AmbientColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "AmbientFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.AmbientFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "DiffuseColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.DiffuseColor.x, &material.DiffuseColor.y, &material.DiffuseColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "DiffuseFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.DiffuseFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "Bump\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.Bump.x, &material.Bump.y, &material.Bump.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "NormalMap"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.NormalMap.x, &material.NormalMap.y, &material.NormalMap.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "BumpFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &material.BumpFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "TransparentColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.TransparentColor.x, &material.TransparentColor.y, &material.TransparentColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "TransparencyFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.TransparencyFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "DisplacementColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.DisplacementColor.x, &material.DisplacementColor.y, &material.DisplacementColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "DisplacementFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &material.DisplacementFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "VectorDisplacementColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.VectorDisplacementColor.x, &material.VectorDisplacementColor.y, &material.VectorDisplacementColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "VectorDisplacementFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &material.VectorDisplacementFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "SpecularColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.SpecularColor.x, &material.SpecularColor.y, &material.SpecularColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "SpecularFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.SpecularFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "ShininessExponent"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.ShininessExponent);
            if (readCount != 1)
                return false;
        }
        else if (strstr(property, "ReflectionColor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf, %lf, %lf", &material.ReflectionColor.x, &material.ReflectionColor.y, &material.ReflectionColor.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(property, "ReflectionFactor"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &material.ReflectionFactor);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "}"))
        {
            // MaterialPropertyセグメントの終わり
            return true;
        }

        memset(property, 0, sizeof(property));
    }
    return false;
}

bool FBXLoader::ParseModelProperty(FILE* file, ModelProperty& modelProperty, FbxNode* node)
{
    char buffer[2048];
    double value;
    int readCount;
    int propertyCnt = 0;
    long int pos = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "QuaternionInterpolate"))
        {
            readCount = sscanf(buffer, " %*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.QuaternionInterpolate);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationOffset"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.RotationOffset.x, &modelProperty.RotationOffset.y, &modelProperty.RotationOffset.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "RotationPivot"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.RotationPivot.x, &modelProperty.RotationPivot.y, &modelProperty.RotationPivot.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "ScalingOffset"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.ScalingOffset.x, &modelProperty.ScalingOffset.y, &modelProperty.ScalingOffset.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "ScalingPivot"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.ScalingPivot.x, &modelProperty.ScalingPivot.y, &modelProperty.ScalingPivot.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "TranslationActive"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationActive);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "TranslationMin\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.TranslationMin.x, &modelProperty.TranslationMin.y, &modelProperty.TranslationMin.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "TranslationMax\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.TranslationMax.x, &modelProperty.TranslationMax.y, &modelProperty.TranslationMax.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "TranslationMinX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationMinX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "TranslationMinY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationMinY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "TranslationMinZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationMinZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "TranslationMaxX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationMaxX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "TranslationMaxY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationMaxY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "TranslationMaxZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.TranslationMaxZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationOrder"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationOrder);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationSpaceForLimitOnly"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationSpaceForLimitOnly);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationStiffnessX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.RotationStiffnessX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationStiffnessY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.RotationStiffnessY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationStiffnessZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.RotationStiffnessZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "AxisLen"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.AxisLen);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "PreRotation"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.PreRotation.x, &modelProperty.PreRotation.y, &modelProperty.PreRotation.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "PostRotation"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.PostRotation.x, &modelProperty.PostRotation.y, &modelProperty.PostRotation.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "RotationActive"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationActive);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationMin\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.RotationMin.x, &modelProperty.RotationMin.y, &modelProperty.RotationMin.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "RotationMax\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.RotationMax.x, &modelProperty.RotationMax.y, &modelProperty.RotationMax.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "RotationMinX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationMinX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationMinY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationMinY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationMinZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationMinZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationMaxX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationMaxX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationMaxY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationMaxY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "RotationMaxZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.RotationMaxZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "InheritType"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.InheritType);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingActive"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingActive);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingMin\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.ScalingMin.x, &modelProperty.ScalingMin.y, &modelProperty.ScalingMin.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "ScalingMax\""))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.ScalingMax.x, &modelProperty.ScalingMax.y, &modelProperty.ScalingMax.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "ScalingMinX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingMinX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingMinY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingMinY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingMinZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingMinZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingMaxX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingMaxX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingMaxY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingMaxY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "ScalingMaxZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.ScalingMaxZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "GeometricTranslation"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.GeometricTranslation.x, &modelProperty.GeometricTranslation.y, &modelProperty.GeometricTranslation.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "GeometricRotation"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.GeometricRotation.x, &modelProperty.GeometricRotation.y, &modelProperty.GeometricRotation.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "GeometricScaling"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.GeometricScaling.x, &modelProperty.GeometricScaling.y, &modelProperty.GeometricScaling.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "MinDampRangeX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MinDampRangeX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MinDampRangeY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MinDampRangeY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MinDampRangeZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MinDampRangeZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MaxDampRangeX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MaxDampRangeX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MaxDampRangeY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MaxDampRangeY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MaxDampRangeZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MaxDampRangeZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MinDampStrengthX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MinDampStrengthX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MinDampStrengthY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MinDampStrengthY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MinDampStrengthZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MinDampStrengthZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MaxDampStrengthX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MaxDampStrengthX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MaxDampStrengthY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MaxDampStrengthY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "MaxDampStrengthZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.MaxDampStrengthZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "PreferedAngleX"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.PreferedAngleX);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "PreferedAngleY"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.PreferedAngleY);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "PreferedAngleZ"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%lf", &modelProperty.PreferedAngleZ);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "Show"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.Show);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "NegativePercentShapeSupport"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.NegativePercentShapeSupport);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "DefaultAttributeIndex"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.DefaultAttributeIndex);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "Freeze"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.Freeze);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "LODBox"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.LODBox);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "Lcl Translation"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.Translation.x, &modelProperty.Translation.y, &modelProperty.Translation.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "Lcl Rotation"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.Rotation.x, &modelProperty.Rotation.y, &modelProperty.Rotation.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "Lcl Scaling"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%f, %f, %f", &modelProperty.Scaling.x, &modelProperty.Scaling.y, &modelProperty.Scaling.z);
            if (readCount != 3)
                return false;
        }
        else if (strstr(buffer, "Visibility"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.Visibility);
            if (readCount != 1)
                return false;
        }
        else if (strstr(buffer, "Visibility Inheritance"))
        {
            readCount = sscanf(buffer, "%*[^,],%*[^,],%*[^,],%*[^,],%d", &modelProperty.VisibilityInheritance);
            if (readCount != 1)
                return false;
        }

        
        if (strstr(buffer, "}"))
        {
            // ModelPropertyセグメントの終わり
            if (node)
            {
                ModelProperty* property = new ModelProperty(modelProperty);
                modelProperty.Clear();
                node->nodeData = static_cast<void*>(property);
            }
                
            return true;
        }

        memset(buffer, 0, sizeof(buffer));
        pos = ftell(file);
    }
}

bool FBXLoader::ParseVertexData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType)
{
    char buffer[4096];
    float vertexPos[3];
    float vertex;
    int count = 0;
    char* ptr;
    int verticesFound = 0;
    long int position = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        ptr = buffer;
        if (!verticesFound) 
        {
            ptr = strstr(buffer, "a:");
            if (ptr) 
            {
                ptr += strlen("a:");
                verticesFound = 1;
            }
        }

        if (verticesFound)
        {
            while (*ptr && isspace((unsigned char)*ptr)) ptr++;

            if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.' && *ptr != ',')
                break;

            while (sscanf(ptr, "%f", &vertex) == 1) 
            {
                if (count < 3) 
                {
                    vertexPos[count++] = vertex;
                }
                if (count == 3) 
                {
                    SKINNED_VERTEX_3D_TEMP newVertex;
                    newVertex.Position.x = vertexPos[0];
                    newVertex.Position.y = vertexPos[1];
                    newVertex.Position.z = vertexPos[2];
                    if (geometryType == GeometryType::Mesh)
                        model.modelData->mesh.VerticesTemp.push_back(newVertex);
                    else if (geometryType == GeometryType::Mesh)
                        model.modelData->shapes[model.modelData->shapeCnt - 1].VerticesTemp.push_back(newVertex);

                    count = 0;
                }
                
                while (*ptr != ',' && *ptr != '\0') ptr++;
                if (*ptr == ',') ptr++;
            }
        }

        position = ftell(file);
    }

    if (count > 0 && count < 3)
    {
        if (geometryType == GeometryType::Mesh)
            model.modelData->mesh.VerticesTemp.clear();
        else if (geometryType == GeometryType::Mesh)
            model.modelData->shapes[model.modelData->shapeCnt - 1].VerticesTemp.clear();
        return false;
    }
    else
    {
        if (!strstr(buffer, "}"))
            fseek(file, position, SEEK_SET);
        return true;
    }
}

bool FBXLoader::ParseIndexData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType)
{
    char buffer[4096];
    int index;
    int count = 0;
    char* ptr;
    int indicesFound = 0;
    long int position = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        ptr = buffer;
        if (!indicesFound)
        {
            ptr = strstr(buffer, "a:");
            if (ptr)
            {
                ptr += strlen("a:");
                indicesFound = 1;
            }
        }

        if (indicesFound)
        {
            while (*ptr && isspace((unsigned char)*ptr)) ptr++;

            if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.') break;

            int polygonSize = 4;
            while (sscanf(ptr, "%d", &index) == 1)
            {
                IndexData indexData;
                indexData.Index = index;
                if (count == 2 && index < 0)
                    polygonSize = 3;
                
                if (geometryType == GeometryType::Mesh)
                    model.modelData->mesh.IndicesData.push_back(indexData);
                else if (geometryType == GeometryType::Shape)
                    model.modelData->shapes[model.modelData->shapeCnt - 1].IndicesData.push_back(indexData);
                count++;

                if (count == polygonSize)
                    count = 0;

                while (*ptr != ',' && *ptr != '\0') ptr++;
                if (*ptr == ',') ptr++;
            }
        }

        position = ftell(file);
    }

    if (count > 0 && count < 4 && geometryType == GeometryType::Mesh)
    {
        model.modelData->mesh.IndicesData.clear();
        return false;
    }
    else
    {
        if (!strstr(buffer, "}"))
            fseek(file, position, SEEK_SET);
        return true;
    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             }

bool FBXLoader::ParseNormal(FILE* file, SkinnedMeshModel& model, GeometryType geometryType)
{
    char buffer[4096];
    int readCount;
    char MappingInformationTypeBuffer[32];
    char ReferenceInformationTypeBuffer[32];
    char* ptr;
    int normalFound = 0;
    long int position = 0;
    MappingInformationType mappingInformationType;
    ReferenceInformationType referenceInformationType;

    while (fgets(buffer, sizeof(buffer), file))
    {

        if (strstr(buffer, "MappingInformationType:"))
        {
            readCount = sscanf(buffer, " MappingInformationType: \"%[^\"]\"", MappingInformationTypeBuffer);
            if (readCount != 1)
                return false;

            if (strcmp(MappingInformationTypeBuffer, "ByPolygon") == 0)
                mappingInformationType = ByPolygon;
            else if (strcmp(MappingInformationTypeBuffer, "ByPolygonVertex") == 0)
                mappingInformationType = ByPolygonVertex;
            else if (strcmp(MappingInformationTypeBuffer, "ByVertice") == 0)
                mappingInformationType = ByVertice;
            else if (strcmp(MappingInformationTypeBuffer, "ByEdge") == 0)
                mappingInformationType = ByEdge;
            else if (strcmp(MappingInformationTypeBuffer, "AllSame") == 0)
                mappingInformationType = AllSame;
            else
                return false;

            if (mappingInformationType == ByVertice)
                model.normalLoc = Vertex;
            else if (mappingInformationType == ByPolygonVertex)
                model.normalLoc = Index;
        }
        else if (strstr(buffer, "ReferenceInformationType:"))
        {
            readCount = sscanf(buffer, " ReferenceInformationType: \"%[^\"]\"", ReferenceInformationTypeBuffer);
            if (readCount != 1)
                return false;

            if (strcmp(ReferenceInformationTypeBuffer, "Direct") == 0)
                referenceInformationType = Direct;
            else if (strcmp(ReferenceInformationTypeBuffer, "IndexToDirect") == 0)
                referenceInformationType = IndexToDirect;
            else
                return false;

        }
        else if (strstr(buffer, "Normals:"))
        {
            //fseek(file, position, SEEK_SET);

            ParseNormalData(file, model, geometryType, mappingInformationType, referenceInformationType);

            //if (strstr(buffer, "}"))
            //    fgets(buffer, sizeof(buffer), file);
            //fseek(file, position, SEEK_SET);
        }

        else if (strstr(buffer, "NormalsW:"))
            SkipNode(file);

        else if (strstr(buffer, "}"))
        {
            // Modelセグメントの終わり
            //fgets(buffer, sizeof(buffer), file);
            return true;
        }

        position = ftell(file);
    }

    return false;
}

bool FBXLoader::ParseNormalData(FILE* file, SkinnedMeshModel& model, GeometryType geometryType, MappingInformationType mappingInformationType, ReferenceInformationType referenceInformationType)
{
    char buffer[4096];
    float normal;
    int normalCount = 0;
    int totalNormalCount = 0;
    int vertexCount = 0;
    int normalFound = 0;
    float normalsByVertex[3];
    char* ptr;
    long int position = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        ptr = buffer;
        if (!normalFound)
        {
            ptr = strstr(buffer, "a:");
            if (ptr)
            {
                ptr += strlen("a:");
                normalFound = 1;
            }
        }

        if (normalFound)
        {
            while (*ptr && isspace((unsigned char)*ptr)) ptr++;

            if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.') break;

            if (mappingInformationType == ByPolygonVertex
                && referenceInformationType == Direct)
            {

                while (sscanf(ptr, "%f", &normal) == 1)
                {
                    if (normalCount < 3)
                    {
                        normalsByVertex[normalCount % 3] = normal;
                        normalCount++;
                        totalNormalCount++;
                    }
                    if (normalCount == 3)
                    {
                        if (vertexCount >= model.modelData->mesh.IndicesData.getSize())
                        {
                            model.modelData->mesh.Vertices.clear();
                            return false;
                        }
                        if (geometryType == GeometryType::Mesh)
                        {
                            model.modelData->mesh.IndicesData[vertexCount].Normal.x = normalsByVertex[0];
                            model.modelData->mesh.IndicesData[vertexCount].Normal.y = normalsByVertex[1];
                            model.modelData->mesh.IndicesData[vertexCount].Normal.z = normalsByVertex[2];
                        }
                        else if (geometryType == GeometryType::Shape)
                        {
                            model.modelData->shapes[model.modelData->shapeCnt - 1].IndicesData[vertexCount].Normal.x = normalsByVertex[0];
                            model.modelData->shapes[model.modelData->shapeCnt - 1].IndicesData[vertexCount].Normal.y = normalsByVertex[1];
                            model.modelData->shapes[model.modelData->shapeCnt - 1].IndicesData[vertexCount].Normal.z = normalsByVertex[2];
                        }

                        vertexCount++;
                        normalCount = 0;
                    }

                    while (*ptr != ',' && *ptr != '\0') ptr++;
                    if (*ptr == ',') ptr++;
                }
            }
            else if (mappingInformationType == ByVertice
                && referenceInformationType == Direct)
            {
                while (sscanf(ptr, "%f", &normal) == 1)
                {
                    if (normalCount < 3)
                    {
                        normalsByVertex[normalCount % 3] = normal;
                        normalCount++;
                        totalNormalCount++;
                    }
                    if (normalCount == 3)
                    {
                        if (geometryType == GeometryType::Mesh)
                        {
                            if (vertexCount >= model.modelData->mesh.VerticesTemp.getSize())
                            {
                                model.modelData->mesh.Vertices.clear();
                                return false;
                            }
                            model.modelData->mesh.VerticesTemp[vertexCount].Normal.x = normalsByVertex[0];
                            model.modelData->mesh.VerticesTemp[vertexCount].Normal.y = normalsByVertex[1];
                            model.modelData->mesh.VerticesTemp[vertexCount].Normal.z = normalsByVertex[2];
                        }
                        else if (geometryType == GeometryType::Shape)
                        {
                            if (vertexCount >= model.modelData->shapes[model.modelData->shapeCnt - 1].VerticesTemp.getSize())
                            {
                                model.modelData->shapes[model.modelData->shapeCnt - 1].Vertices.clear();
                                return false;
                            }
                            model.modelData->shapes[model.modelData->shapeCnt - 1].VerticesTemp[vertexCount].Normal.x = normalsByVertex[0];
                            model.modelData->shapes[model.modelData->shapeCnt - 1].VerticesTemp[vertexCount].Normal.y = normalsByVertex[1];
                            model.modelData->shapes[model.modelData->shapeCnt - 1].VerticesTemp[vertexCount].Normal.z = normalsByVertex[2];
                        }

                        vertexCount++;
                        normalCount = 0;
                    }

                    while (*ptr != ',' && *ptr != '\0') ptr++;
                    if (*ptr == ',') ptr++;
                }
            }

        }

        position = ftell(file);
    }

    int VertexArrayCnt, IndexArrayCnt;
    if (geometryType == GeometryType::Mesh)
    {
        VertexArrayCnt = model.modelData->mesh.Vertices.getSize();
        IndexArrayCnt = model.modelData->mesh.IndicesData.getSize();
    }
    else if (geometryType == GeometryType::Shape)
    {
        VertexArrayCnt = model.modelData->shapes[model.modelData->shapeCnt - 1].Vertices.getSize();
        IndexArrayCnt = model.modelData->shapes[model.modelData->shapeCnt - 1].IndicesData.getSize();
    }


    if (mappingInformationType == ByPolygon
        && vertexCount != VertexArrayCnt)
        return false;
    else if (mappingInformationType == ByPolygonVertex
        && vertexCount != IndexArrayCnt)
        return false;
    return true;
}

bool FBXLoader::ParseUV(FILE* file, SkinnedMeshModel& model, GeometryType geometryType)
{
    char buffer[4096];
    int readCount;
    char MappingInformationTypeBuffer[32];
    char ReferenceInformationTypeBuffer[32];
    char* ptr;
    SimpleArray<float> UVCoord;
    SimpleArray<float> UVCoordIndex;
    float UV;
    int UVIndex;
    long int position = 0;
    MappingInformationType mappingInformationType;
    ReferenceInformationType referenceInformationType;

    while (fgets(buffer, sizeof(buffer), file))
    {

        if (strstr(buffer, "MappingInformationType:"))
        {
            readCount = sscanf(buffer, " MappingInformationType: \"%[^\"]\"", MappingInformationTypeBuffer);
            if (readCount != 1)
                return false;

            if (strcmp(MappingInformationTypeBuffer, "ByPolygon") == 0)
                mappingInformationType = ByPolygon;
            else if (strcmp(MappingInformationTypeBuffer, "ByPolygonVertex") == 0)
                mappingInformationType = ByPolygonVertex;
            else if (strcmp(MappingInformationTypeBuffer, "ByVertice") == 0)
                mappingInformationType = ByVertice;
            else if (strcmp(MappingInformationTypeBuffer, "ByEdge") == 0)
                mappingInformationType = ByEdge;
            else if (strcmp(MappingInformationTypeBuffer, "AllSame") == 0)
                mappingInformationType = AllSame;
            else
                return false;

            if (mappingInformationType == ByVertice)
                model.texLoc = Vertex;
            else if (mappingInformationType == ByPolygonVertex)
                model.texLoc = Index;
        }
        else if (strstr(buffer, "ReferenceInformationType:"))
        {
            readCount = sscanf(buffer, " ReferenceInformationType: \"%[^\"]\"", ReferenceInformationTypeBuffer);
            if (readCount != 1)
                return false;

            if (strcmp(ReferenceInformationTypeBuffer, "Direct") == 0)
                referenceInformationType = Direct;
            else if (strcmp(ReferenceInformationTypeBuffer, "IndexToDirect") == 0)
                referenceInformationType = IndexToDirect;
            else
                return false;

        }
        else if (strstr(buffer, "UV:"))
        {
            //fseek(file, position, SEEK_SET);
            int UVFound = 0;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!UVFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        ptr += strlen("a:");
                        UVFound = 1;
                    }
                }

                if (UVFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.') 
                        break;

                    while (sscanf(ptr, "%f", &UV) == 1)
                    {
                        UVCoord.push_back(UV);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }

                position = ftell(file);
            }
            
            //if (strstr(buffer, "}"))
            //    fgets(buffer, sizeof(buffer), file);
            //fseek(file, position, SEEK_SET);
        }
        else if (strstr(buffer, "UVIndex:"))
        {
            //fseek(file, position, SEEK_SET);
            int UVIndexFound = 0;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!UVIndexFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        ptr += strlen("a:");
                        UVIndexFound = 1;
                    }
                }

                if (UVIndexFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.') 
                        break;

                    while (sscanf(ptr, "%d", &UVIndex) == 1)
                    {       
                        UVCoordIndex.push_back(UVIndex);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }

                position = ftell(file);
            }

            //fseek(file, position, SEEK_SET);
        }

        // Modelセグメントの終わり
        else if (strstr(buffer, "}"))
        {
            if (mappingInformationType == ByPolygonVertex
                && referenceInformationType == IndexToDirect)
            {
                int UVCoordIndexSize = UVCoordIndex.getSize();
                if (UVCoordIndexSize != model.modelData->mesh.IndicesData.getSize())
                    return false;

                for (int i = 0; i < UVCoordIndexSize; i++)
                {                    
                    int coordIndex = UVCoordIndex[i];

                    model.modelData->mesh.IndicesData[i].TexCoord.x = UVCoord[coordIndex * 2];
                    model.modelData->mesh.IndicesData[i].TexCoord.y = UVCoord[coordIndex * 2 + 1];
                }
            }
            //else if (mappingInformationType == AllSame)

            //fgets(buffer, sizeof(buffer), file);
            return true;
        }

        position = ftell(file);
    }

    return false;
}

bool FBXLoader::ParseTexture(FILE* file, SkinnedMeshModel& model)
{
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "Properties70"))
            SkipNode(file);
        else if (strstr(buffer, "RelativeFilename"))
        {
            if (sscanf(buffer, " RelativeFilename: \"%[^\"]\"", model.textureName) != 1)
                return false;
        }
        else if (strstr(buffer, "}"))
            return true;
    }
    return false;
}

bool FBXLoader::ParseObjectConnections(FILE* file, SkinnedMeshModel& model)
{
    char buffer[512];
    uint64_t nodeID1, nodeID2;

    while (fgets(buffer, sizeof(buffer), file))
    {
        char attributeType[64];
        if (sscanf(buffer, " C: \"OO\",%llu,%llu", &nodeID1, &nodeID2) == 2)
        {
            FbxNode** ppChildNode = model.fbxNodes.search(nodeID1);
            FbxNode** ppParentNode = model.fbxNodes.search(nodeID2);
            if (ppChildNode == nullptr || ppParentNode == nullptr) continue;

            (*ppChildNode)->parentNode = *ppParentNode;
            (*ppParentNode)->childNodes.push_back(*ppChildNode);
        }
        else if (sscanf(buffer, " C: \"OP\",%llu,%llu, \"%[^\"]\"", &nodeID1, &nodeID2, &attributeType) == 3)
        {
            FbxNode** ppChildNode = model.fbxNodes.search(nodeID1);
            FbxNode** ppParentNode = model.fbxNodes.search(nodeID2);
            if (ppChildNode == nullptr || ppParentNode == nullptr) continue;

            if (strcmp(attributeType, "d|X") == 0)
            {
                AnimationCurveNode* animationCurveNode = static_cast<AnimationCurveNode*>((*ppParentNode)->nodeData);
                if (animationCurveNode == nullptr)
                    animationCurveNode = new AnimationCurveNode();
                animationCurveNode->dX = (*ppChildNode)->nodeID;
                (*ppParentNode)->nodeData = static_cast<void*>(animationCurveNode);
            }
            else if (strcmp(attributeType, "d|Y") == 0)
            {
                AnimationCurveNode* animationCurveNode = static_cast<AnimationCurveNode*>((*ppParentNode)->nodeData);
                if (animationCurveNode == nullptr)
                    animationCurveNode = new AnimationCurveNode();
                animationCurveNode->dY = (*ppChildNode)->nodeID;
                (*ppParentNode)->nodeData = static_cast<void*>(animationCurveNode);
            }
            else if (strcmp(attributeType, "d|Z") == 0)
            {
                AnimationCurveNode* animationCurveNode = static_cast<AnimationCurveNode*>((*ppParentNode)->nodeData);
                if (animationCurveNode == nullptr)
                    animationCurveNode = new AnimationCurveNode();
                animationCurveNode->dZ = (*ppChildNode)->nodeID;
                (*ppParentNode)->nodeData = static_cast<void*>(animationCurveNode);
            }
            else if (strcmp(attributeType, "Lcl Translation") == 0)
            {
                LimbNodeAnimation* limbNodeAnimation = static_cast<LimbNodeAnimation*>((*ppParentNode)->limbNodeAnimation);
                if (limbNodeAnimation == nullptr)
                    limbNodeAnimation = new LimbNodeAnimation();
                limbNodeAnimation->LclTranslation = (*ppChildNode)->nodeID;
                (*ppParentNode)->limbNodeAnimation = static_cast<void*>(limbNodeAnimation);
            }
            else if (strcmp(attributeType, "Lcl Rotation") == 0)
            {
                LimbNodeAnimation* limbNodeAnimation = static_cast<LimbNodeAnimation*>((*ppParentNode)->limbNodeAnimation);
                if (limbNodeAnimation == nullptr)
                    limbNodeAnimation = new LimbNodeAnimation();
                limbNodeAnimation->LclRot = (*ppChildNode)->nodeID;
                (*ppParentNode)->limbNodeAnimation = static_cast<void*>(limbNodeAnimation);
            }
            else if (strcmp(attributeType, "Lcl Scaling") == 0)
            {
                LimbNodeAnimation* limbNodeAnimation = static_cast<LimbNodeAnimation*>((*ppParentNode)->limbNodeAnimation);
                if (limbNodeAnimation == nullptr)
                    limbNodeAnimation = new LimbNodeAnimation();
                limbNodeAnimation->LclScl = (*ppChildNode)->nodeID;
                (*ppParentNode)->limbNodeAnimation = static_cast<void*>(limbNodeAnimation);
            }
            (*ppChildNode)->parentNode = *ppParentNode;
            (*ppParentNode)->childNodes.push_back(*ppChildNode);
        }
        else if (strstr(buffer, "}"))
            return true;
    }
    return true;
}

bool FBXLoader::ParseBindPose(FILE* file, SkinnedMeshModel& model, FbxNode* node)
{
    char buffer[4096];
    char* ptr;
    uint64_t modelNodeID;
    XMFLOAT4X4 mtxBindPose{};
    int count = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "NbPoseNodes:"))
        {
            if (sscanf(buffer, " NbPoseNodes: %d", &model.bindPose.numePoseNodes) != 1)
            {
                return false;
            }
        }
        else if (strstr(buffer, "PoseNode:"))
        {
            while (fgets(buffer, sizeof(buffer), file))
            {

                if (strstr(buffer, "Node:"))
                {
                    if (sscanf(buffer, " Node: %llu", &modelNodeID) != 1)
                    {
                        return false;
                    }
                }

                else if (strstr(buffer, "Matrix:"))
                {
                    int matrixFound = 0;
                    SimpleArray<float> matrixArray;
                    float mtxEntry;
                    while (fgets(buffer, sizeof(buffer), file))
                    {
                        ptr = buffer;
                        if (!matrixFound)
                        {
                            ptr = strstr(buffer, "a:");
                            if (ptr)
                            {
                                ptr += strlen("a:");
                                matrixFound = 1;
                            }
                        }

                        if (matrixFound)
                        {
                            while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                            if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                                break;

                            while (sscanf(ptr, "%f", &mtxEntry) == 1)
                            {
                                matrixArray.push_back(mtxEntry);

                                while (*ptr != ',' && *ptr != '\0') ptr++;
                                if (*ptr == ',') ptr++;
                            }
                        }
                    }
                    if (matrixArray.getSize() != 16)
                    {
                        return false;
                    }

                    int index = 0;
                    for (int col = 0; col < 4; col++)
                    {
                        for (int row = 0; row < 4; row++)
                        {
                            mtxBindPose.m[col][row] = matrixArray[index++];
                        }
                    }
                }
                else if (strstr(buffer, "}"))
                {
                    model.bindPose.mtxBindPoses.insert(modelNodeID, mtxBindPose);
                    count++;
                    break;
                }
                    
            }
        }
        else if (strstr(buffer, "}"))
        {
            if (count != model.bindPose.numePoseNodes)
            {
                model.bindPose.numePoseNodes = 0;
                model.bindPose.mtxBindPoses.clear();
                return false;
            }
            else
            {
                return true;
            }

        }
    }
    return false;
}

bool FBXLoader::ParseAnimationCurve(FILE* file, FbxNode* node)
{
    char buffer[4096];
    char* ptr;
    AnimationCurve* animationCurve = new AnimationCurve();
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "Default:"))
        {
            if (sscanf(buffer, " Default: %f", &animationCurve->DefaultValue) != 1)
            {
                delete animationCurve; 
                return false;
            }
        }
        else if (strstr(buffer, "KeyTime:"))
        {
            int keyTimeFound = 0;
            uint64_t keyTime;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!keyTimeFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        ptr += strlen("a:");
                        keyTimeFound = 1;
                    }
                }

                if (keyTimeFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                        break;

                    while (sscanf(ptr, "%llu", &keyTime) == 1)
                    {
                        animationCurve->KeyTime.push_back(keyTime);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }
            }
        }
        else if (strstr(buffer, "KeyValueFloat:"))
        {
            int keyValueFound = 0;
            float keyValue;
            while (fgets(buffer, sizeof(buffer), file))
            {
                ptr = buffer;
                if (!keyValueFound)
                {
                    ptr = strstr(buffer, "a:");
                    if (ptr)
                    {
                        ptr += strlen("a:");
                        keyValueFound = 1;
                    }
                }

                if (keyValueFound)
                {
                    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

                    if (!isdigit((unsigned char)*ptr) && *ptr != '-' && *ptr != '.')
                        break;

                    while (sscanf(ptr, "%f", &keyValue) == 1)
                    {
                        animationCurve->KeyValue.push_back(keyValue);

                        while (*ptr != ',' && *ptr != '\0') ptr++;
                        if (*ptr == ',') ptr++;
                    }
                }
            }
        }
        else if (strstr(buffer, "KeyAttrFlags"))
        {
            if(strstr(buffer, "{"))
                SkipNode(file);
        }
        else if (strstr(buffer, "KeyAttrDataFloat"))
        {
            if (strstr(buffer, "{"))
                SkipNode(file);
        }
        else if (strstr(buffer, "KeyAttrRefCount:"))
        {
            SkipNode(file);
        }
        else if (strstr(buffer, "}"))
        {
            if (animationCurve->KeyTime.getSize() != animationCurve->KeyValue.getSize())
            {
                delete animationCurve;
                return false;
            }
            else
            {
                animationCurve->KeyAttrRefCount = animationCurve->KeyTime.getSize();
                node->nodeData = static_cast<void*>(animationCurve);
                return true;
            }

        }

    }
    return false;
}

void FBXLoader::SkipNode(FILE* file)
{
    char buffer[4096];
    int bracket = 1;
    long int pos = 0;
    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "{"))
            bracket++;
        else if (strstr(buffer, "}"))
            bracket--;
        if (bracket == 0)
            return;

        pos = ftell(file);
    }
}

void FBXLoader::HandleDeformer(FbxNode* node, ModelData* data, SkinnedMeshModel& model, int curIdx, int prevIdx)
{
    
    Deformer* deformer = static_cast<Deformer*>(node->nodeData);
    if (deformer)
    {
        deformer->boneIdx = curIdx;
        model.mBoneHierarchy.push_back(prevIdx);
        model.deformerHashMap.insert(curIdx, node->nodeID);
        model.deformerIdxHashMap.insert(node->nodeID, curIdx);
        model.mBoneOffsets.push_back(deformer->TransformLink);
        model.mBoneToParentTransforms.push_back(deformer->Transform);

        for (int i = 0; i < deformer->Index.getSize(); i++)
        {
            int vertexIdx = deformer->Index[i];
            float weight = deformer->Weights[i];
            model.modelData->mesh.VerticesTemp[vertexIdx].BoneIndices.push_back(curIdx);
            model.modelData->mesh.VerticesTemp[vertexIdx].Weights.push_back(weight);

        }
       
        model.modelData->IndexNum += deformer->Index.getSize();

        for (int i = 0; i < node->childNodes.getSize(); i++)
        {
            if (node->childNodes[i]->nodeType == FbxNodeType::Cluster)
            {
                int prev = deformer->boneIdx;
                curIdx++;
                HandleDeformer(node->childNodes[i], data, model, curIdx, prev);
            }
            else if (node->childNodes[i]->nodeType == FbxNodeType::LimbNode)
            {
                model.deformerToLimb.insert(node->nodeID, node->childNodes[i]->nodeID);
            }

        }
    }
}


bool FBXLoader::CreateFbxNode(char* buffer, FbxNode* fbxNode)
{
    char nodeNameBuffer[128];
    char nodeTypeBuffer[128];
    char nodeBuffer[128];
    FbxNodeType nodeType = FbxNodeType::None;
    uint64_t nodeID;
    if (sscanf(buffer, " %[^:]: %lld, \"%[^\"]\", \"%[^\"]\"", nodeBuffer, &nodeID, nodeNameBuffer, nodeTypeBuffer) < 2)
        return false;
    if (strcmp(nodeTypeBuffer, "LimbNode") == 0)
        nodeType = FbxNodeType::LimbNode;
    else if (strcmp(nodeTypeBuffer, "Mesh") == 0)
        nodeType = FbxNodeType::Mesh;
    else if (strcmp(nodeTypeBuffer, "Skin") == 0)
        nodeType = FbxNodeType::Skin;
    else if (strcmp(nodeTypeBuffer, "BlendShape") == 0)
        nodeType = FbxNodeType::BlendShape;
    else if (strcmp(nodeTypeBuffer, "BlendShapeChannel") == 0)
        nodeType = FbxNodeType::BlendShapeChannel;
    else if (strcmp(nodeTypeBuffer, "Cluster") == 0)
        nodeType = FbxNodeType::Cluster;
    else if (strcmp(nodeTypeBuffer, "BindPose") == 0)
        nodeType = FbxNodeType::BindPose;
    else if (strcmp(nodeBuffer, "AnimationCurve") == 0)
        nodeType = FbxNodeType::AnimationCurve;
    else if (strcmp(nodeBuffer, "AnimationCurveNode") == 0)
        nodeType = FbxNodeType::AnimationCurveNode;

    fbxNode->nodeID = nodeID;
    strcpy(fbxNode->nodeName, nodeNameBuffer);
    fbxNode->nodeType = nodeType;

    return true;
}
