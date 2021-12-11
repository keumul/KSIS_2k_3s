#include <iostream>
using namespace std;
#include <winsock2.h>
#include <iphlpapi.h> 
#include <stdio.h>
#include <conio.h>
#include <Ws2tcpip.h>
#pragma comment(lib , "iphlpapi.lib") 
#pragma comment(lib , "ws2_32.lib") 
void GetMacAddress(unsigned char*, struct in_addr);
int main()
{
	setlocale(LC_ALL, "RUS");
	unsigned char mac[6];
	struct in_addr srcip = { 0 };
	struct sockaddr_in sa;
	char ip_address[32];
	WSADATA firstsock;
	if (WSAStartup(MAKEWORD(2, 2), &firstsock) != 0)
	{
		cout << "Ошибка инициализации winsock";
			cout << WSAGetLastError();
		return -1;
	}
	cout << "Введите IP : ";
	cin >> ip_address;
	//преобразование IP адреса другим способом
	//srcip.s_addr = inet_addr(ip_address);
	inet_pton(AF_INET, ip_address, &(sa.sin_addr));
	//Получение MAC по IP
	GetMacAddress(mac, sa.sin_addr);
	//GetMacAddress(mac, srcip);
	printf("MAC адрес : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	printf("\n");
	
	_getch();
	return 0;
}
void GetMacAddress(unsigned char* mac, struct in_addr
	destip)
{
	DWORD ret;
	IPAddr srcip;
	ULONG MacAddr[2];
	ULONG PhyAddrLen = 6;
	int i;
	srcip = 0;
	//Послать ARP пакет
	ret = SendARP((IPAddr)destip.S_un.S_addr,
		srcip, MacAddr, &PhyAddrLen);
	//Преобразовать адрес
	if (PhyAddrLen)
	{
		BYTE* bMacAddr = (BYTE*)&MacAddr;
		for (i = 0; i < (int)PhyAddrLen; i++)
		{
			mac[i] = (char)bMacAddr[i];
		}
	}
}
