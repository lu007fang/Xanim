/*
   Demo Name:  Template
      Author:  Allen Sherrod
     Chapter:  Ch 1
*/

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "CXModel.h"
#include <MMSystem.h>
#include "commandScript.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define SAFE_RELEASE(P) {if(P){(P)->Release(); (P)=NULL;}}
#define WINDOW_CLASS    "UGPDX"
#define WINDOW_NAME     "Template"
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600

#define MAX_COMMAND_SIZE  32
#define MAX_PARAM_SIZE 20

struct stD3DVertex
{
	float x,y,z,rhw;
	unsigned long color;
	float u,v;
	
};

struct stGameWorld
{
	stGameWorld(): m_skyBoxID(-1),m_terrainID(-1){}

	int m_skyBoxID;
	int m_terrainID;
	
};

#define D3DFVF_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// Function Prototypes...
bool InitializeD3D(HWND hWnd, bool fullscreen);
bool InitializeObjects();
void RenderScene();

bool LoadScript(char* file);
void RenderXModel();

void Shutdown();


// Direct3D object and device.
LPDIRECT3D9 g_D3D = NULL;
LPDIRECT3DDEVICE9 g_D3DDevice = NULL;
CCommandScript g_script;




// Matrices.
D3DXMATRIX g_projection;
D3DXMATRIX g_ViewMatrix;
D3DXMATRIX g_WorldMatrix;




D3DLIGHT9 g_light;
D3DMATERIAL9 g_material;

CXModel* g_XModel = NULL;
stGameWorld g_gameWorld;

float g_startTime =0;



LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
   switch(msg)
      {
         case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            break;

         case WM_KEYUP:
            if(wp == VK_ESCAPE) PostQuitMessage(0);
            break;
      }

   return DefWindowProc(hWnd, msg, wp, lp);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE ph, LPSTR cmd, int s)
{

	//////////////////窗口的设计//////////////////
   
   WNDCLASSEX wc = { sizeof(WNDCLASSEX),    //定义该结构图的字节数大小 UINT类型
					 CS_CLASSDC,            //UINT类型 定义窗口的样式 窗口画面会不会重新绘制， 是否能关闭， 是否发送鼠标消息
					 MsgProc,               //WNDPROC类型 是一个函数指针 指向窗口过程函数
					 0L,                    //int类型 表示窗口类的附加内存
					 0L,                    //int类型 表示窗口的附加内存
					 GetModuleHandle(NULL), //HINSTANCE类型 实例句柄
					 NULL,                  //窗口类的图标句柄 
					 NULL,                  //窗口类的鼠标句柄
					 NULL,                  //窗口类的背景笔刷句柄
					 NULL,                  //LPCTSTR类型 一个以空终止的字符串 指定菜单资源的名字
					 WINDOW_CLASS,          //LPCTSTR类型 一个以空终止的字符串 指定窗口类的名字
					 NULL };                //窗口类的图标句柄 在窗口右下角

   //////////////////窗口的注册//////////////////

   RegisterClassEx(&wc);  //注册窗口 能够得到窗口过程函数的地址

   //////////////////窗口的创建//////////////////

   HWND hWnd = CreateWindow(WINDOW_CLASS,                  //LPCTSTR类型 对应窗口类的名称
						    WINDOW_NAME,                   //LPCTSTR类型 指定窗口名字
						    WS_OVERLAPPEDWINDOW,           //DWORD类型 定义窗口的样式  OVERLAPPEDWINDOW是个综合类型
						    100,                           //int类型 指定窗口水平位置
						    100,                           //int类型 指定窗口竖直位置
						    WINDOW_WIDTH,                  //int类型 指定窗口宽度
						    WINDOW_HEIGHT,                 //int类型 指定窗口高度
						    GetDesktopWindow(),            //HWND类型 指定窗口的父窗口句柄
						    NULL,                          //HMENU类型 指定窗口菜单的资源句柄
						    wc.hInstance,                  //HINSTANCE类型 指定实例句柄 即WinMain的第一的参数
						    NULL);                         //LPVOID类型 lpPrama 作为WM_CREATE消息的 附加参数LParam 传入的 数据指针

  //////////////////窗口的显示与更新//////////////////

   if(InitializeD3D(hWnd, false)) //对Direct3D资源初始化
      {
         
         ShowWindow(hWnd, SW_SHOWDEFAULT); //显示 （最后的参数用于指定窗口的显示状态）
         UpdateWindow(hWnd);               //更新 通过发送WM_PAINT消息来更新 WM_PAINT发送给了窗口过程函数 而不是消息队列


		 //////////////////消息循环//////////////////
        
         MSG msg; //定义变量msg （MSG是个结构体）
         ZeroMemory(&msg, sizeof(msg));

         while(msg.message != WM_QUIT)  //通过while循环来保证程序处于运行状态
            {
               if(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                  {
                     TranslateMessage(&msg);  //把虚拟键消息转成字符消息并发给消息队列 （以便下一次调用peekmessage时取出）
                     DispatchMessage(&msg);   //把窗口消息传给操作系统 系统通过窗口过程函数处理消息
                  }
               else //有消息处理消息⬆ 没消息绘制画面⬇
                  RenderScene();
            }
      }

   // Release any and all resources.
   Shutdown();

   //////////////////窗口的注销//////////////////

   UnregisterClass(WINDOW_CLASS, wc.hInstance); //对设计好的窗口类进行注销

   return 0;
}


bool InitializeD3D(HWND hWnd, bool fullscreen)
{
   D3DDISPLAYMODE displayMode;

   //////////////////创建Direct3D接口对象//////////////////

   g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
   if(g_D3D == NULL) return false;


   ///////////////////获取后台缓冲区保存像素的格式//////////////////

   if(FAILED(g_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
      &displayMode))) return false;


  //////////////////填充D3DPRESENT_PARAMETERS结构体//////////////////

   D3DPRESENT_PARAMETERS d3dpp;
   ZeroMemory(&d3dpp, sizeof(d3dpp));

   if(fullscreen)
      {
         d3dpp.Windowed = FALSE;
         d3dpp.BackBufferWidth = WINDOW_WIDTH;
         d3dpp.BackBufferHeight = WINDOW_HEIGHT;
      }
   else
      d3dpp.Windowed = TRUE;
   d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
   d3dpp.BackBufferFormat = displayMode.Format;


  //////////////////创建设备接口//////////////////

   if(FAILED(g_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
      &d3dpp, &g_D3DDevice))) return false;


   // Initialize any objects we will be displaying.
   if(!InitializeObjects()) return false;

   return true;
}


bool InitializeObjects()
{
   // Set the projection matrix.
   D3DXMatrixPerspectiveFovLH(&g_projection, 45.0f,
      WINDOW_WIDTH/WINDOW_HEIGHT, 0.1f, 1000.0f);

   g_D3DDevice->SetTransform(D3DTS_PROJECTION, &g_projection);

  


   
	
   
   g_XModel = new CXModel(g_D3DDevice);

  // if(FAILED(g_XModel->LoadXfile("skybox.x")))
	//	return false;

   if(!LoadScript("sphere.lvl"))
	   return false;
   
   g_startTime= (float)timeGetTime();

   D3DXVECTOR3 center;
   float radius;

   g_XModel->GetBoundingSphere(&center,&radius);
 

  

    ///////创建灯光////////////

   g_light.Type = D3DLIGHT_DIRECTIONAL;
   g_light.Direction = D3DXVECTOR3(0.0,-1.0f,-1.0f);
   g_light.Diffuse.r = g_light.Diffuse.g = 1;
   g_light.Diffuse.b = g_light.Diffuse.a =1;

   g_light.Specular.r = g_light.Specular.g = 1;
   g_light.Specular.b = g_light.Specular.a = 1;


    ///////导进设备////////////
   g_D3DDevice->SetLight(0,&g_light); 
   g_D3DDevice->LightEnable(0,FALSE);



   	// Define camera information.
   D3DXVECTOR3 cameraPos(0.0f, center.y - (radius * 3), -100.0f);
   D3DXVECTOR3 lookAtPos = center - cameraPos;
   D3DXVec3Normalize(&lookAtPos,&lookAtPos);
   D3DXVECTOR3 upDir(0.0f, 1.0f, 0.0f);

   // Build view matrix.
   D3DXMatrixLookAtLH(&g_ViewMatrix, &cameraPos,
                      &lookAtPos, &upDir);


   return true;
}




void RenderScene()
{
   // Clear the backbuffer.
   g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET,
                      D3DCOLOR_XRGB(0,0,0), 1.0f, 0);

 

   // Begin the scene.  Start rendering.
   g_D3DDevice->BeginScene();

      // Apply the view (camera).
      g_D3DDevice->SetTransform(D3DTS_VIEW, &g_ViewMatrix);


	  D3DXMatrixIdentity(&g_WorldMatrix);

	  float time = (float)timeGetTime();
	  time = (time-g_startTime) * 0.001f;
	  g_startTime = (float)timeGetTime();

	  g_XModel->Update(time,&g_WorldMatrix);
	  g_XModel->Render();




   g_D3DDevice->EndScene();

   // Display the scene.
   g_D3DDevice->Present(NULL, NULL, NULL, NULL);

   
}

bool LoadScript(char* file)
{
	if(!file)
		return false;

	
	char command[MAX_COMMAND_SIZE];
	char param[MAX_PARAM_SIZE];

	if(!g_script.LoadScriptFile(file))
		return false;

	for(int i=0;i<g_script.GetTotalLines();i++)
	{
		g_script.ParseCommand(command);

		if(stricmp(command,"LoadSphereAsX") == 0){}
		else if(stricmp(command,"LoadTerrainAsX") == 0){}
		else if(stricmp(command,"LoadStaticModelAsX") == 0){}
		else if(stricmp(command,"LoadAnimtedModelAsX") == 0){}
		else if(stricmp(command,"#") == 0){}
		else
		{
			char err[64];
			sprintf(err,"error loading model on %d.",g_script.GetCurrentLineNum() + 1);

			g_script.Shutdown();

			return false;
		}
		g_script.MoveToNextLine();
	}
	
	g_script.MoveToStart();

	for(int i=0;i<g_script.GetTotalLines();i++)
	{
		g_script.ParseCommand(command);

		if(stricmp(command,"LoadSphereAsX") == 0)
		{
			g_script.ParseStringParse(param);

			if(FAILED(g_XModel->LoadXfile(param)))
				return false;
			
		}
		else if(stricmp(command,"LoadTerrainAsX") == 0){}
		else if(stricmp(command, "LoadStaticModelAsX") == 0) { }
        else if(stricmp(command, "LoadAnimatedModelAsX") == 0) { }
        else if(stricmp(command, "#") == 0) { }
		
		g_script.MoveToNextLine();

	}

	//script.Shutdown();

	return true;
}

void Shutdown()
{
	g_script.Shutdown();
   // Release all resources.
   if(g_D3DDevice != NULL) g_D3DDevice->Release();
   if(g_D3D != NULL) g_D3D->Release();

   g_D3DDevice = NULL;
   g_D3D = NULL;

   if(g_XModel)
      {
         g_XModel->shutdown();
         delete g_XModel;
         g_XModel = NULL;
      }

}
