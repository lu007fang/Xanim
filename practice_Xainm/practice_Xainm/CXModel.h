#pragma once
#ifndef _CXMODEL_H_
#define _CXMODEL_H_

#include "xStructuers.h"

class CXModel
{
public:
	CXModel(LPDIRECT3DDEVICE9 device);
	~CXModel(){shutdown();}

	bool LoadXfile(char* file);

	void shutdown();

	void GetBoundingSphere(D3DXVECTOR3* center,float* radius)
	{
		if(center)  *center= m_center; 
		if(radius)  *radius= m_radius; 
	}

	void Update(float time,const D3DXMATRIX* matrixWorld);
	void Render();

	void SetAnimation(unsigned int index);
	void NextAnimation();

	unsigned int GetCurrentAnimtion() {return m_currentAnim;}
	unsigned int GetTotalAnimation() {return m_numAnims;}

	float GetAnimationSpeed() {return m_speed;}
	void IncreaseAnimationSpeed(float amt) {m_speed += amt;}
	void DecreaseAnimationSpeed(float amt)
	{
		if(m_speed > 0.1f)
			m_speed -= amt;
	}

	float GetTransitionSpeed() {return m_transition;}
	void IncreaseTransition(float amt) { m_transition += amt;}
	void DecreaseTransition(float amt)
	{
		if(m_transition > 0.1f)
			m_transition -= amt;
	}

	private:
		void SetupMatrices(stD3DFrameEx* inFrame,LPD3DXMATRIX parentMatrix);
		void UpdateMatrices(const D3DXFRAME* inFrame,const D3DXMATRIX* parentMatrix);
		void DrawFrame(LPD3DXFRAME frame);


private:
	LPDIRECT3DDEVICE9 m_device;

	LPD3DXFRAME m_root;
	LPD3DXANIMATIONCONTROLLER m_animControl;

	D3DXMATRIX* m_boneMatrices;
	unsigned int m_maxBones;

	stD3DContainerEx* m_currentContainer;


	D3DXVECTOR3 m_center;
	float m_radius;

	unsigned int m_currentAnim;
	unsigned int m_numAnims;
	unsigned int m_currentTrack;
	float m_speed;
	float m_transition;
	float m_currentTime;
};


#endif