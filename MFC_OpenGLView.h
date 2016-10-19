
// MFC_OpenGLView.h : CMFC_OpenGLView 类的接口
//

#pragma once
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GL/wglew.h>

// GLFW
#include <GLFW/glfw3.h>

#include <common/shader.hpp>


class CMFC_OpenGLView : public CView
{
//openGL by proverbs
protected:
	HGLRC m_hRC;    //Rendering Context着色描述表   
	CClientDC* m_pDC;        //Device Context设备描述表   
	BOOL InitializeOpenGL();    //初始化 OpenGL

	GLuint shaderProgram;
	bool InitializeShader();	//初始化 shader，着色器

	void RenderScene();         //绘制场景  
	int m_wide;    //m_wide为在CVCOpenGL2View类中添加的表示视口宽度的成员变量   
	int m_heigth;  //m_height为在CVCOpenGL2View类中添加的表示视口高度的成员变量   

				   //Error Handling
	void SetError(int e);
	static const char* const ErrorStrings[];
	const char* m_ErrorString;

	BOOL GetRenderingContext();

	virtual BOOL GetOldStyleRenderingContext(void);
	virtual BOOL SetupPixelFormat(void);


protected: // 仅从序列化创建
	CMFC_OpenGLView();
	DECLARE_DYNCREATE(CMFC_OpenGLView)

// 特性
public:
	CMFC_OpenGLDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CMFC_OpenGLView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};

#ifndef _DEBUG  // MFC_OpenGLView.cpp 中的调试版本
inline CMFC_OpenGLDoc* CMFC_OpenGLView::GetDocument() const
   { return reinterpret_cast<CMFC_OpenGLDoc*>(m_pDocument); }
#endif

