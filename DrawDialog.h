#pragma once


// CDrawDialog 对话框

class CDrawDialog : public CDialog
{
	DECLARE_DYNAMIC(CDrawDialog)

public:
	CDrawDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDrawDialog();
// Opengl新增
	int MyModel;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DRAW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
