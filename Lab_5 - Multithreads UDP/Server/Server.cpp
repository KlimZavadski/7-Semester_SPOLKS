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

const string path = "d:\\1\\Files\\";
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

bool IsFileExist(string name)
{
    _WIN32_FIND_DATAA wfd;
    string filePath = path + name;
    return FindFirstFileA(filePath.c_str(), &wfd) != INVALID_HANDLE_VALUE;
}

void GetFileInfo(unsigned long size)
{
    unsigned long count = size / blockSize + 1;
    char *info = new char[8];

    for (int i = 7; i > 3; i--)
    {
        info[i] = (char)count;
        count = count >> 8;
    }
    for (int i = 3; i > 0; i--)
    {
        info[i] = (char)size;
        size = size >> 8;
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

unsigned long TransmitFile(SOCKET serverSocket, string name)
{
    char *buffer = new char[blockSize];
    string filePath = path + name;
    
    DWORD tr = timeoutRead;
    DWORD tw = timeoutWrite;

    // Setup timeout for receive calls.
    //setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &tr, sizeof(tr));
    //setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, (char*) &tw, sizeof(tw));

    int rBytes, sBytes, progress = 0;
    unsigned long transmitedBytes = 0;
    ifstream file;
    file.open(filePath.c_str(), ios::in | ios::binary);
    file.seekg(0, ios_base::end);
    unsigned long fileSize = file.tellg();
    file.seekg(0);

    // Send to client size of file and count of packages.
    sBytes = sendto(serverSocket, GetFileInfo(fileSize), 8, 0, (struct sockaddr*)&clientAddress, size);

    printf("\n\tStart sending file with size %d bytes\n\t\tProgress:  0%%", fileSize);
    while(!file.eof())
    {
        rBytes = recvfrom(serverSocket, buffer, 1, 0, (struct sockaddr*)&clientAddress, &size);
        if (rBytes == -1)
        {
            printf("Error: Invalid receive answer (%d)\n", WSAGetLastError());
            break;
        }
        else
        {
            printf("Answer #%d - %s", progress / 10, buffer[0]);
        }

        file.read(buffer, blockSize);
        if (fileSize - transmitedBytes > blockSize)
        {
            sBytes = sendto(serverSocket, buffer, blockSize, 0, (struct sockaddr*)&clientAddress, size);
        }
        else
        {
            sBytes = sendto(serverSocket, buffer, fileSize - transmitedBytes, 0, (struct sockaddr*)&clientAddress, size);
        }
        if (sBytes == -1)
        {
            int e = GetLastError();
            if (e == WSAETIMEDOUT)
            {
                printf("\n\tTime out!");
            }
            printf("\nError: File don't send (%d)", WSAGetLastError());
            break;
        }

        transmitedBytes += sBytes;
        progress = transmitedBytes * 100 / fileSize;
        PutProgress(progress);
    }
    file.close();
    puts("\tEnd sending file.");
    return transmitedBytes;
}


void main()
{
    Initialize();

    puts("Server started.");
    while (true)
    {
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
        SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (serverSocket == INVALID_SOCKET)
        {
            printf("Error: Invalid Socket (%d)\n", WSAGetLastError());
            return;
        }

        /* ѕрив€зка адреса интерфейса и номера порта к прослушивающему сокету
        s - дескриптор прослушивающего сокета
        name - порт и сетевой интерфейс. ќбычно в качестве адреса задаетс€ константа INADDR_ANY. Ёто означает, что будет прин€то соединение, запрашиваемое по любому интерфейсу. ≈сли хосту с несколькими сетевыми адресами нужно принимать соединении только по одному интерфейсу, то следует указать IP-адрес этого интерфейса
        namelen - размер структуры
        */
        result = bind(serverSocket, (struct sockaddr*)&serverAddress, size);
        if (result == SOCKET_ERROR)
        {
            printf("Error: Invalid Bind (%d)\n", WSAGetLastError());
            return;
        }

        char *buffer = new char[blockSize];

        int rBytes = recvfrom(serverSocket, buffer, blockSize, 0, (struct sockaddr*)&clientAddress, &size);
        if (rBytes == -1)
        {
            printf("Error: Invalid Connection (%d)\n", WSAGetLastError());
            return;
        }
        else
        {
            buffer[rBytes] = '\0';
            if (IsFileExist(buffer))
            {
                TransmitFile(serverSocket, buffer);
            }
        }
        delete buffer;
        closesocket(serverSocket);
    }
    WSACleanup();
}