#pragma once

#include "main.h"
#include "renderer.h"
#include "HashMap.h"

#define MAX_SRV			222

struct SRV_POOL
{
	ID3D11ShaderResourceView* srv;
	unsigned int count;

	SRV_POOL() : srv(nullptr), count(0) {}
	~SRV_POOL()
	{
		Release();
	}

	SRV_POOL(const SRV_POOL& other) = delete;
	SRV_POOL& operator=(const SRV_POOL& other) = delete;

	SRV_POOL(SRV_POOL&& other) noexcept
		: srv(other.srv), count(other.count)
	{
		other.srv = nullptr;
		other.count = 0;
	}

	// move
	SRV_POOL& operator=(SRV_POOL&& other) noexcept
	{
		if (this != &other)
		{
			Release();
			srv = other.srv;
			count = other.count;
			other.srv = nullptr;
			other.count = 0;
		}
		return *this;
	}

	void AddRef() { count++; }

	void Release()
	{
		if (srv && --count == 0)
		{
			srv->Release();
			srv = nullptr;
		}
	}
};

class TextureMgr
{
public:
	TextureMgr();
	~TextureMgr();

	void Init(ID3D11Device* device);
	void Uninit(void);

	ID3D11ShaderResourceView* CreateTexture(char* filename);

private:
	TextureMgr(const TextureMgr& rhs);
	TextureMgr& operator=(const TextureMgr& rhs) = default;

private:
	ID3D11Device* md3dDevice;
	static HashMap<char*, SRV_POOL*, CharPtrHash, CharPtrEquals> mTextureSRV;
	Renderer& renderer = Renderer::get_instance();
};