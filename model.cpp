//=============================================================================
//
// モデルの処理 [model.cpp]
// Author : 
//
//=============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include "main.h"
#include "model.h"
#include "camera.h"
#include "input.h"
#include "debugproc.h"
#include "MapEditor.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VALUE_MOVE_MODEL	(0.50f)					// 移動速度
#define	RATE_MOVE_MODEL		(0.20f)					// 移動慣性係数
#define	VALUE_ROTATE_MODEL	(XM_PI * 0.05f)			// 回転速度
#define	RATE_ROTATE_MODEL	(0.20f)					// 回転慣性係数
#define	SCALE_MODEL			(10.0f)					// 回転慣性係数
#define MAX_MODEL_NUM		(30)

//*****************************************************************************
// グローバル変数
//*****************************************************************************
HashMap<char*, MODEL_POOL, CharPtrHash, CharPtrEquals> Model::modelHashMap(
	MAX_MODEL_NUM,
	CharPtrHash(),
	CharPtrEquals()
);

//=============================================================================
// 初期化処理
//=============================================================================
Model::Model(char *FileName)
{
	MODEL_DATA* model = new MODEL_DATA;
	LoadObj(FileName, model);

	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( VERTEX_3D ) * model->VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = model->VertexArray;

		renderer.GetDevice()->CreateBuffer( &bd, &sd, &this->VertexBuffer );


		bd.ByteWidth = sizeof(VERTEX_3D) * 24;
		renderer.GetDevice()->CreateBuffer(&bd, NULL, &this->BBVertexBuffer);

		CreateBoundingBoxVertex();
	}

	char* filename = "data/MODEL/neko.jpg";
	ID3D11ShaderResourceView* srv = nullptr;
	D3DX11CreateShaderResourceViewFromFile(renderer.GetDevice(),
		filename,
		NULL,
		NULL,
		&srv,
		NULL);

	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( unsigned int ) * model->IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = model->IndexArray;

		renderer.GetDevice()->CreateBuffer( &bd, &sd, &this->IndexBuffer );
	}

	// サブセット設定
	{
		for( unsigned int i = 0; i < this->SubsetNum; i++ )
		{
			this->SubsetArray[i].StartIndex = this->SubsetArray[i].StartIndex;
			this->SubsetArray[i].IndexNum = this->SubsetArray[i].IndexNum;


			D3DX11CreateShaderResourceViewFromFile(renderer.GetDevice(),
				this->SubsetArray[i].Material.TextureName,
													NULL,
													NULL,
													&this->SubsetArray[i].Texture,
													NULL );
		}
	}

	this->modelData = model;
}


//=============================================================================
// 終了処理
//=============================================================================
Model::~Model()
{

	if( this->VertexBuffer )		this->VertexBuffer->Release();
	if( this->IndexBuffer )		this->IndexBuffer->Release();
	if( this->SubsetArray )		delete[] this->SubsetArray;
	if (this->BBVertexBuffer)	this->BBVertexBuffer->Release();
	if (this->modelData)		delete modelData;

}


//=============================================================================
// 描画処理
//=============================================================================
void Model::DrawModel()
{
	// 頂点バッファ設定
	UINT stride = sizeof( VERTEX_3D );
	UINT offset = 0;
	renderer.GetDeviceContext()->IASetVertexBuffers( 0, 1, &this->VertexBuffer, &stride, &offset );

	// インデックスバッファ設定
	renderer.GetDeviceContext()->IASetIndexBuffer( this->IndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	// プリミティブトポロジ設定
	renderer.GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	for( unsigned int i = 0; i < this->SubsetNum; i++ )
	{
		// マテリアル設定
		if (this->SubsetArray[i].Material.MaterialData.LoadMaterial)
			renderer.SetMaterial( this->SubsetArray[i].Material.MaterialData);

		// テクスチャ設定
		if (this->SubsetArray[i].Material.MaterialData.noTexSampling == 0)
		{
			renderer.GetDeviceContext()->PSSetShaderResources(0, 1, &this->SubsetArray[i].Texture);
		}

		// ポリゴン描画
		renderer.GetDeviceContext()->DrawIndexed( this->SubsetArray[i].IndexNum, this->SubsetArray[i].StartIndex, 0 );
	}


}

void Model::DrawBoundingBox()
{
	if (renderer.GetRenderMode() == RENDER_MODE_SHADOW) return;

	renderer.SetFillMode(D3D11_FILL_WIREFRAME);
	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	renderer.GetDeviceContext()->IASetVertexBuffers(0, 1, &this->BBVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	renderer.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.noTexSampling = TRUE;
	renderer.SetMaterial(material);

	renderer.GetDeviceContext()->Draw(24, 0);
	renderer.SetFillMode(D3D11_FILL_SOLID);
}



//モデル読込////////////////////////////////////////////
void Model::LoadObj( char *FileName, MODEL_DATA* Model )
{

	XMFLOAT3	*positionArray;
	XMFLOAT3	*normalArray;
	XMFLOAT2	*texcoordArray;

	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	MODEL_MATERIAL	*materialArray = NULL;
	unsigned int	materialNum = 0;

	char str[256];
	char *s;
	char c;


	FILE *file;
	file = fopen( FileName, "rt" );
	if( file == NULL )
	{
		printf( "エラー:LoadModel %s \n", FileName );
		return;
	}



	//要素数カウント
	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "v" ) == 0 )
		{
			positionNum++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			normalNum++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			texcoordNum++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			subsetNum++;
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			in = 0;

			do
			{
				fscanf( file, "%s", str );
				vertexNum++;
				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c!= '\r' );

			//四角は三角に分割
			if( in == 4 )
				in = 6;

			indexNum += in;
		}
	}


	//メモリ確保
	positionArray = new XMFLOAT3[ positionNum ];
	normalArray   = new XMFLOAT3[ normalNum ];
	texcoordArray = new XMFLOAT2[ texcoordNum ];


	Model->VertexArray = new VERTEX_3D[ vertexNum ];
	Model->VertexNum = vertexNum;

	Model->IndexArray = new unsigned int[ indexNum ];
	Model->IndexNum = indexNum;

	this->SubsetArray = new SUBSET[ subsetNum ];
	this->SubsetNum = subsetNum;


	this->boundingBox.minPoint = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	this->boundingBox.maxPoint = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	//要素読込
	XMFLOAT3 *position = positionArray;
	XMFLOAT3 *normal = normalArray;
	XMFLOAT2 *texcoord = texcoordArray;

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;


	fseek( file, 0, SEEK_SET );

	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "mtllib" ) == 0 )
		{
			//マテリアルファイル
			fscanf( file, "%s", str );

			char path[256];
		//	strcpy( path, "data/model/" );

			//----------------------------------- フォルダー対応
			strcpy(path, FileName);
			char* adr = path;
			char* ans = adr;
			while (1)
			{
				adr = strstr(adr, "/");
				if (adr == NULL) break;
				else ans = adr;
				adr++;
			}
			if (path != ans) ans++;
			*ans = 0;
			//-----------------------------------

			strcat( path, str );

			LoadMaterial( path, &materialArray, &materialNum );
		}
		else if( strcmp( str, "o" ) == 0 )
		{
			//オブジェクト名
			fscanf( file, "%s", str );
		}
		else if( strcmp( str, "v" ) == 0 )
		{
			//頂点座標
			fscanf( file, "%f", &position->x );
			fscanf( file, "%f", &position->y );
			fscanf( file, "%f", &position->z );
			position->x *= SCALE_MODEL;
			position->y *= SCALE_MODEL;
			position->z *= SCALE_MODEL;

			// Update bounding box
			this->boundingBox.minPoint.x = min(this->boundingBox.minPoint.x, position->x);
			this->boundingBox.minPoint.y = min(this->boundingBox.minPoint.y, position->y);
			this->boundingBox.minPoint.z = min(this->boundingBox.minPoint.z, position->z);

			this->boundingBox.maxPoint.x = max(this->boundingBox.maxPoint.x, position->x);
			this->boundingBox.maxPoint.y = max(this->boundingBox.maxPoint.y, position->y);
			this->boundingBox.maxPoint.z = max(this->boundingBox.maxPoint.z, position->z);


			position++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			//法線
			fscanf( file, "%f", &normal->x );
			fscanf( file, "%f", &normal->y );
			fscanf( file, "%f", &normal->z );
			normal++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			//テクスチャ座標
			fscanf( file, "%f", &texcoord->x );
			fscanf( file, "%f", &texcoord->y );
			texcoord->y = 1.0f - texcoord->y;
			texcoord++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			//マテリアル
			fscanf( file, "%s", str );

			if( sc != 0 )
				this->SubsetArray[ sc - 1 ].IndexNum = ic - this->SubsetArray[ sc - 1 ].StartIndex;

			this->SubsetArray[ sc ].StartIndex = ic;


			for( unsigned int i = 0; i < materialNum; i++ )
			{
				if( strcmp( str, materialArray[i].Name ) == 0 )
				{
					this->SubsetArray[ sc ].Material.MaterialData = materialArray[i].MaterialData;
					this->SubsetArray[sc].Material.MaterialData.LoadMaterial = TRUE;
					strcpy(this->SubsetArray[ sc ].Material.TextureName, materialArray[i].TextureName );
					strcpy(this->SubsetArray[ sc ].Material.Name, materialArray[i].Name );

					break;
				}
			}

			sc++;
			
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			//面
			in = 0;

			do
			{
				fscanf( file, "%s", str );

				s = strtok( str, "/" );	
				Model->VertexArray[vc].Position = positionArray[ atoi( s ) - 1 ];
				if( s[ strlen( s ) + 1 ] != '/' )
				{
					//テクスチャ座標が存在しない場合もある
					s = strtok( NULL, "/" );
					Model->VertexArray[vc].TexCoord = texcoordArray[ atoi( s ) - 1 ];
				}
				s = strtok( NULL, "/" );	
				Model->VertexArray[vc].Normal = normalArray[ atoi( s ) - 1 ];

				Model->VertexArray[vc].Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

				Model->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' );

			//四角は三角に分割
			if( in == 4 )
			{
				Model->IndexArray[ic] = vc - 4;
				ic++;
				Model->IndexArray[ic] = vc - 2;
				ic++;
			}
		}
	}


	if( sc != 0 )
		this->SubsetArray[ sc - 1 ].IndexNum = ic - this->SubsetArray[ sc - 1 ].StartIndex;




	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	delete[] materialArray;

	fclose(file);
}




//マテリアル読み込み///////////////////////////////////////////////////////////////////
void Model::LoadMaterial( char *FileName, MODEL_MATERIAL **MaterialArray, unsigned int *MaterialNum )
{
	char str[256];

	FILE *file;
	file = fopen( FileName, "rt" );
	if( file == NULL )
	{
		printf( "エラー:LoadMaterial %s \n", FileName );
		return;
	}

	MODEL_MATERIAL *materialArray;
	unsigned int materialNum = 0;

	//要素数カウント
	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			materialNum++;
		}
	}


	//メモリ確保
	materialArray = new MODEL_MATERIAL[ materialNum ];
	ZeroMemory(materialArray, sizeof(MODEL_MATERIAL)*materialNum);


	//要素読込
	int mc = -1;

	fseek( file, 0, SEEK_SET );

	while( TRUE )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;


		if( strcmp( str, "newmtl" ) == 0 )
		{
			//マテリアル名
			mc++;
			fscanf( file, "%s", materialArray[ mc ].Name );
			strcpy( materialArray[ mc ].TextureName, "" );
			materialArray[mc].MaterialData.noTexSampling = 1;
		}
		else if( strcmp( str, "Ka" ) == 0 )
		{
			//アンビエント
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Ambient.x );
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Ambient.y );
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Ambient.z );
			materialArray[ mc ].MaterialData.Ambient.w = 1.0f;
		}
		else if( strcmp( str, "Kd" ) == 0 )
		{
			//ディフューズ
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Diffuse.x );
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Diffuse.y );
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Diffuse.z );

			// Mayaでテクスチャを貼ると0.0fになっちゃうみたいなので
			if ((materialArray[mc].MaterialData.Diffuse.x + materialArray[mc].MaterialData.Diffuse.y + materialArray[mc].MaterialData.Diffuse.z) == 0.0f)
			{
				materialArray[mc].MaterialData.Diffuse.x = materialArray[mc].MaterialData.Diffuse.y = materialArray[mc].MaterialData.Diffuse.z = 1.0f;
			}

			materialArray[ mc ].MaterialData.Diffuse.w = 1.0f;
		}
		else if( strcmp( str, "Ks" ) == 0 )
		{
			//スペキュラ
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Specular.x );
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Specular.y );
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Specular.z );
			materialArray[ mc ].MaterialData.Specular.w = 1.0f;
		}
		else if( strcmp( str, "Ns" ) == 0 )
		{
			//スペキュラ強度
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Shininess );
		}
		else if( strcmp( str, "d" ) == 0 )
		{
			//アルファ
			fscanf( file, "%f", &materialArray[ mc ].MaterialData.Diffuse.w );
		}
		else if( strcmp( str, "map_Kd" ) == 0 )
		{
			//テクスチャ
			fscanf( file, "%s", str );

			char path[256];
		//	strcpy( path, "data/model/" );

			//----------------------------------- フォルダー対応
			strcpy(path, FileName);
			char* adr = path;
			char* ans = adr;
			while (1)
			{
				adr = strstr(adr, "/");
				if (adr == NULL) break;
				else ans = adr;
				adr++;
			}
			if (path != ans) ans++;
			*ans = 0;
			//-----------------------------------

			strcat( path, str );

			strcat( materialArray[ mc ].TextureName, path );
			materialArray[mc].MaterialData.noTexSampling = 0;
		}
	}


	*MaterialArray = materialArray;
	*MaterialNum = materialNum;

	fclose(file);
}


// モデルの全マテリアルのディフューズを取得する。Max16個分にしてある
void Model::GetModelDiffuse(XMFLOAT4 *diffuse)
{
	unsigned int max = (this->SubsetNum < MODEL_MAX_MATERIAL) ? this->SubsetNum : MODEL_MAX_MATERIAL;

	for (unsigned int i = 0; i < max; i++)
	{
		// ディフューズ設定
		diffuse[i] = this->SubsetArray[i].Material.MaterialData.Diffuse;
	}
}


// モデルの指定マテリアルのディフューズをセットする。
void Model::SetModelDiffuse(int mno, XMFLOAT4 diffuse)
{
	// ディフューズ設定
	this->SubsetArray[mno].Material.MaterialData.Diffuse = diffuse;
}

void Model::CreateBoundingBoxVertex()
{
	D3D11_MAPPED_SUBRESOURCE msr;
	renderer.GetDeviceContext()->Map(this->BBVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.minPoint.y, this->boundingBox.minPoint.z);
	vertex[1].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.minPoint.z);
	vertex[2].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.minPoint.z);

	vertex[3].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.minPoint.z);
	vertex[4].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.minPoint.z);
	vertex[5].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.minPoint.z);

	vertex[6].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.minPoint.y, this->boundingBox.maxPoint.z);
	vertex[7].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.maxPoint.z);
	vertex[8].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.maxPoint.z);

	vertex[9].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.maxPoint.z);
	vertex[10].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.maxPoint.z);
	vertex[11].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.maxPoint.z);

	vertex[12].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.minPoint.z);
	vertex[13].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.minPoint.z);
	vertex[14].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.maxPoint.z);

	vertex[15].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.minPoint.z);
	vertex[16].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.maxPoint.z);
	vertex[17].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.maxPoint.y, this->boundingBox.maxPoint.z);

	vertex[18].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.minPoint.y, this->boundingBox.minPoint.z);
	vertex[19].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.minPoint.z);
	vertex[20].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.minPoint.y, this->boundingBox.maxPoint.z);

	vertex[21].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.minPoint.z);
	vertex[22].Position = XMFLOAT3(this->boundingBox.maxPoint.x, this->boundingBox.minPoint.y, this->boundingBox.maxPoint.z);
	vertex[23].Position = XMFLOAT3(this->boundingBox.minPoint.x, this->boundingBox.minPoint.y, this->boundingBox.maxPoint.z);


	// 法線の設定
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[4].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[5].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[6].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[7].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[8].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[9].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[10].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[11].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[12].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[13].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[14].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[15].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[16].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[17].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[18].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[19].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[20].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[21].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[22].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[23].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[4].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[5].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[6].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[7].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[8].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[9].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[10].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[11].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[12].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[13].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[14].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[15].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[16].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[17].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[18].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[19].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[20].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[21].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[22].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[23].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[4].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[5].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[6].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[7].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[8].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[9].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[10].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[11].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[12].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[13].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[14].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[15].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[16].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[17].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[18].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[19].TexCoord = XMFLOAT2(1.0f, 1.0f);
	vertex[20].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[21].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[22].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[23].TexCoord = XMFLOAT2(1.0f, 1.0f);

	renderer.GetDeviceContext()->Unmap(this->BBVertexBuffer, 0);
}

Model* Model::StoreModel(char* modelPath)
{
	MODEL_POOL* modelPool = GetModel(modelPath);
	if (modelPool == nullptr)
	{
		modelPool = new MODEL_POOL;
		modelPool->pModel = new Model(modelPath);
		modelPool->count = 1;
		modelHashMap.insert(modelPath, *modelPool);
	}
	else
		modelPool->count++;

	return modelPool->pModel;

}

MODEL_POOL* Model::GetModel(char* modelPath)
{
	return modelHashMap.search(modelPath);
}

void Model::RemoveModel(char* modelPath)
{
	modelHashMap.remove(modelPath);
}
