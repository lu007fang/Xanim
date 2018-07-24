#include "CXModel.h"


CXModel::CXModel(LPDIRECT3DDEVICE9 device)
{
	m_device = device;

	m_root = NULL;
	m_animControl = NULL;

	m_boneMatrices = NULL;
	m_maxBones = 0;

	m_currentContainer = NULL;


	m_center = D3DXVECTOR3(0,0,0);
	m_radius = 0;

	m_currentAnim = 0;
	m_numAnims = 0;
	m_currentTrack = 0;
	m_speed = 1.0f;
	m_transition = 0.25f;
	m_currentTime = 0;

}

bool CXModel::LoadXfile(char* file)
{
	if(!m_device)
		return false;
	
	CD3DAllocate alh;

	if(FAILED(D3DXLoadMeshHierarchyFromX(file,D3DXMESH_MANAGED,m_device,&alh,NULL,&m_root,&m_animControl)))
		return false;

	if(m_animControl)
		m_numAnims = m_animControl->GetMaxNumAnimationSets();
	
	if(m_root)
	{
		SetupMatrices((stD3DFrameEx*)m_root,NULL);
		m_boneMatrices = new D3DXMATRIX[m_maxBones];
		ZeroMemory(m_boneMatrices,sizeof(D3DXMATRIX)*m_maxBones);

		D3DXFrameCalculateBoundingSphere(m_root,&m_center,&m_radius);
	}

	SetAnimation(0);

	return true;
}

void CXModel::shutdown()
{
	if(m_root)
	{
		CD3DAllocate alh;
		D3DXFrameDestroy(m_root,&alh);
		m_root=NULL;
	}
	if(m_animControl)
		m_animControl->Release();
		
	m_animControl = NULL;

	if(m_boneMatrices)
		delete[] m_boneMatrices;
	m_boneMatrices = NULL;

}




void CXModel::Update(float time,const D3DXMATRIX* matrixWorld)
{
	time /= m_speed;
	m_currentTime += time;

	if(m_animControl)
		m_animControl->AdvanceTime(time,NULL);

	UpdateMatrices(m_root,matrixWorld);

	stD3DContainerEx* pMesh = m_currentContainer;

	if(pMesh && pMesh->pSkinInfo)
	{
		unsigned int numBones = pMesh->pSkinInfo->GetNumBones();

		for(unsigned int i=0;i<numBones;i++)
			D3DXMatrixMultiply(&m_boneMatrices[i], pMesh->pSkinInfo->GetBoneOffsetMatrix(i), pMesh->boneMatrices[i]);

			void* srcPtr;
			pMesh->MeshData.pMesh->LockVertexBuffer(D3DLOCK_READONLY,(void**)&srcPtr);
			void* destPtr;
			pMesh->originMesh->LockVertexBuffer(D3DLOCK_READONLY,(void**)&destPtr);

			pMesh->pSkinInfo->UpdateSkinnedMesh(m_boneMatrices,NULL,srcPtr,destPtr);

			pMesh->originMesh->UnlockVertexBuffer();
			pMesh->MeshData.pMesh->UnlockVertexBuffer();
		
	}
	

}

void CXModel::Render()
{
	if(m_root)
		DrawFrame(m_root);
}

void CXModel::DrawFrame(LPD3DXFRAME frame)
{
	if(!m_device)
		return;
	
	LPD3DXMESHCONTAINER meshContainer = frame->pMeshContainer;

	while(meshContainer)
	{
		stD3DFrameEx* frameEx = (stD3DFrameEx*)frame;
		m_device->SetTransform(D3DTS_WORLD,&frameEx->finalMatrix);
		
		stD3DContainerEx* containerEx = (stD3DContainerEx*)meshContainer;
		for(unsigned int i=0;i<containerEx->NumMaterials;i++)
		{
			m_device->SetMaterial(&containerEx->mat[i]);
			m_device->SetTexture(0,containerEx->textures[i]);

			LPD3DXMESH pDrawMesh;

			if(containerEx->pSkinInfo)
				pDrawMesh = containerEx->originMesh;
			else
				pDrawMesh = containerEx->MeshData.pMesh;

			pDrawMesh->DrawSubset(i);
		}
		meshContainer = meshContainer->pNextMeshContainer;
	}

	if(frame->pFrameSibling != NULL)
		DrawFrame(frame->pFrameSibling);
	if(frame->pFrameFirstChild != NULL)
		DrawFrame(frame->pFrameFirstChild);
}




void CXModel::SetupMatrices(stD3DFrameEx* inFrame,LPD3DXMATRIX parentMatrix)
{
	if(!m_device) 
		return;

   stD3DContainerEx *containerEx = (stD3DContainerEx*)inFrame->pMeshContainer;

   if(containerEx)
      {
         if(!m_currentContainer)
            m_currentContainer = containerEx;

         if(containerEx->pSkinInfo && containerEx->MeshData.pMesh)
            {
               D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];

               if(FAILED(containerEx->MeshData.pMesh->GetDeclaration(Declaration)))
                  return;

               containerEx->MeshData.pMesh->CloneMesh(D3DXMESH_MANAGED,
                  Declaration, m_device, &containerEx->originMesh);

               m_maxBones = max(m_maxBones, containerEx->pSkinInfo->GetNumBones());

               for(unsigned int i = 0; i < containerEx->pSkinInfo->GetNumBones(); i++)
                  {   
                     stD3DFrameEx *temp = (stD3DFrameEx*)D3DXFrameFind(m_root,
                        containerEx->pSkinInfo->GetBoneName(i));

                     containerEx->boneMatrices[i] = &temp->finalMatrix;
                  }

            }
      }

   if(inFrame->pFrameSibling)
      SetupMatrices((stD3DFrameEx*)inFrame->pFrameSibling, parentMatrix);

   if(inFrame->pFrameFirstChild)
      SetupMatrices((stD3DFrameEx*)inFrame->pFrameFirstChild, &inFrame->finalMatrix);
}


void CXModel::UpdateMatrices(const D3DXFRAME* inFrame,const D3DXMATRIX* parentMatrix)
{
	stD3DFrameEx *frame = (stD3DFrameEx*)inFrame;

   if(parentMatrix != NULL)
	   D3DXMatrixMultiply(&frame->finalMatrix, &frame->TransformationMatrix, parentMatrix);
   else
      frame->finalMatrix = frame->TransformationMatrix;

   if(frame->pFrameSibling != NULL)
      UpdateMatrices(frame->pFrameSibling, parentMatrix);

   if(frame->pFrameFirstChild != NULL)
      UpdateMatrices(frame->pFrameFirstChild, &frame->finalMatrix);
}



void CXModel::SetAnimation(unsigned int index)
{
	if(index >= m_numAnims || index == m_currentAnim)
      return;

   m_currentAnim = index;

   LPD3DXANIMATIONSET set;
   //得到当前动画
   m_animControl->GetAnimationSet(m_currentAnim, &set);



   unsigned long nextTrack = (m_currentTrack == 0 ? 1 : 0);

   // Set next track.
   m_animControl->SetTrackAnimationSet(nextTrack, set);
   set->Release();	

   // Take way all tracks.
   m_animControl->UnkeyAllTrackEvents(m_currentTrack);
   m_animControl->UnkeyAllTrackEvents(nextTrack);

   // Key current track.//从当前动画转到下一个动画的属性？
   //设置动画主要路径
   m_animControl->KeyTrackEnable(m_currentTrack, FALSE, m_currentTime + m_transition);
   //路径事件的速度
   m_animControl->KeyTrackSpeed(m_currentTrack, 0.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);
   //动画路径权重
   m_animControl->KeyTrackWeight(m_currentTrack, 0.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);

   // Key next track.
   m_animControl->SetTrackEnable(nextTrack, TRUE);
   m_animControl->KeyTrackSpeed(nextTrack, 1.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);
   m_animControl->KeyTrackWeight(nextTrack, 1.0f, m_currentTime, m_transition, D3DXTRANSITION_LINEAR);

   m_currentTrack = nextTrack;
}



void CXModel::NextAnimation()
{
	  unsigned int newAnimationSet = m_currentAnim + 1;

   if(newAnimationSet >= m_numAnims)
      newAnimationSet = 0;

   SetAnimation(newAnimationSet);
}