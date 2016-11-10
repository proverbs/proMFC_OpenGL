
// MFC_OpenGLView.h : CMFC_OpenGLView 类的接口
//

#pragma once

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GL/wglew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GL includes
#include "learnopengl/shader.h"
#include "learnopengl/camera.h"
#include "learnopengl/model.h"



// #include <SOIL/SOIL.h>
#include <FreeImage.h>

#include "DrawDialog.h"


class CMFC_OpenGLView : public CView
{
private:
	enum TRANS {NONE = 0, ROTATE_X, ROTATE_Y, ROTATE_Z, MOVE_X, MOVE_Y, MOVE_Z, SCALE};
	TRANS trans_type;
	enum GRAPH_T {NONE_M = 0, XK_M, M_M, SJ_M, WL_M};
	GRAPH_T viewModel;

//openGL by proverbs
protected:
	HGLRC m_hRC;    //Rendering Context着色描述表   
	CClientDC* m_pDC;        //Device Context设备描述表   
	BOOL InitializeOpenGL();    //初始化 OpenGL
	BOOL GetRenderingContext();
	virtual BOOL GetOldStyleRenderingContext();
	virtual BOOL SetupPixelFormat();	//设置像素格式  

	Shader *ourShader;

	void RenderScene();         //绘制场景  
	int m_wide;    //m_wide为在View类中添加的表示视口宽度的成员变量   
	int m_height;  //m_height为在View类中添加的表示视口高度的成员变量   

	//Error Handling
	void SetError(int e);
	static const char* const ErrorStrings[];
	const char* m_ErrorString;

	// control view
	Camera *camera;
	bool keys[1024];
	int lastX, lastY;
	bool firstMouse;
	bool isPress;// 鼠标左键是否按下
	void do_movement();
	

	// 绘图
	GLuint VBO, containerVAO;
	GLuint cubeVAO, cubeVBO;
	GLuint cubeTexture;
	GLuint lightVAO;
	GLuint diffuseMap, specularMap, emissionMap;
	Shader *lightingShader, *lampShader, *shader;
	
	// 透明立方体排序
	std::vector<glm::vec3> windows;

	// 载入
	Model *ourModel;

	GLuint loadTexture(GLchar const * path, GLboolean alpha);

	// 读入obj模型的变换矩阵
	glm::mat4 *model;
	


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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDrawOpt();
	afx_msg void OnDrawLoad();
	afx_msg void OnRotateX();
	afx_msg void OnUpdateRotateX(CCmdUI *pCmdUI);
	afx_msg void OnRotateY();
	afx_msg void OnUpdateRotateY(CCmdUI *pCmdUI);
	afx_msg void OnRotateZ();
	afx_msg void OnUpdateRotateZ(CCmdUI *pCmdUI);
	afx_msg void OnMoveX();
	afx_msg void OnUpdateMoveX(CCmdUI *pCmdUI);
	afx_msg void OnMoveY();
	afx_msg void OnUpdateMoveY(CCmdUI *pCmdUI);
	afx_msg void OnMoveZ();
	afx_msg void OnUpdateMoveZ(CCmdUI *pCmdUI);
	afx_msg void OnScale();
	afx_msg void OnUpdateScale(CCmdUI *pCmdUI);
	afx_msg void OnNone();
	afx_msg void OnUpdateNone(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // MFC_OpenGLView.cpp 中的调试版本
inline CMFC_OpenGLDoc* CMFC_OpenGLView::GetDocument() const
   { return reinterpret_cast<CMFC_OpenGLDoc*>(m_pDocument); }
#endif

