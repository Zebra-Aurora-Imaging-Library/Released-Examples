//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved


struct CUSTOMVERTEX {FLOAT X, Y, Z; FLOAT U, V;};
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_TEX1)
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define FAIL_RET(x) do { if( FAILED( hr = ( x  ) ) ) \
   return hr; } while(0)



typedef struct _D3D_EFFECT
   {
   _D3D_EFFECT ()
      {
      Init();
      }
   void Init()
      {
      AutomaticMoving = true;
      ProcessFrameCount = 0;
      DisplayStartX = 0;
      DisplayStartY = 0;
      DisplayCurX= 0;
      DisplayCurY = 0;
      SizeX = 1920;
      SizeY = 1080;
      memset(VertexDefinition, 0 , sizeof(VertexDefinition));
      pD3DVertexBuffer = NULL;
      pTexture = NULL;
      pSurface = NULL;
      IsAllocated = false;
      }

   void Alloc(LPDIRECT3DDEVICE9 pD3DDevice)
      {
      if(!IsAllocated)
         {
         pD3DDevice->CreateVertexBuffer(sizeof(VertexDefinition), 0, CUSTOMFVF, D3DPOOL_DEFAULT, &pD3DVertexBuffer, NULL);
         D3DDISPLAYMODE dm; 
         pD3DDevice->GetDisplayMode(NULL,  & dm );
         pD3DDevice->CreateTexture((UINT)dm.Width, (UINT)dm.Height, 1, D3DUSAGE_RENDERTARGET, dm.Format, D3DPOOL_DEFAULT, &pTexture, NULL);
         pTexture->GetSurfaceLevel(0, &pSurface);
         }
      IsAllocated = true;
      }

   void Free()
      {
      if(IsAllocated)
         {
         if(pD3DVertexBuffer)
            pD3DVertexBuffer->Release();
         if(pTexture)
            pTexture->Release();
         if(pSurface)
            pSurface->Release();
         }
      IsAllocated = false;
      }

   bool       IsAllocated;
   bool       AutomaticMoving;
   MIL_INT    ProcessFrameCount;
   MIL_INT    SizeX;
   MIL_INT    SizeY;
   MIL_INT    DisplayStartX;
   MIL_INT    DisplayStartY;

   MIL_INT    DisplayCurX;
   MIL_INT    DisplayCurY;
   struct CUSTOMVERTEX VertexDefinition[4*4];
   LPDIRECT3DVERTEXBUFFER9 pD3DVertexBuffer;
   LPDIRECT3DTEXTURE9      pTexture;
   LPDIRECT3DSURFACE9      pSurface;

   } D3D_EFFECT;



HRESULT DX9Processing(D3D_EFFECT *pD3DEffect, LPDIRECT3DDEVICE9  pD3DDevice, LPDIRECT3DSURFACE9 pSrcSurface, LPDIRECT3DSURFACE9 pDestSurface);
