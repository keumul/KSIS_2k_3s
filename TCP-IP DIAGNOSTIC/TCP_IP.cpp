#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib");
#pragma comment(lib, "ws2_32.lib");

#define IP_STATUS_BASE 11000
#define IP_SUCCESS 0
#define IP_DEST_NET_UNREACHABLE 11002
#define IP_DEST_HOST_UNREACHABLE 11003
#define IP_DEST_PROT_UNREACHABLE 11004
#define IP_DEST_PORT_UNREACHABLE 11005
#define IP_REQ_TIMED_OUT 11010
#define IP_BAD_REQ 11011
#define IP_BAD_ROUTE 11012  
#define IP_TTL_EXPIRED_TRANSIT 11013

void Ping(const char* cHost,     // адрес ip
    unsigned int Timeout,        // ожидаемое время отклика
    unsigned int RequestCount) { // кол-во запросов 

    //получение манипулятора для выполнения ICMP-запросов
    HANDLE hIP = IcmpCreateFile(); //создать файл сервиса
    if (hIP == INVALID_HANDLE_VALUE) {
        WSACleanup();            // завершает использование win sock 2
        return;
    }
    char SendData[32] = "Data for ping"; // буфер для передачи
    // statistic
    int LostPacketsCount = 0;    // кол-во потерянных пакетов
    unsigned int MaxMS = 0;      // максимальное время отклика (мс)
    int MinMS = -1;              // минимальное время отклика (мс)
    // выделяем память под пакет - буфер ответа
    PICMP_ECHO_REPLY pIpe = (PICMP_ECHO_REPLY)GlobalAlloc(GHND, sizeof(ICMP_ECHO_REPLY) + sizeof(SendData));
    if (pIpe == 0) {
        IcmpCloseHandle(hIP);
        WSACleanup();            
        return;
    }

    pIpe->Data = SendData;
    pIpe->DataSize = sizeof(SendData);
    unsigned long ipaddr = inet_addr(cHost); // получение сетевого формата адреса
    IP_OPTION_INFORMATION option = { 255,0,0,0,0 };  // ставим опции

    for (unsigned int c = 0; c < RequestCount; c++) {
        int dwStatus = IcmpSendEcho(hIP, //манипулятор
            ipaddr,                      //IP-адрес опрашиваемого узла
            SendData,                    //указатели на отправляемые ехо-запросы
            sizeof(SendData),            //размер этих данных
            &option,                     //указатель га структуру, соде-е доп опции запроса
            pIpe,                        //указатель на буфер для эхо-ответов
            sizeof(ICMP_ECHO_REPLY) + sizeof(SendData),
            Timeout);                    //входной параметр функции ping

        // выводим на консоль IP - адрес,время, прошедшее с момента отправки эхо - запроса до получения эхо ответа(RTT), 
        // число переданных байт и т.п.Фиксируем максимальное и минимальное время.
        if (dwStatus > 0) {
            for (int i = 0; i < dwStatus; i++) {
                unsigned char* pIpPtr = (unsigned char*)&pIpe->Address;

                std::cout << "Ответ от " << (int)*(pIpPtr)
                    << '.' << (int)*(pIpPtr + 1)
                    << '.' << (int)*(pIpPtr + 2)
                    << '.' << (int)*(pIpPtr + 3)
                    << ": число байт = " << pIpe->DataSize
                    << " время = " << pIpe->RoundTripTime
                    << "мс TTL = " << (int)pIpe->Options.Ttl
                    << std::endl;
                MaxMS = (MaxMS > pIpe->RoundTripTime) ? MaxMS : pIpe->RoundTripTime;
                MinMS = (MinMS < (int)pIpe->RoundTripTime&& MinMS >= 0) ? MinMS : pIpe->RoundTripTime;
            }                           
        }
        else {                  //число потерянных пакетов и выводим код ошибки
            if (pIpe->Status) {
                LostPacketsCount++;
                switch (pIpe->Status) {
                case IP_DEST_NET_UNREACHABLE:
                case IP_DEST_HOST_UNREACHABLE:
                case IP_DEST_PROT_UNREACHABLE:
                case IP_DEST_PORT_UNREACHABLE:
                    std::cout << "Remote host may be down." << std::endl;
                    break;
                case IP_REQ_TIMED_OUT:
                    std::cout << "Request timed out." << std::endl;
                    break;
                case IP_TTL_EXPIRED_TRANSIT:
                    std::cout << "TTL expired in transit." << std::endl;
                    break;
                default:
                    std::cout << "Error code: %ld" << pIpe->Status << std::endl;
                    break;
                }
            }
        }
    }
    IcmpCloseHandle(hIP);
    WSACleanup();
    // выводим статистику
    if (MinMS < 0) MinMS = 0;
    unsigned char* pByte = (unsigned char*)&pIpe->Address;
    std::cout << "Статистика Ping    "
        << (int)*(pByte)
        << "." << (int)*(pByte + 1)
        << "." << (int)*(pByte + 2)
        << "." << (int)*(pByte + 3) << std::endl;
    std::cout << "\tПакетов: отправлено = " << RequestCount
        << ", получено = " << RequestCount - LostPacketsCount
        << ", потеряно = " << LostPacketsCount << "<" << (int)(100 / (float)RequestCount) * LostPacketsCount << " % потерь>, " << std::endl;
    std::cout << "Приблизительное время приема-передачи:" << std::endl
        << "Минимальное = " << MinMS
        << "мс, Максимальное = " << MaxMS
        << "мс, Среднее = " << (MaxMS + MinMS) / 2 << "мс" << std::endl;
}

int main() {
    setlocale(LC_ALL, "rus");
    Ping("192.168.244.130", 60, 10); //ping адресa
  //  Ping("127.0.0.1", 60, 7);   //ping себя
    system("pause");
    return 0;
}