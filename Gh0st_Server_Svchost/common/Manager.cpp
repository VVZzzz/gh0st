// Manager.cpp: implementation of the CManager class.
//
//////////////////////////////////////////////////////////////////////
#include "KeyboardManager.h"

#include "Manager.h"
#include "until.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CManager::CManager(CClientSocket *pClient)
{
    m_pClient = pClient;
    m_pClient->setManagerCallBack(this);

    /*
    bManualReset
    [in] Specifies whether a manual-reset or auto-reset event object is created. If TRUE, then you must use the ResetEvent function to manually reset the state to nonsignaled. If FALSE, the system automatically resets the state to nonsignaled after a single waiting thread has been released.
    */
    // �ڶ�������Ϊtrue,��ֹϵͳ�Զ������¼�
    m_hEventDlgOpen = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CManager::~CManager()
{
    TCHAR szModule[MAX_PATH];
    CloseHandle(m_hEventDlgOpen);
    CKeyboardManager::MyGetModuleFileName(NULL, szModule, MAX_PATH);
}
void CManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{

}

int CManager::Send(LPBYTE lpData, UINT nSize)
{
    int	nRet = 0;
    try
    {
        nRet = m_pClient->Send((LPBYTE)lpData, nSize);
    }
    catch (...){};
    return nRet;
}

void CManager::WaitForDialogOpen()
{
    TCHAR szModule[MAX_PATH];
    WaitForSingleObject(m_hEventDlgOpen, INFINITE);
    CKeyboardManager::MyGetModuleFileName(NULL, szModule, MAX_PATH);
    // �����Sleep,��ΪԶ�̴��ڴ�InitDialog�з���COMMAND_NEXT����ʾ��Ҫһ��ʱ��
    Sleep(150);
    CKeyboardManager::MyGetShortPathName(szModule, szModule, MAX_PATH);
}

void CManager::NotifyDialogIsOpen()
{
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    SetEvent(m_hEventDlgOpen);
    CloseServiceHandle(hSCM);
}