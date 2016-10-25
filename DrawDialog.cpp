// DrawDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "MFC_OpenGL.h"
#include "DrawDialog.h"
#include "afxdialogex.h"


// CDrawDialog 对话框

IMPLEMENT_DYNAMIC(CDrawDialog, CDialog)

CDrawDialog::CDrawDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DRAW, pParent)
{
	MyModel = 0;
}

CDrawDialog::~CDrawDialog()
{
}

void CDrawDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDrawDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CDrawDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CDrawDialog 消息处理程序


void CDrawDialog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (IsDlgButtonChecked(IDC_RADIO_XK)) MyModel = 1;
	else if (IsDlgButtonChecked(IDC_RADIO_M)) MyModel = 2;
	else if (IsDlgButtonChecked(IDC_RADIO_SJ)) MyModel = 3;
	else if (IsDlgButtonChecked(IDC_RADIO_WL)) MyModel = 4;

	CDialog::OnOK();
}
