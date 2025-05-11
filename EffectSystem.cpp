//=============================================================================
//
// EffectSystem���� [EffectSystem.cpp]
// Author : 
//
//=============================================================================
#include "EffectSystem.h"
#include "SmokeEffectRenderer.h"
#include "FireEffectRenderer.h"

void EffectSystem::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_device = device;
	m_context = context;

	// ���炩���߃��U�[�u���ă������m�ہi���萔�l�j
	m_allEffects.reserve(MAX_EFFECT_NUM);
	m_fireEffects.reserve(MAX_FIRE_EFFECT_NUM);
	m_smokeEffects.reserve(MAX_SMOKE_EFFECT_NUM);
	m_softBodyEffects.reserve(MAX_SOFTBODY_EFFECT_NUM);
}

void EffectSystem::Shutdown(void)
{
	for (auto& effect : m_allEffects)
	{
		effect->Shutdown();
		delete effect;
	}
}

void EffectSystem::Update(void)
{
	for (auto& effect : m_allEffects)
	{
		effect->Update();
	}
}

void EffectSystem::Draw(const XMMATRIX& viewProj)
{
	// ���t���[�����ŕ��ރN���A
	m_fireEffects.clear();
	m_smokeEffects.clear();
	m_softBodyEffects.clear();

	// ���ޏ���
	for (auto effect : m_allEffects)
	{
		switch (effect->GetEffectType())
		{
		case EffectType::Fire:
			m_fireEffects.push_back(effect);
			break;
		case EffectType::Smoke:
			m_smokeEffects.push_back(effect);
			break;
		case EffectType::SoftBody:
			m_softBodyEffects.push_back(effect);
			break;
		default:
			break;
		}
	}

	EffectType lastType = EffectType::None; // �ŏ��̃_�~�[�l

	// �t�@�C���[�t�F�[�N�g�`��
	if (!m_fireEffects.empty())
	{
		if (lastType != EffectType::Fire)
		{
			m_context->GSSetShader(nullptr, nullptr, 0);
			FireEffectRenderer::ResetPipelineState(); // �N���X�P�ʂŃ��Z�b�g
		}

		for (auto fire : m_fireEffects)
		{
			fire->SetupPipeline();
			fire->Draw(viewProj);
		}

		lastType = EffectType::Fire;
	}

	// �X���[�N�t�F�[�N�g�`��
	if (!m_smokeEffects.empty())
	{
		if (lastType != EffectType::Smoke)
		{
			m_context->GSSetShader(nullptr, nullptr, 0);
			SmokeEffectRenderer::ResetPipelineState(); // �N���X�P�ʂŃ��Z�b�g
		}

		for (auto smoke : m_smokeEffects)
		{
			smoke->SetupPipeline();
			smoke->Draw(viewProj);
		}

		lastType = EffectType::Smoke;
	}

	// �\�t�g�{�f�B�t�F�[�N�g�`��
	if (!m_softBodyEffects.empty())
	{
		if (lastType != EffectType::SoftBody)
		{
			m_context->GSSetShader(nullptr, nullptr, 0);
			//SoftBodyRenderer::ResetPipelineState(); // �N���X�P�ʂŃ��Z�b�g
		}

		for (auto softBody : m_softBodyEffects)
		{
			softBody->SetupPipeline();
			softBody->Draw(viewProj);
		}

		lastType = EffectType::SoftBody;
	}	

	// �Ō�Ƀp�C�v���C����Ԃ����Z�b�g
	ClearAllEffectBindings();
}

ParticleEffectRendererBase* EffectSystem::SpawnParticleEffect(ParticleEffectParams params)
{
	if (m_allEffects.getSize() >= MAX_EFFECT_NUM)
		return nullptr; // �ő吔�ɒB���Ă���ꍇ�͐������Ȃ�

	// �p�[�e�B�N�����̐���
	if (params.numParticles > MAX_PARTICLES)
		params.numParticles = MAX_PARTICLES; // �ő吔�ɐ���
	else if (params.numParticles < 1)
		params.numParticles = 1; // �ŏ����ɐ���

	ParticleEffectRendererBase* effect = nullptr;

	// �^�C�v�ɉ����ăC���X�^���X����
	switch (params.type)
	{
	case EffectType::Fire:
		//effect = new FireEffectRenderer();
		break;
	case EffectType::Smoke:
		effect = new SmokeEffectRenderer();
		break;
	default:
		return nullptr; // ���Ή��^�C�v
	}

	if (!effect)
		return nullptr;

	effect->ConfigureEffect(params); // ������
	if (effect->Initialize(m_device, m_context))
	{
		m_allEffects.push_back(effect);
		return effect;
	}
	else
	{
		delete effect;
		return nullptr;
	}
}

void EffectSystem::RemoveEffect(IEffectRenderer* effect)
{
	int index = m_allEffects.find_index(effect);
	if (index >= 0)
	{
		m_allEffects[index]->Shutdown();
		delete m_allEffects[index];
		m_allEffects.erase(index);
	}
}

void EffectSystem::ClearAllEffectBindings(void)
{
	ID3D11ShaderResourceView* nullSRV = nullptr;

	m_ShaderResourceBinder.BindShaderResource(ShaderStage::VS, SLOT_SRV_PARTICLE, nullSRV);
	m_ShaderResourceBinder.BindShaderResource(ShaderStage::PS, SLOT_TEX_DIFFUSE, nullSRV);
	m_context->GSSetShader(nullptr, nullptr, 0);
}
