//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <windows.h> 
#include <mil.h>
#include <queue>
#include <list>

#include "d3d9.h"
#include "d3dx9.h"
#include "md3ddisplayeffect.h"


struct CUSTOMVERTEX VertexDefinitionOriginal[] = 
   {{-1.0f,  1.0f ,  0.0f,  0.0f, 0.0f},
   { -1.0f, -1.0f ,  0.0f,  0.0f, 1.0f},
   {  1.0f,  1.0f ,  0.0f,  1.0f, 0.0f},
   {  1.0f, -1.0f ,  0.0f,  1.0f, 1.0f}};


void  VertexResize(CUSTOMVERTEX *pVertexDefinition, float XFactor, float YFactor);
void  VertexTranslate(CUSTOMVERTEX *pVertexDefinition, float X, float Y);
void  AdjustViewMatrix(D3D_EFFECT *pD3DEffect, IDirect3DDevice9* d3ddev );


HRESULT DX9Processing(D3D_EFFECT *pD3DEffect, LPDIRECT3DDEVICE9  pD3DDevice, LPDIRECT3DSURFACE9 pSrcSurface, LPDIRECT3DSURFACE9 pDestSurface)
   {
   HRESULT hr;
   LPDIRECT3DTEXTURE9 SourceTexture1  = NULL;
   LPDIRECT3DTEXTURE9 SourceTexture2  = NULL;
   LPDIRECT3DTEXTURE9 MainTexture          = NULL;
   LPDIRECT3DTEXTURE9 VideoOverlayTexture  = NULL;

   LPDIRECT3DTEXTURE9 OverlayTexture1 = NULL;
   LPDIRECT3DSURFACE9 DestSurface     = NULL;

   MIL_INT SizeX = pD3DEffect->SizeX;
   MIL_INT SizeY = pD3DEffect->SizeY;

   if(!pD3DEffect->IsAllocated)
      pD3DEffect->Alloc(pD3DDevice);

   hr = pD3DDevice->StretchRect(pSrcSurface, NULL, pD3DEffect->pSurface, NULL, D3DTEXF_LINEAR);

   MainTexture = pD3DEffect->pTexture;

   pD3DEffect->ProcessFrameCount++;

   FAIL_RET(pD3DDevice->SetFVF(CUSTOMFVF));

   AdjustViewMatrix( pD3DEffect, pD3DDevice ) ;

   if(pD3DEffect->ProcessFrameCount < 2)
      {
      memcpy(&pD3DEffect->VertexDefinition[0], VertexDefinitionOriginal, sizeof(CUSTOMVERTEX) * 4);

      FAIL_RET(pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE ) ) ;
      FAIL_RET(pD3DDevice->SetRenderState( D3DRS_CULLMODE,D3DCULL_NONE)); 
      FAIL_RET(pD3DDevice->SetRenderState( D3DRS_ZENABLE,TRUE)); 

      FAIL_RET(pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE));
      FAIL_RET(pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE));

      FAIL_RET(pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER));
      FAIL_RET(pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER));   
      }

   float TranslationX = 0;
   float TranslationY = 0;

   VertexResize   (&pD3DEffect->VertexDefinition[0], 1.0, 1.0);
   VertexTranslate(&pD3DEffect->VertexDefinition[0], 0.0, 0.0);

   void* pVoid = 0; 
   pD3DEffect->pD3DVertexBuffer->Lock(0, 0, (void**)&pVoid, 0);
   memcpy(pVoid, pD3DEffect->VertexDefinition, sizeof(pD3DEffect->VertexDefinition));
   pD3DEffect->pD3DVertexBuffer->Unlock();

   while(pD3DDevice->BeginScene() != 0);

   FAIL_RET(pD3DDevice->SetStreamSource(0, pD3DEffect->pD3DVertexBuffer , 0, sizeof(CUSTOMVERTEX)));
   FAIL_RET(pD3DDevice->SetRenderTarget(0, pDestSurface));
   hr =  pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(127,127,127), 1.0f, 0 );

   FAIL_RET(pD3DDevice->SetTexture(0, MainTexture));
   FAIL_RET(pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));

   FAIL_RET(pD3DDevice->EndScene());

   return 0;
   }


void  AdjustViewMatrix(D3D_EFFECT *pD3DEffect, IDirect3DDevice9* d3ddev )
   {
   float fRotX = 0.0f;
   float fRotY = 0.0f;

   D3DXMATRIX mPos;
   D3DXMATRIX mView;
   D3DXMATRIX mProj;

   if(pD3DEffect->AutomaticMoving)
      {
      if((pD3DEffect->ProcessFrameCount % 600) == 0)
         {
         D3DXMatrixTranslation( &mPos, 0,0,0 );
         D3DXMatrixTranslation( &mView, 0,0,0 );
         D3DXMatrixTranslation( &mProj, 0,0,0 );
         pD3DEffect->DisplayCurX = rand() % (pD3DEffect->SizeX/2);
         pD3DEffect->DisplayCurY = rand() % (pD3DEffect->SizeY/2);
         }
      else
         {
         pD3DEffect->DisplayCurX--;
         pD3DEffect->DisplayCurY++;
         }
      }

   if(0)
      {
      D3DXMatrixTranslation( &mPos, 0,0,0 );
      D3DXMatrixTranslation( &mView, 0,0,0 );
      D3DXMatrixTranslation( &mProj, 0,0,0 );
      }
   else
      {
      fRotX = pD3DEffect->DisplayCurX/255.0f*60.0f*(D3DX_PI/180.0f);
      fRotY = pD3DEffect->DisplayCurY/255.0f*60.0f*(D3DX_PI/180.0f);

      D3DXMATRIX mWorld;
      D3DXMATRIX mWorldX;
      D3DXMatrixRotationY( &mWorld, 0);
      D3DXMatrixRotationX( &mWorldX,0);

      D3DXVECTOR3 eye( fRotX, fRotY, -1.0);
      D3DXVECTOR3 at( 0, 0, 0 );
      D3DXVECTOR3 up( 0, 1, 0 );
      D3DXMatrixLookAtLH( &mView, &eye, &at, &up );
      D3DXMatrixPerspectiveFovLH( &mProj, D3DX_PI/4.0f, 1.0f, 1.0f, 100.0f );

      D3DXMatrixTranslation( &mPos, 0,0,0 );

      mPos = mWorld*mWorldX*mPos;
      }

   d3ddev->SetTransform( D3DTS_VIEW, &mView );
   d3ddev->SetTransform( D3DTS_PROJECTION, &mProj );
   d3ddev->SetTransform( D3DTS_WORLD, &mPos );

   return ;
   }


void  VertexResize(CUSTOMVERTEX *pVertexDefinition, float XFactor, float YFactor)
   {
   pVertexDefinition[1].Y =  1.0f - (2.0f * YFactor);
   pVertexDefinition[2].X = -1.0f + (2.0f * XFactor);
   pVertexDefinition[3].Y =  1.0f - (2.0f * YFactor);
   pVertexDefinition[3].X = -1.0f + (2.0f * XFactor);
   }

void  VertexTranslate(CUSTOMVERTEX *pVertexDefinition, float X, float Y)
   {
   pVertexDefinition[0].X += X;
   pVertexDefinition[0].Y += Y;
   pVertexDefinition[1].X += X;
   pVertexDefinition[1].Y += Y;
   pVertexDefinition[2].X += X;
   pVertexDefinition[2].Y += Y;
   pVertexDefinition[3].X += X;
   pVertexDefinition[3].Y += Y;

   pVertexDefinition[0].X = min(pVertexDefinition[0].X, 1.0f);
   pVertexDefinition[1].X = min(pVertexDefinition[1].X, 1.0f);
   pVertexDefinition[2].X = min(pVertexDefinition[2].X, 1.0f);
   pVertexDefinition[3].X = min(pVertexDefinition[3].X, 1.0f);

   }
