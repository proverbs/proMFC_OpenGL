
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
	ON_COMMAND(ID_DRAW_LOAD, &CMFC_OpenGLView::OnDrawLoad)
	ON_COMMAND(ID_ROTATE_X, &CMFC_OpenGLView::OnRotateX)
	ON_UPDATE_COMMAND_UI(ID_ROTATE_X, &CMFC_OpenGLView::OnUpdateRotateX)
	ON_COMMAND(ID_ROTATE_Y, &CMFC_OpenGLView::OnRotateY)
	ON_UPDATE_COMMAND_UI(ID_ROTATE_Y, &CMFC_OpenGLView::OnUpdateRotateY)
	ON_COMMAND(ID_ROTATE_Z, &CMFC_OpenGLView::OnRotateZ)
	ON_UPDATE_COMMAND_UI(ID_ROTATE_Z, &CMFC_OpenGLView::OnUpdateRotateZ)
	ON_COMMAND(ID_MOVE_X, &CMFC_OpenGLView::OnMoveX)
	ON_UPDATE_COMMAND_UI(ID_MOVE_X, &CMFC_OpenGLView::OnUpdateMoveX)
	ON_COMMAND(ID_MOVE_Y, &CMFC_OpenGLView::OnMoveY)
	ON_UPDATE_COMMAND_UI(ID_MOVE_Y, &CMFC_OpenGLView::OnUpdateMoveY)
	ON_COMMAND(ID_MOVE_Z, &CMFC_OpenGLView::OnMoveZ)
	ON_UPDATE_COMMAND_UI(ID_MOVE_Z, &CMFC_OpenGLView::OnUpdateMoveZ)
	ON_COMMAND(ID_SCALE, &CMFC_OpenGLView::OnScale)
	ON_UPDATE_COMMAND_UI(ID_SCALE, &CMFC_OpenGLView::OnUpdateScale)
	ON_COMMAND(ID_NONE, &CMFC_OpenGLView::OnNone)
	ON_UPDATE_COMMAND_UI(ID_NONE, &CMFC_OpenGLView::OnUpdateNone)
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
	trans_type = NONE;

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

	viewModel = NONE_M;
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
	if (viewModel == NONE_M) return;
	// draw一般写成循环形式
	if (ourModel != NULL) {// 加载模型
		// Clear the colorbuffer
		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 3d绘图处理覆盖关系

		ourShader->Use();  
		// Transformation matrices
		glm::mat4 projection = glm::perspective(camera->Zoom, (float)m_wide / (float)m_height, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Draw the loaded model
		// glm::mat4 model;
		//model = glm::rotate(model, 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));// rotate
		//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(ourShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(*model));
		ourModel->Draw(*ourShader);
	}
	else {// 本地绘制图形
		if (viewModel == XK_M || viewModel == M_M) {

			// 设置光源位置
			glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

			// Clear the colorbuffer
	        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	        // Use cooresponding shader when setting uniforms/drawing objects
	        lightingShader->Use();
	        GLint objectColorLoc = glGetUniformLocation(lightingShader->Program, "objectColor");
	        GLint lightColorLoc  = glGetUniformLocation(lightingShader->Program, "lightColor");
	        GLint lightPosLoc    = glGetUniformLocation(lightingShader->Program, "lightPos");
	        GLint viewPosLoc     = glGetUniformLocation(lightingShader->Program, "viewPos");
	        glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
	        glUniform3f(lightColorLoc,  1.0f, 1.0f, 1.0f);
	        glUniform3f(lightPosLoc,    lightPos.x, lightPos.y, lightPos.z);
	        glUniform3f(viewPosLoc,     camera->Position.x, camera->Position.y, camera->Position.z);



	        // Create camera transformations
	        glm::mat4 view;
	        view = camera->GetViewMatrix();
	        glm::mat4 projection = glm::perspective(camera->Zoom, (GLfloat)m_wide / (GLfloat)m_height, 0.1f, 100.0f);
	        // Get the uniform locations
	        GLint modelLoc = glGetUniformLocation(lightingShader->Program, "model");
	        GLint viewLoc  = glGetUniformLocation(lightingShader->Program,  "view");
	        GLint projLoc  = glGetUniformLocation(lightingShader->Program,  "projection");
	        // Pass the matrices to the shader
	        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	        // Draw the container (using container's vertex attributes)
	        glBindVertexArray(containerVAO);
	        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
	        glDrawArrays(GL_TRIANGLES, 0, 36);
	        glBindVertexArray(0);

	        // Also draw the lamp object, again binding the appropriate shader
	        lampShader->Use();
	        // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
	        modelLoc = glGetUniformLocation(lampShader->Program, "model");
	        viewLoc  = glGetUniformLocation(lampShader->Program, "view");
	        projLoc  = glGetUniformLocation(lampShader->Program, "projection");
	        // Set matrices
	        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	        glm::mat4 s_model;
			s_model = glm::translate(s_model, lightPos);
			s_model = glm::scale(s_model, glm::vec3(0.001f)); // Make it a smaller cube
	        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(s_model));
	        // Draw the light object (using light's vertex attributes)
	        glBindVertexArray(lightVAO);
	        glDrawArrays(GL_TRIANGLES, 0, 36);
	        glBindVertexArray(0);
		}
		else if (viewModel == WL_M) {
			// Light attributes
			glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
			// Clear the colorbuffer
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use cooresponding shader when setting uniforms/drawing objects
			lightingShader->Use();
			GLint lightPosLoc = glGetUniformLocation(lightingShader->Program, "light.position");
			GLint viewPosLoc = glGetUniformLocation(lightingShader->Program, "viewPos");
			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(viewPosLoc, camera->Position.x, camera->Position.y, camera->Position.z);
			// Set lights properties
			glUniform3f(glGetUniformLocation(lightingShader->Program, "light.ambient"), 0.2f, 0.2f, 0.2f);
			glUniform3f(glGetUniformLocation(lightingShader->Program, "light.diffuse"), 0.5f, 0.5f, 0.5f);
			glUniform3f(glGetUniformLocation(lightingShader->Program, "light.specular"), 1.0f, 1.0f, 1.0f);
			// Set material properties
			glUniform1f(glGetUniformLocation(lightingShader->Program, "material.shininess"), 32.0f);

			// Create camera transformations
			glm::mat4 view;
			view = camera->GetViewMatrix();
			glm::mat4 projection = glm::perspective(camera->Zoom, (GLfloat)m_wide / (GLfloat)m_height, 0.1f, 100.0f);
			// Get the uniform locations
			GLint modelLoc = glGetUniformLocation(lightingShader->Program, "model");
			GLint viewLoc = glGetUniformLocation(lightingShader->Program, "view");
			GLint projLoc = glGetUniformLocation(lightingShader->Program, "projection");
			// Pass the matrices to the shader
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

			// Bind diffuse map
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseMap);
			// Bind specular map
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularMap);

			// Draw the container (using container's vertex attributes)
			glBindVertexArray(containerVAO);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(*model));
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			// Also draw the lamp object, again binding the appropriate shader
			lampShader->Use();
			// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
			modelLoc = glGetUniformLocation(lampShader->Program, "model");
			viewLoc = glGetUniformLocation(lampShader->Program, "view");
			projLoc = glGetUniformLocation(lampShader->Program, "projection");
			// Set matrices
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glm::mat4 s_model;
			s_model = glm::translate(s_model, lightPos);
			s_model = glm::scale(s_model, glm::vec3(0.001f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(s_model));
			// Draw the light object (using light's vertex attributes)
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
		else if (viewModel == SJ_M) {
			// Clear the colorbuffer
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			
			// 计算变化后的面心到camera的距离
			glm::vec4 a;
			glm::mat4 b;
			std::map<GLfloat, int> sorted;
			for (GLuint i = 0; i < windows.size(); i++)
			{
				a = glm::vec4(windows[i], 1.0f);
				a = (*model) * a;
				GLfloat distance = glm::length(glm::vec4(camera->Position, 1.0f) - a);
				sorted[distance] = i;
			}

			// Draw objects
			shader->Use();
			glm::mat4 view = camera->GetViewMatrix();
			glm::mat4 projection = glm::perspective(camera->Zoom, (float)m_wide / (float)m_height, 0.1f, 100.0f);
			glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			// Cubes
			glBindVertexArray(cubeVAO);
			glBindTexture(GL_TEXTURE_2D, cubeTexture);
			glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(*model));
			//glDrawArrays(GL_TRIANGLES, 0, 36);
			for (std::map<float, int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
			{
				glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(*model));
				glDrawArrays(GL_TRIANGLES, it->second * 6, 6);
			}
			glBindVertexArray(0);
		}
		
	}
	

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
	m_height = cy;  //m_height为在CVCOpenGL2View类中添加的表示视口高度的成员变量   
					//避免除数为0   
	if (m_height == 0)
	{
		m_height = 1;
	}
	//设置视口与窗口的大小   
	glViewport(0, 0, m_wide, m_height);
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
	SetCapture(); // 设置鼠标捕获

	CView::OnLButtonDown(nFlags, point);
}


// 取消左键按下
void CMFC_OpenGLView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	isPress = false;
	firstMouse = true;
	ReleaseCapture();

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
	// 键盘控制变换方式
	if (nChar == VK_LEFT) {
		if (trans_type == ROTATE_X) {
			*model = glm::rotate(*model, -0.5f, glm::vec3(1.0f, 0.0f, 0.0f));// 步长0.5
		}
		else if (trans_type == ROTATE_Y) {
			*model = glm::rotate(*model, -0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (trans_type == ROTATE_Z) {
			*model = glm::rotate(*model, -0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (trans_type == MOVE_X) {
			*model = glm::translate(*model, glm::vec3(-0.1f, 0.0f, 0.0f));// 步长0.1f
		}
		else if (trans_type == MOVE_Y) {
			*model = glm::translate(*model, glm::vec3(0.0f, -0.1f, 0.0f));
		}
		else if (trans_type == MOVE_Z) {
			*model = glm::translate(*model, glm::vec3(0.0f, 0.0f, -0.1f));
		}
		else if (trans_type == SCALE) {
			*model = glm::scale(*model, glm::vec3(0.9f, 0.9f, 0.9f));// 步长0.9倍
		}
	}
	else if (nChar == VK_RIGHT) {
		if (trans_type == ROTATE_X) {
			*model = glm::rotate(*model, 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));// 步长5
		}
		else if (trans_type == ROTATE_Y) {
			*model = glm::rotate(*model, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (trans_type == ROTATE_Z) {
			*model = glm::rotate(*model, 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (trans_type == MOVE_X) {
			*model = glm::translate(*model, glm::vec3(0.1f, 0.0f, 0.0f));// 步长0.1f
		}
		else if (trans_type == MOVE_Y) {
			*model = glm::translate(*model, glm::vec3(0.0f, 0.1f, 0.0f));
		}
		else if (trans_type == MOVE_Z) {
			*model = glm::translate(*model, glm::vec3(0.0f, 0.0f, 0.1f));
		}
		else if (trans_type == SCALE) {
			*model = glm::scale(*model, glm::vec3(1.1f, 1.1f, 1.1f));// 步长1.1倍
		}
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

// Set up vertex data (and buffer(s)) and attribute pointers
GLfloat vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

GLfloat vertices_1[] = {
	// Positions          // Normals           // Texture Coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};

GLfloat cubeVertices[] = {
	// Positions          // Texture Coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f
};

void CMFC_OpenGLView::OnDrawOpt()
{
	// TODO: 在此添加命令处理程序代码
	CDrawDialog drawDialg;
	if (drawDialg.DoModal() == IDOK) {
		if (drawDialg.MyModel == 1) viewModel = XK_M;
		else if (drawDialg.MyModel == 2) viewModel = M_M;
		else if (drawDialg.MyModel == 3) viewModel = SJ_M;
		else if (drawDialg.MyModel == 4) viewModel = WL_M;
		ourModel = NULL;// 切换绘图选项

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);// 默认模式
		glDisable(GL_BLEND);// 默认不开启混合

		if (viewModel == XK_M || viewModel == M_M) {
			if (viewModel == XK_M) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);// 线框模型 

			// Build and compile our shader program
			lightingShader = new Shader("shaders/basic_lighting.vs", "shaders/basic_lighting.frag");
			lampShader = new Shader("shaders/lamp.vs", "shaders/lamp.frag");
			// 新建转换矩阵
			model = new glm::mat4();

			// First, set the container's VAO (and VBO)
			glGenVertexArrays(1, &containerVAO);
			glGenBuffers(1, &VBO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glBindVertexArray(containerVAO);
			// Position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			// Normal attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);
			glBindVertexArray(0);

			// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
			glGenVertexArrays(1, &lightVAO);
			glBindVertexArray(lightVAO);
			// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// Set the vertex attributes (only position data for the lamp))
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the normal vectors
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);
		}
		else if (viewModel == WL_M) {
			// Build and compile our shader program
			lightingShader = new Shader("shaders/lighting_maps.vs", "shaders/lighting_maps.frag");
			lampShader = new Shader("shaders/lamp.vs", "shaders/lamp.frag");

			// 新建转换矩阵
			model = new glm::mat4();

			// First, set the container's VAO (and VBO)
			glGenVertexArrays(1, &containerVAO);
			glGenBuffers(1, &VBO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_1), vertices_1, GL_STATIC_DRAW);

			glBindVertexArray(containerVAO);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
			glEnableVertexAttribArray(2);
			glBindVertexArray(0);

			// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
			glGenVertexArrays(1, &lightVAO);
			glBindVertexArray(lightVAO);
			// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// Set the vertex attributes (only position data for the lamp))
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);


			// Load textures
			glGenTextures(1, &diffuseMap);
			glGenTextures(1, &specularMap);
			glGenTextures(1, &emissionMap);
			// Diffuse map
			int width, height;
			// 读取图像
			FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType("res/container2.png", 0);
			FIBITMAP * bitmap1 = FreeImage_Load(fifmt, "res/container2.png", 0);

			// 获得图像的宽和高（像素）
			width = FreeImage_GetWidth(bitmap1);
			height = FreeImage_GetHeight(bitmap1);

			if (FreeImage_GetBPP(bitmap1) != 32)
			{
				FIBITMAP* tempImage = bitmap1;
				bitmap1 = FreeImage_ConvertTo32Bits(tempImage);
			}
			// 修改freeimage的颜色存储方式
			BYTE *bits = new BYTE[FreeImage_GetWidth(bitmap1) * FreeImage_GetHeight(bitmap1) * 4];
			BYTE *pixels = (BYTE*)FreeImage_GetBits(bitmap1);

			for (int pix = 0; pix<FreeImage_GetWidth(bitmap1) * FreeImage_GetHeight(bitmap1); pix++)
			{
				bits[pix * 4 + 0] = pixels[pix * 4 + 2];
				bits[pix * 4 + 1] = pixels[pix * 4 + 1];
				bits[pix * 4 + 2] = pixels[pix * 4 + 0];
				bits[pix * 4 + 3] = pixels[pix * 4 + 3]; // Add this line to copy Alpha
			}

			glBindTexture(GL_TEXTURE_2D, diffuseMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
			glGenerateMipmap(GL_TEXTURE_2D);
			FreeImage_Unload(bitmap1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

			// Specular map
			// 读取图像
			fifmt = FreeImage_GetFileType("res/container2_specular.png", 0);
			bitmap1 = FreeImage_Load(fifmt, "res/container2_specular.png", 0);

			// 获得图像的宽和高（像素）
			width = FreeImage_GetWidth(bitmap1);
			height = FreeImage_GetHeight(bitmap1);

			if (FreeImage_GetBPP(bitmap1) != 32)
			{
				FIBITMAP* tempImage = bitmap1;
				bitmap1 = FreeImage_ConvertTo32Bits(tempImage);
			}
			// 修改freeimage的颜色存储方式
			bits = new BYTE[FreeImage_GetWidth(bitmap1) * FreeImage_GetHeight(bitmap1) * 4];
			pixels = (BYTE*)FreeImage_GetBits(bitmap1);

			for (int pix = 0; pix<FreeImage_GetWidth(bitmap1) * FreeImage_GetHeight(bitmap1); pix++)
			{
				bits[pix * 4 + 0] = pixels[pix * 4 + 2];
				bits[pix * 4 + 1] = pixels[pix * 4 + 1];
				bits[pix * 4 + 2] = pixels[pix * 4 + 0];
				bits[pix * 4 + 3] = pixels[pix * 4 + 3]; // Add this line to copy Alpha
			}
			glBindTexture(GL_TEXTURE_2D, specularMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
			glGenerateMipmap(GL_TEXTURE_2D);
			FreeImage_Unload(bitmap1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);


			// Set texture units
			lightingShader->Use();
			glUniform1i(glGetUniformLocation(lightingShader->Program, "material.diffuse"), 0);
			glUniform1i(glGetUniformLocation(lightingShader->Program, "material.specular"), 1);
		}
		else if (viewModel == SJ_M) {
			// 立方体6个面的中心
			windows.clear();
			windows.push_back(glm::vec3(0.0f, 0.0f, -0.5f));
			windows.push_back(glm::vec3(0.0f, 0.0f, 0.5f));
			windows.push_back(glm::vec3(-0.5f, 0.0f, 0.0f));
			windows.push_back(glm::vec3(0.5f, 0.0f, 0.0f));
			windows.push_back(glm::vec3(0.0f, -0.5f, 0.0f));
			windows.push_back(glm::vec3(0.0f, 0.5f, 0.0f));


			// Setup and compile our shaders
			shader = new Shader("shaders/blending_sorted.vs", "shaders/blending_sorted.frag");
			// 新建转换矩阵
			model = new glm::mat4();
			// 进行一定的旋转，避免出现显示bug
			*model = glm::rotate(*model, 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
			*model = glm::rotate(*model, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
			*model = glm::rotate(*model, 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));

			// enable
			glEnable(GL_BLEND);// 开启混合
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// 设置混合参数

			// Setup cube VAO
			glGenVertexArrays(1, &cubeVAO);
			glGenBuffers(1, &cubeVBO);
			glBindVertexArray(cubeVAO);
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glBindVertexArray(0);

			cubeTexture = loadTexture("res/window.png", true);

		}

		MessageBox(L"Setting completed.");
		
		// 重新绘图
		RedrawWindow();
	}
}


void CMFC_OpenGLView::OnDrawLoad()
{
	// TODO: 在此添加命令处理程序代码
	wchar_t *filters = L"三维文件(*.obj)|*.obj||";
	CFileDialog fileDlg(TRUE, L"obj", L"*.obj", OFN_HIDEREADONLY, filters);
	if (fileDlg.DoModal() == IDOK) {
		glDisable(GL_BLEND);// 取消混合
		// viewModel = 5;// 表示载入文件
		CString sPath = fileDlg.GetPathName();
		string str = CStringA(sPath);
		// 转\为/
		for (int i = 0; i < str.length(); i++)
			if (str[i] == '\\') str[i] = '/';
		// Setup and compile our shaders
		ourShader = new Shader("shaders/shader.vs", "shaders/shader.frag");
		// Load models
		ourModel = new Model(str.c_str());
		// 新建转换矩阵
		model = new glm::mat4();
		MessageBox(sPath, L"Load Successfully.");
		// 重新绘图
		RedrawWindow();
	}
}

GLuint CMFC_OpenGLView::loadTexture(GLchar const * path, GLboolean alpha)
{
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	// 读取图像
	FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(path, 0);
	FIBITMAP * bitmap1 = FreeImage_Load(fifmt, path, 0);

	// 获得图像的宽和高（像素）
	width = FreeImage_GetWidth(bitmap1);
	height = FreeImage_GetHeight(bitmap1);

	if (FreeImage_GetBPP(bitmap1) != 32)
	{
		FIBITMAP* tempImage = bitmap1;
		bitmap1 = FreeImage_ConvertTo32Bits(tempImage);
	}
	// 修改freeimage的颜色存储方式
	BYTE *bits = new BYTE[FreeImage_GetWidth(bitmap1) * FreeImage_GetHeight(bitmap1) * 4];
	BYTE *pixels = (BYTE*)FreeImage_GetBits(bitmap1);

	for (int pix = 0; pix<FreeImage_GetWidth(bitmap1) * FreeImage_GetHeight(bitmap1); pix++)
	{
		bits[pix * 4 + 0] = pixels[pix * 4 + 2];
		bits[pix * 4 + 1] = pixels[pix * 4 + 1];
		bits[pix * 4 + 2] = pixels[pix * 4 + 0];
		bits[pix * 4 + 3] = pixels[pix * 4 + 3]; // Add this line to copy Alpha
	}
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, bits);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);	// Use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes value from next repeat 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	FreeImage_Unload(bitmap1);
	return textureID;

}

void CMFC_OpenGLView::OnRotateX()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = ROTATE_X;
}


void CMFC_OpenGLView::OnUpdateRotateX(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == ROTATE_X);
}


void CMFC_OpenGLView::OnRotateY()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = ROTATE_Y;
}


void CMFC_OpenGLView::OnUpdateRotateY(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == ROTATE_Y);
}


void CMFC_OpenGLView::OnRotateZ()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = ROTATE_Z;
}


void CMFC_OpenGLView::OnUpdateRotateZ(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == ROTATE_Z);
}


void CMFC_OpenGLView::OnMoveX()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = MOVE_X;
}


void CMFC_OpenGLView::OnUpdateMoveX(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == MOVE_X);
}


void CMFC_OpenGLView::OnMoveY()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = MOVE_Y;
}


void CMFC_OpenGLView::OnUpdateMoveY(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == MOVE_Y);
}


void CMFC_OpenGLView::OnMoveZ()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = MOVE_Z;
}


void CMFC_OpenGLView::OnUpdateMoveZ(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == MOVE_Z);
}


void CMFC_OpenGLView::OnScale()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = SCALE;
}


void CMFC_OpenGLView::OnUpdateScale(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == SCALE);
}


void CMFC_OpenGLView::OnNone()
{
	// TODO: 在此添加命令处理程序代码
	trans_type = NONE;
}


void CMFC_OpenGLView::OnUpdateNone(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(trans_type == NONE);
}
