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

#define blockSize 1024 * 10
#define timeoutRead 3000
#define timeoutWrite 5000

const string path = "d:\\1\\Client\\";
const int port = 2000;

sockaddr_in clientAddress, serverAddress;
int size;
int result = 0;


void Initialize()
{
    puts("Initializing.");
    clientAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(port);

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

void GetFileInfo(char *buffer, unsigned long &size, unsigned long &count)
{
    //
}

void main()
{
    Initialize();
    printf("Please, input name of file: ");
    char *info = new char[blockSize];
    int rBytes, sBytes;
    gets(info);

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
    DWORD tr = timeoutRead + 10000;
    DWORD tw = timeoutWrite + 10000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tr, sizeof(tr));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&tw, sizeof(tw));

    sBytes = sendto(clientSocket, info, strlen(info), 0, (struct sockaddr*)&serverAddress, size);
    if (sBytes == -1)
    {
        printf("Error: Invalid sending file name (%d)\n", WSAGetLastError());
        return;
    }

    // Receive answer from Server about a file.
    rBytes = recvfrom(clientSocket, info, blockSize, 0, (struct sockaddr*)&serverAddress, &size);
    if (rBytes == -1)
    {
        printf("Error: Invalid receiving file length from Server (%d)\n", WSAGetLastError());
        return;
    }
    else if (rBytes == 8)
    {
        unsigned long fileSize = 0;
        unsigned long packageCount = 0;
        unsigned long pacakages = 0;
        // Solving size of file and count of packeges.
        GetFileInfo(info, fileSize, packageCount);

        string filePath = path + info;
        ofstream file;
        file.open(filePath.c_str(), ios::out | ios::binary);
        
        printf("\n\tSend ok.\n\t\t");
        sBytes = sendto(clientSocket, "1", 1, 0, (struct sockaddr*)&serverAddress, size);
        if (sBytes == -1)
        {
            printf("Error: Invalid sending Ready for transmitting (%d)\n", WSAGetLastError());
            return;
        }

        printf("\n\tStart receiving file.\n\t\t");
        while (pacakages < packageCount)
        {
            char *buffer = new char[blockSize];

            int rBytes = recvfrom(clientSocket, buffer, blockSize, 0, (struct sockaddr*)&serverAddress, &size);
            if (rBytes == -1)
            {
                int e = GetLastError();
                if (e == WSAETIMEDOUT)
                {
                    puts("\t\nFile receive correctly!");
                }
                else
                {
                    printf("\tError: File don't receive (%d)\n\n", e);
                }
                break;
            }

            pacakages++;
            file.write(buffer, rBytes);
            delete buffer;

            sBytes = sendto(clientSocket, "1", 1, 0, (struct sockaddr*)&serverAddress, size);
            if (sBytes == -1)
            {
                printf("Error: Invalid sending Ready for transmitting (%d)\n", WSAGetLastError());
                return;
            }
        }
        file.close();
        //system(filePath.c_str());
    }
    closesocket(clientSocket);
    WSACleanup();
}