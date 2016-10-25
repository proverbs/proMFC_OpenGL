﻿
// MFC_OpenGLView.cpp : CMFC_OpenGLView 类的实现
//

#include "stdafx.h"

#include <ctime>

// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFC_OpenGL.h"
#endif

#include "MFC_OpenGLDoc.h"
#include "MFC_OpenGLView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFC_OpenGLView

IMPLEMENT_DYNCREATE(CMFC_OpenGLView, CView)

BEGIN_MESSAGE_MAP(CMFC_OpenGLView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_DRAW_OPT, &CMFC_OpenGLView::OnDrawOpt)
END_MESSAGE_MAP()

// openGL error code
const char* const CMFC_OpenGLView::ErrorStrings[] = {
	{ "No Error" },                     // 0
	{ "Unable to get a DC" },           // 1
	{ "ChoosePixelFormat failed" },     // 2
	{ "SelectPixelFormat failed" },     // 3
	{ "wglCreateContext failed" },      // 4
	{ "wglMakeCurrent failed" },        // 5
	{ "wglDeleteContext failed" },      // 6
	{ "SwapBuffers failed" },           // 7
	{ "Shader conpilation failed" },	// 8
	{ "Shader linking failed" }			// 9	
};

// CMFC_OpenGLView 构造/析构

CMFC_OpenGLView::CMFC_OpenGLView()
{
	// TODO: 在此处添加构造代码
	// openGL by proverbs
	m_hRC = NULL;
	m_pDC = NULL;
	m_ErrorString = ErrorStrings[0];

	// control view
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

	memset(keys, 0, sizeof keys);
	lastX = 400, lastY = 300;
	firstMouse = true;

	firstMouse = true;
	isPress = false;

	viewModel = 0;
}

CMFC_OpenGLView::~CMFC_OpenGLView()
{
}

BOOL CMFC_OpenGLView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFC_OpenGLView 绘制

void CMFC_OpenGLView::OnDraw(CDC* /*pDC*/)
{
	CMFC_OpenGLDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	RenderScene();// openGL by proverbs

	/*
	//Swap buffers to show result
	if (FALSE == ::SwapBuffers(m_pDC->GetSafeHdc()))
	{
		SetError(7);
	}
	*/
}


// CMFC_OpenGLView 打印

BOOL CMFC_OpenGLView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFC_OpenGLView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMFC_OpenGLView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CMFC_OpenGLView 诊断

#ifdef _DEBUG
void CMFC_OpenGLView::AssertValid() const
{
	CView::AssertValid();
}

void CMFC_OpenGLView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFC_OpenGLDoc* CMFC_OpenGLView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFC_OpenGLDoc)));
	return (CMFC_OpenGLDoc*)m_pDocument;
}
#endif //_DEBUG


// openGL by proverbs
// GL Rendering Context Creation Functions
//
// Since we are using Windows native windowing, we need to set up our own
// OpenGL rendering context. These functions do it to the main view area.
// It is possible to do it to a smaller sub view. If you are curious, you can
// find tutorials on how to do that on the net.
//
void CMFC_OpenGLView::SetError(int e)
{
	// if there was no previous error,
	// then save this one
	if (ErrorStrings[0] == m_ErrorString)
	{
		m_ErrorString = ErrorStrings[e];
	}
}

BOOL CMFC_OpenGLView::GetRenderingContext()
{
	// Can we put this in the constructor?
	m_pDC = new CClientDC(this);

	if (NULL == m_pDC) // failure to get DC
	{
		SetError(1);
		return FALSE;
	}

	if (!GetOldStyleRenderingContext())
	{
		return TRUE;
	}

	//Get access to modern OpenGL functionality from this old style context.
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		AfxMessageBox(_T("GLEW could not be initialized!"));
		return FALSE;
	}

	//Get a new style pixel format
	if (!SetupPixelFormat())
	{
		return FALSE;
	}

	//Setup request for OpenGL 3.2 Core Profile
	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,   3,
		WGL_CONTEXT_MINOR_VERSION_ARB,   3,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0, 0  //End
	};

	if (wglewIsSupported("WGL_ARB_create_context") == 1)
	{
		//If this driver supports new style rendering contexts, create one
		HGLRC oldContext = m_hRC;
		if (0 == (m_hRC = m_hRC = wglCreateContextAttribsARB(m_pDC->GetSafeHdc(), 0, attribs)))
		{
			SetError(4);
			return FALSE;
		}

		if (!wglMakeCurrent(NULL, NULL))
			wglDeleteContext(oldContext);
		if (FALSE == wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC))
		{
			SetError(5);
			return FALSE;
		}
	}
	else
	{
		//Otherwise use the old style rendering context we created earlier.
		AfxMessageBox(_T("GL 3.2 Context not possible. Using old style context. (GL 2.1 and before)"));
	}

	return TRUE;
}

BOOL CMFC_OpenGLView::InitializeOpenGL()
{
	PIXELFORMATDESCRIPTOR pfd;
	int n;
	m_pDC = new CClientDC(this);
	ASSERT(m_pDC != NULL);
	// 设置当前的绘图像素格式  
	if (!SetupPixelFormat())
	{
		return FALSE;
	}

	n = ::GetPixelFormat(m_pDC->GetSafeHdc());
	::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);
	// 创建绘图描述表  
	m_hRC = wglCreateContext(m_pDC->GetSafeHdc());
	if (m_hRC == NULL)
	{
		return FALSE;
	}
	// 使绘图描述表为当前调用现程的当前绘图描述表  
	if (wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CMFC_OpenGLView::GetOldStyleRenderingContext()
{
	//A generic pixel format descriptor. This will be replaced with a more
	//specific and modern one later, so don't worry about it too much.
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |            // support window
		PFD_SUPPORT_OPENGL |            // support OpenGL
		PFD_DOUBLEBUFFER,               // double buffered
		PFD_TYPE_RGBA,                  // RGBA type
		32,                             // 32-bit color depth
		0, 0, 0, 0, 0, 0,               // color bits ignored
		0,                              // no alpha buffer
		0,                              // shift bit ignored
		0,                              // no accumulation buffer
		0, 0, 0, 0,                     // accum bits ignored
		24,                        // 24-bit z-buffer
		0,                              // no stencil buffer
		0,                              // no auxiliary buffer
		PFD_MAIN_PLANE,                 // main layer
		0,                              // reserved
		0, 0, 0                         // layer masks ignored
	};

	// Get the id number for the best match supported by the hardware device context
	// to what is described in pfd
	int pixelFormat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd);

	//If there's no match, report an error
	if (0 == pixelFormat)
	{
		SetError(2);
		return FALSE;
	}

	//If there is an acceptable match, set it as the current 
	if (FALSE == SetPixelFormat(m_pDC->GetSafeHdc(), pixelFormat, &pfd))
	{
		SetError(3);
		return FALSE;
	}

	//Create a context with this pixel format
	if (0 == (m_hRC = wglCreateContext(m_pDC->GetSafeHdc())))
	{
		SetError(4);
		return FALSE;
	}

	//Make it current. Now we're ready to get extended features.
	if (FALSE == wglMakeCurrent(m_pDC->GetSafeHdc(), m_hRC))
	{
		SetError(5);
		return FALSE;
	}
	return TRUE;
}


BOOL CMFC_OpenGLView::SetupPixelFormat()
{
	/*
	//This is a modern pixel format attribute list.
	//It has an extensible structure. Just add in more argument pairs 
	//befroe the null to request more features.
	const int attribList[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
		WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
		WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,     32,
		WGL_DEPTH_BITS_ARB,     24,
		WGL_STENCIL_BITS_ARB,   8,
		0, 0  //End
	};


	unsigned int numFormats;
	int pixelFormat;
	PIXELFORMATDESCRIPTOR pfd;

	//Select a pixel format number
	wglChoosePixelFormatARB(m_pDC->GetSafeHdc(), attribList, NULL, 1, &pixelFormat, &numFormats);

	//Optional: Get the pixel format's description. We must provide a 
	//description to SetPixelFormat(), but its contents mean little.
	//According to MSDN: 
	//  The system's metafile component uses this structure to record the logical
	//  pixel format specification. The structure has no other effect upon the
	//  behavior of the SetPixelFormat function.
	//DescribePixelFormat(m_pDC->GetSafeHdc(), pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	//Set it as the current 
	if (FALSE == SetPixelFormat(m_pDC->GetSafeHdc(), pixelFormat, &pfd))
	{
		SetError(3);
		return FALSE;
	}

	return TRUE;
	*/


	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  // pfd结构的大小   
		1,                              // 版本号   
		PFD_DRAW_TO_WINDOW |            // 支持在窗口中绘图   
		PFD_SUPPORT_OPENGL |            // 支持 OpenGL   
		PFD_DOUBLEBUFFER,               // 双缓存模式   
		PFD_TYPE_RGBA,                  // RGBA 颜色模式   
		24,                             // 24 位颜色深度   
		0, 0, 0, 0, 0, 0,               // 忽略颜色位   
		0,                              // 没有非透明度缓存   
		0,                              // 忽略移位位   
		0,                              // 无累计缓存   
		0, 0, 0, 0,                     // 忽略累计位   
		32,                             // 32 位深度缓存       
		0,                              // 无模板缓存   
		0,                              // 无辅助缓存   
		PFD_MAIN_PLANE,                 // 主层   
		0,                              // 保留   
		0, 0, 0                         // 忽略层,可见性和损毁掩模   

	};
	int pixelFormat;
	// 为设备描述表得到最匹配的像素格式   
	if ((pixelFormat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0)
	{
		MessageBox(_T("ChoosePixelFormat failed"));
		return FALSE;
	}
	// 设置最匹配的像素格式为当前的像素格式   
	if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelFormat, &pfd) == FALSE)
	{
		MessageBox(_T("SetPixelFormat failed"));
		return FALSE;
	}
	return TRUE;
}


// openGL by proverbs
// 主要绘图程序
void CMFC_OpenGLView::RenderScene() {
	if (viewModel == 0) return;
	// draw一般写成循环形式

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// Clear the colorbuffer
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ourShader->Use();  
	// Transformation matrices
	glm::mat4 projection = glm::perspective(camera->Zoom, (float)m_wide / (float)m_heigt, 0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

	// Draw the loaded model
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
	glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	ourModel->Draw(*ourShader);


	// 显示图形，与MFC相关
	//Swap buffers to show result
	if (FALSE == ::SwapBuffers(m_pDC->GetSafeHdc()))
	{
		SetError(7);
	}

}

// CMFC_OpenGLView 消息处理程序

//BOOL CMFC_OpenGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//
//	return CView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
//}


int CMFC_OpenGLView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	// openGL by proverbs
	if (!GetRenderingContext())
	{
		//Something went wrong with getting the rendering context.
		//Create a popup with the error message, then quit.
		AfxMessageBox(CString(m_ErrorString));
		return -1;
	}

	if (InitializeOpenGL())
	{
		return 0;
	}

	return 0;
}


void CMFC_OpenGLView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	// TODO: 在此处添加消息处理程序代码   
	m_wide = cx;    //m_wide为在CVCOpenGL2View类中添加的表示视口宽度的成员变量   
	m_heigt = cy;  //m_height为在CVCOpenGL2View类中添加的表示视口高度的成员变量   
					//避免除数为0   
	if (m_heigt == 0)
	{
		m_heigt = 1;
	}
	//设置视口与窗口的大小   
	glViewport(0, 0, m_wide, m_heigt);
	// 为了显示三维物体，开启深度
	glEnable(GL_DEPTH_TEST);
	// FreeImage初始化
	FreeImage_Initialise(FALSE);// STATIC
}


void CMFC_OpenGLView::OnDestroy()
{
	CView::OnDestroy();
	// FreeImage终结
	FreeImage_DeInitialise();
	// TODO: 在此处添加消息处理程序代码
	// openGL by proverbs
	// 避免内存泄漏
	m_hRC = ::wglGetCurrentContext();
	if (::wglMakeCurrent(0, 0) == FALSE)
	{
		MessageBox(_T("Could not make RC non-current"));
	}

	if (m_hRC)
	{
		if (::wglDeleteContext(m_hRC) == FALSE)
		{
			MessageBox(_T("Could not delete RC"));
		}
	}

	if (m_pDC)
	{
		delete m_pDC;
	}
	m_pDC = NULL;
}


BOOL CMFC_OpenGLView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 避免闪屏
	return TRUE;
	// return CView::OnEraseBkgnd(pDC);
}


void CMFC_OpenGLView::do_movement() {
	// Camera controls
	if (keys['W']) 
		camera->ProcessKeyboard(FORWARD, 1.0);
	if (keys['S'])
		camera->ProcessKeyboard(BACKWARD, 1.0);
	if (keys['A'])
		camera->ProcessKeyboard(LEFT, 1.0);
	if (keys['D'])
		camera->ProcessKeyboard(RIGHT, 1.0);
	RedrawWindow();
}


// 移动即可调整视角
void CMFC_OpenGLView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//SetCursor(LoadCursor(NULL, IDC_HAND));// 设置成手形状
	// 只有鼠标左键按下才运行
	if (isPress) {
		if (firstMouse)
		{
			lastX = point.x;
			lastY = point.y;
			firstMouse = false;
		}
		GLint xoffset = point.x - lastX;
		GLint yoffset = lastY - point.y;  // Reversed since y-coordinates go from bottom to left
		lastX = point.x;
		lastY = point.y;

		camera->ProcessMouseMovement(xoffset, yoffset);
		do_movement();// 调整camera，重新绘图
	}

	CView::OnMouseMove(nFlags, point);
}


// 按下左键，然后移动（涉及move函数）可以调整视角
void CMFC_OpenGLView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	isPress = true;

	CView::OnLButtonDown(nFlags, point);
}


// 取消左键按下
void CMFC_OpenGLView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	isPress = false;
	firstMouse = true;

	CView::OnLButtonUp(nFlags, point);
}


void CMFC_OpenGLView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nChar) { // 判断用户按键
		case 'W': keys['W'] = true; break; // ↑：上移一个步长
		case 'S': keys['S'] = true; break; // ↓：下移一个步长
		case 'A': keys['A'] = true; break; // ←：左移一个步长
		case 'D': keys['D'] = true; break; // →：右移一个步长
	}
	do_movement();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMFC_OpenGLView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//SetCursor(LoadCursor(NULL, IDC_ARROW));// 设置成标准形状
	
	switch (nChar) { // 判断用户按键
		case 'W': keys['W'] = false; break; // ↑：上移一个步长
		case 'S': keys['S'] = false; break; // ↓：下移一个步长
		case 'A': keys['A'] = false; break; // ←：左移一个步长
		case 'D': keys['D'] = false; break; // →：右移一个步长
	}
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CMFC_OpenGLView::OnDrawOpt()
{
	// TODO: 在此添加命令处理程序代码
	CDrawDialog drawDialg;
	if (drawDialg.DoModal() == IDOK) {
		viewModel = drawDialg.MyModel;
		MessageBox(L"successful");
		// Setup and compile our shaders
		ourShader = new Shader("shaders/shader.vs", "shaders/shader.frag");

		// Load models
		ourModel = new Model("res/nanosuit/nanosuit.obj");
		
		// 重新绘图
		RedrawWindow();
	}
}
