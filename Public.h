#if !defined(AFX_PUBLIC_H__CADW455A_)
#define AFX_PUBLIC_H__CADW455A_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define XorValue 10			//ɱ����PID��������ֵ
#define IDR_ENCODE 513		//��Դ�е�������Ϣ
#define IDR_CONFIG 514		//��Դ�еķ�����Ϣ
#define IOCTL_KILL  (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA )
typedef struct _MYDATA
{
	ULONG Pid;
	ULONG ModuleAddress;
}MYDATA;//ɱ����ʱ��Ľṹ��






#endif