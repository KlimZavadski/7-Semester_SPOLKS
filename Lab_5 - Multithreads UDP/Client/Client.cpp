// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <Windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
using namespace std;

#define packageSize 10240
#define timeoutRead 3000
#define timeoutWrite 5000

const string path = "d:\\1\\Client_1\\";
const int port = 2000;

sockaddr_in clientAddress, serverAddress;
int size;
int result = 0;


void Initialize()
{
    puts("Initializing.");

    serverAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    size = sizeof(serverAddress);

    // Initialize Winsock
    WSADATA wsaData;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed (%d)\n", result);
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

void GetFileInfo(char *info, unsigned long &size, unsigned long &count)
{
    size = count = 0;

    for (int i = 0; i < 4; i++)
    {
        size <<= 8;
        size += (unsigned long)(byte)info[i];
    }
    for (int i = 4; i < 8; i++)
    {
        count <<= 8;
        count += (unsigned long)(byte)info[i];
    }
}

void main()
{
    Initialize();
    printf("Please, input name of file: ");
    char *info = new char[packageSize];
    int rBytes, sBytes;
    gets(info);
    string filePath = path + info;

    /* Получаем сокет.
    domain:
        AF_INET - internet
        AF_LOCAL - межпроцессное взаимодействие (IPC)
    type:
        SOCK_STREAM - обеспечивают надежный дуплексный протокол на основе установления логического соединения (TCP)
        SOCK_DGRAM - обеспечивают ненадежный сервис доставки датаграмм (UDP)
        SOCK_RAW - предоставляют доступ к некоторым датаграммам на уровне протокола IP. Они используются в особых случаях, например для просмотра всех ICМР-сообщений.
    protocol:
        0 - (auto) определяется типом сокета
        raw - простой сокет
    */
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Error: Invalid Socket (%d)\n", WSAGetLastError());
        return;
    }

    // Setup time out for receive calls.
    DWORD tr = timeoutRead;
    DWORD tw = timeoutWrite;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tr, sizeof(tr));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&tw, sizeof(tw));

    sBytes = sendto(clientSocket, info, strlen(info), 0, (struct sockaddr*)&serverAddress, size);
    if (sBytes == -1)
    {
        printf("Error: Invalid sending file name (%d)\n", WSAGetLastError());
        return;
    }

    // Receive answer from Server about a file.
    rBytes = recvfrom(clientSocket, info, packageSize, 0, (struct sockaddr*)&serverAddress, &size);
    if (rBytes == -1)
    {
        printf("Error: Invalid receiving file length from Server (%d)\n", WSAGetLastError());
        return;
    }
    else if (rBytes == 8)
    {
        unsigned long fileSize = 0;
        unsigned long packageCount = 0;
        unsigned long packages = 0;
        // Solving size of file and count of packages.
        GetFileInfo(info, fileSize, packageCount);

        ofstream file;
        file.open(filePath.c_str(), ios::out | ios::binary);
        
        printf("\n\tReady for receiving file.");
        sBytes = sendto(clientSocket, "1", 1, 0, (struct sockaddr*)&serverAddress, size);
        if (sBytes == -1)
        {
            printf("Error: Invalid sending Ready for transmitting (%d)\n", WSAGetLastError());
            return;
        }

        printf("\n\tStart receiving file with size %d bytes (%d packages).\n\t\tProgress:  0%%", fileSize, packageCount);
        while (packages < packageCount)
        {
            char *buffer = new char[packageSize];

            int rBytes = recvfrom(clientSocket, buffer, packageSize, 0, (struct sockaddr*)&serverAddress, &size);
            if (rBytes == -1)
            {
                int e = GetLastError();
                printf("\tError: File don't receive (%d)", WSAGetLastError());
                if (e == WSAETIMEDOUT)
                {
                    puts("- Time out!\n");
                }
                break;
            }

            packages++;
            file.write(buffer, rBytes);
            delete buffer;
            PutProgress(packages * 100 / packageCount);

            sBytes = sendto(clientSocket, "1", 1, 0, (struct sockaddr*)&serverAddress, size);
            if (sBytes == -1)
            {
                int e = GetLastError();
                printf("Error: Invalid sending Ready for transmitting (%d)\n", WSAGetLastError());
                if (e == WSAETIMEDOUT)
                {
                    puts("- Time out!\n");
                }
                break;
            }
        }
        file.close();
        printf("\n\tReceiving end correctly.");
        //system(filePath.c_str());
    }
    closesocket(clientSocket);
    WSACleanup();
    getch();
}