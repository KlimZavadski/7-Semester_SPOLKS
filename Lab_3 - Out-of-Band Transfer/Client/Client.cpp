// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <Windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
using namespace std;

#define blockSize 1024 * 100
#define timeout 3000

const string path = "d:\\1\\Client\\";
sockaddr_in clientAddress;
int result = 0;


void Initialize()
{
    puts("Initializing.");
    clientAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(2000);

    // Initialize Winsock
    WSADATA wsaData;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("\nWSAStartup failed (%d)\n", result);
        return;
    }
}

void PutProgress(int progress)
{
    if (progress > 9)
    {
        printf("%c", 0x08);
    }
    printf("%c%c%d%%", 0x08, 0x08, progress);
}


void main()
{
    Initialize();
    printf("Please, input name of file: ");
    char *name = new char[blockSize];
    gets(name);

    /* ѕолучаем сокет.
    domain:
        AF_INET - internet
        AF_LOCAL - межпроцессное взаимодействие (IPC)
    type:
        SOCK_STREAM - обеспечивают надежный дуплексный протокол на основе установлени€ логического соединени€ (TCP)
        SOCK_DGRAM - обеспечивают ненадежный сервис доставки датаграмм (UDP)
        SOCK_RAW - предоставл€ют доступ к некоторым датаграммам на уровне протокола IP. ќни используютс€ в особых случа€х, например дл€ просмотра всех ICћ–-сообщений.
    protocol:
        0 - (auto) определ€етс€ типом сокета
        raw - простой сокет
    */
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("\nError: Invalid Socket (%d)\n", WSAGetLastError());
        return;
    }

    /* ”станавливаем соединение с сервером
    s - это дескриптор сокета, который вернул системный вызов
    peer - указывает на структуру, в которой хранитс€ адрес удаленного хоста и некотора€ дополнительна€ информаци€
    peer_len - содержит размер структуры в байтах
    */
    result = connect(clientSocket, (struct sockaddr*) &clientAddress, sizeof(clientAddress));
    if (result != 0)
    {
        printf("\nError: Invalid Connection (%d)\n", WSAGetLastError());
        return;
    }
    
    /* ѕередача данных
    flags:
        MSG_OOB - следует послать или прин€ть срочные данные
        MSG_PEEK - используетс€ дл€ просмотра поступивших данных без их удалени€ из приемного буфера. ѕосле возврата из системного вызова данные еще  могут быть получены при последующем вызове read или recv
        MSG_DONTROUTE - сообщает €дру, что не надо выполн€ть обычный алгоритм маршрутизации.  ак правило, используетс€ программами маршрутизации  или дл€ диагностических целей
    */
    int sBytes = send(clientSocket, name, strlen(name), 0);
    if (sBytes == -1)
    {
        printf("\nError: Invalid Send (%d)\n", WSAGetLastError());
        return;
    }

    // Setup timeout for receive calls.
    DWORD t = timeout;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &t, sizeof(t));

    // Setup Out of Band transfer mode.
    fd_set rset, xset;
    FD_ZERO(&rset);
    FD_ZERO(&xset);    

    long fileSize = 0;
    string filePath = path + name;
    ofstream file;
    file.open(filePath.c_str(), ios::out | ios::binary);
    printf("\n\tStart receive file.\n\t\tProgress:  0%%");

    while (true)
    {
        FD_SET(clientSocket, &rset);
        FD_SET(clientSocket, &xset);
        select(clientSocket + 1, &rset, NULL, &xset, NULL);

        if (FD_ISSET(clientSocket, &rset))
        {
            char *buffer = new char[blockSize];
            int rButes = recv(clientSocket, buffer, blockSize, 0);
            if (rButes == -1)
            {
                int e = GetLastError();
                if (e == WSAETIMEDOUT)
                {
                    printf("\n\tTime out!");
                }
                printf("\nError: File don't receive (%d)", e);
                break;
            }
            if (rButes == 0)
            {
                if (fileSize == 0)
                {
                    printf("\tError: File not found on the server!");
                }
                {
                    printf("\tFile receive correctly!");
                }
                break;
            }
            file.write(buffer, rButes);
            fileSize += rButes;
            delete buffer;
        }
        if (FD_ISSET(clientSocket, &xset)) // receive Out of Band data.
        {
            char c;
            recv(clientSocket, &c, 1, MSG_OOB);
            PutProgress((int)c);
        }
    }
    printf("\n\n\tEnd receiving file with size %d bytes\n\n", fileSize);
    file.close();
    closesocket(clientSocket);
    WSACleanup();    
}