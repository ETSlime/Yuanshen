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

    model.Texture = texMgr.CreateTexture(model.textureName);
    model.modelData->IndexNum = model.modelData->IndicesData.getSize();
    model.modelData->VertexNum = model.modelData->IndexNum;
    model.modelData->VertexArray = new VERTEX_3D[model.modelData->VertexNum];
    model.modelData->IndexArray = new unsigned short[model.modelData->IndexNum];
    for (int i = 0; i < model.modelData->IndexNum; i++)
    {
        VERTEX_3D vertex;
        int vertexIndex = model.modelData->IndicesData[i].Index;
        if (vertexIndex < 0)
        {
            vertexIndex = -vertexIndex;
            vertexIndex--;
        }

        
        vertex.Position = model.modelData->VerticesTemp[vertexIndex].Position;

        if (model.normalLoc == Vertex)
            vertex.Normal = model.modelData->VerticesTemp[vertexIndex].Normal;
        else if (model.normalLoc == Index)
            vertex.Normal = model.modelData->IndicesData[i].Normal;

        if (model.texLoc == Vertex)
            vertex.TexCoord = model.modelData->VerticesTemp[vertexIndex].TexCoord;
        else if (model.texLoc == Index)
        {
            vertex.TexCoord = model.modelData->IndicesData[i].TexCoord;

            vertex.TexCoord.y = -vertex.TexCoord.y;
        }
            

        model.modelData->VertexArray[i] = vertex;
        model.modelData->IndexArray[i] = i;
    }

    // 頂点バッファ生成
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(VERTEX_3D) * model.modelData->VertexNum;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.pSysMem = model.modelData->VertexArray;

        GetDevice()->CreateBuffer(&bd, &sd, &model.VertexBuffer);
    }

    // インデックスバッファ生成
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(unsigned short) * model.modelData->IndexNum;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.pSysMem = model.modelData->IndexArray;

        GetDevice()->CreateBuffer(&bd, &sd, &model.IndexBuffer);
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
                            if (!ParseModelProperty(file, model.globalModelProperty))
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
                SkipNode(file);
            }
            if (strstr(buffer, "Geometry:")) 
            {
                char GeometryName[128], GeometryType[128];
                if (sscanf(buffer, " Geometry: %*[^,], \"%[^\"]\", \"%[^\"]\"", GeometryName, GeometryType) != 2)
                    return false; // Geometryの解析が失敗した
                if (!ParseGeometry(file, model)) 
                    return false;  // Geometryの解析が失敗した
            }
            // Modelフィールドを解析
            if (strstr(buffer, "Model:"))
            {
                char modelName[128], modelType[128];
                if (sscanf(buffer, " Model: %*[^,], \"%[^\"]\", \"%[^\"]\"", modelName, modelType) != 2)
                    return false; // Modelの解析が失敗した
                if (!ParseModel(file, model))
                    return false;  // Modelの解析が失敗した
            }
            if (strstr(buffer, "Pose:"))
            {
                SkipNode(file);
            }
            // Materialフィールドを解析
            if (strstr(buffer, "Material:"))
            {
                char materialName[128];
                if (sscanf(buffer, " Material: %*[^,], \"%[^\"]\", \"%[^\"]\"", materialName) != 1)
                    return false; // Materialの解析が失敗した
                if (!ParseMaterial(file, model))
                    return false;  // Materialの解析が失敗した
            }
            if (strstr(buffer, "Deformer:"))
            {
                SkipNode(file);
            }
            if (strstr(buffer, "Video:"))
            {
                SkipNode(file);
            }
            // Textureフィールドを解析
            if (strstr(buffer, "Texture:"))
            {
                if (model.textureName[0] != '\0')
                    SkipNode(file);
                else
                {
                    char TextureName[128];
                    if (sscanf(buffer, " Texture: %*[^,], \"%[^\"]\", \"%[^\"]\"", TextureName) != 1)
                        return false; // Textureの解析が失敗した
                    if (!ParseTexture(file, model))
                        return false;  // Textureの解析が失敗した
                }

            }
            if (strstr(buffer, "AnimationCurve"))
            {
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

bool FBXLoader::ParseGeometry(FILE* file, SkinnedMeshModel& model)
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

            if (!ParseVertexData(file, model))
                return false;
        }
        else if (strstr(buffer, "PolygonVertexIndex:"))
        {
            //fseek(file, position, SEEK_SET);

            if (!ParseIndexData(file, model))
                return false;
        }
        else if (strstr(buffer, "LayerElementNormal:"))
        {
            if (!ParseNormal(file, model))
                return false;     
        }
        else if (strstr(buffer, "LayerElementSmoothing:"))
        {
            while (!strstr(buffer, "}"))
                fgets(buffer, sizeof(buffer), file);
        }
        else if (strstr(buffer, "LayerElementUV:"))
        {
            if (!ParseUV(file, model))
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

bool FBXLoader::ParseModel(FILE* file, SkinnedMeshModel& model)
{
    char buffer[2048];
    int readCount;
    long int position = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (strstr(buffer, "Properties70:"))
        {
            if (!ParseModelProperty(file, model.modelProperty))
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

bool FBXLoader::ParseModelProperty(FILE* file, ModelProperty& modelProperty)
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
            return true;
        }

        memset(buffer, 0, sizeof(buffer));
        pos = ftell(file);
    }
}

bool FBXLoader::ParseVertexData(FILE* file, SkinnedMeshModel& model)
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
                    VERTEX_3D newVertex;
                    newVertex.Position.x = vertexPos[0];
                    newVertex.Position.y = vertexPos[1];
                    newVertex.Position.z = vertexPos[2];
                    model.modelData->VerticesTemp.push_back(newVertex);
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
        model.modelData->Vertices.clear();
        return false;
    }
    else
    {
        if (!strstr(buffer, "}"))
            fseek(file, position, SEEK_SET);
        return true;
    }
}

bool FBXLoader::ParseIndexData(FILE* file, SkinnedMeshModel& model)
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
                model.modelData->IndicesData.push_back(indexData);
                count++;

                if (count == polygonSize)
                    count = 0;

                while (*ptr != ',' && *ptr != '\0') ptr++;
                if (*ptr == ',') ptr++;
            }
        }

        position = ftell(file);
    }

    if (count > 0 && count < 4)
    {
        model.modelData->Vertices.clear();
        return false;
    }
    else
    {
        if (!strstr(buffer, "}"))
            fseek(file, position, SEEK_SET);
        return true;
    }
}

bool FBXLoader::ParseNormal(FILE* file, SkinnedMeshModel& model)
{
    char buffer[4096];
    int readCount;
    char MappingInformationTypeBuffer[32];
    char ReferenceInformationTypeBuffer[32];
    float normal;
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

            int normalCount = 0;
            int totalNormalCount = 0;
            int vertexCount = 0;
            float normalsByVertex[3];
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
                                if (vertexCount >= model.modelData->IndicesData.getSize())
                                {
                                    model.modelData->Vertices.clear();
                                    return false;
                                }
                                model.modelData->IndicesData[vertexCount].Normal.x = normalsByVertex[0];
                                model.modelData->IndicesData[vertexCount].Normal.y = normalsByVertex[1];
                                model.modelData->IndicesData[vertexCount].Normal.z = normalsByVertex[2];
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
                                if (vertexCount >= model.modelData->VerticesTemp.getSize())
                                {
                                    model.modelData->Vertices.clear();
                                    return false;
                                }
                                model.modelData->VerticesTemp[vertexCount].Normal.x = normalsByVertex[0];
                                model.modelData->VerticesTemp[vertexCount].Normal.y = normalsByVertex[1];
                                model.modelData->VerticesTemp[vertexCount].Normal.z = normalsByVertex[2];
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
            if (mappingInformationType == ByPolygon
                && vertexCount != model.modelData->Vertices.getSize())
                return false;
            else if (mappingInformationType == ByPolygonVertex
                && vertexCount != model.modelData->IndicesData.getSize())
                return false;

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

bool FBXLoader::ParseUV(FILE* file, SkinnedMeshModel& model)
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
                if (UVCoordIndexSize != model.modelData->IndicesData.getSize())
                    return false;

                for (int i = 0; i < UVCoordIndexSize; i++)
                {                    
                    int coordIndex = UVCoordIndex[i];

                    model.modelData->IndicesData[i].TexCoord.x = UVCoord[coordIndex * 2];
                    model.modelData->IndicesData[i].TexCoord.y = UVCoord[coordIndex * 2 + 1];
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
    return true;
}

void FBXLoader::SkipNode(FILE* file)
{
    char buffer[512];
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
