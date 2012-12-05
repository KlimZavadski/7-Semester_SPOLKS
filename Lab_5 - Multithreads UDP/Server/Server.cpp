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


#define packageSize 10240
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

char* GetFileInfo(unsigned long size, unsigned long &packageCount)
{
    unsigned long count = size / packageSize;
    if (size % packageSize != 0) count++;
    packageCount = count;
    
    char *info = new char[8];

    for (int i = 7; i > 3; i--)
    {
        info[i] = (char)count;
        if (count > 0)
        {
            count >>= 8;
        }
    }
    for (int i = 3; i >= 0; i--)
    {
        info[i] = (char)size;
        if (size > 0)
        {
            size >>= 8;
        }
    }
    
    return info;
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
    char *buffer = new char[packageSize];
    string filePath = path + name;
    
    // Setup timeout for receive calls.
    DWORD tr = timeoutRead;
    DWORD tw = timeoutWrite;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &tr, sizeof(tr));
    setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, (char*) &tw, sizeof(tw));

    int rBytes, sBytes, progress = 0;
    unsigned long fileSize, transmitedBytes = 0;
    unsigned long packageCount, packages = 0;
    ifstream file;
    file.open(filePath.c_str(), ios::in | ios::binary);
    file.seekg(0, ios_base::end);
    fileSize = file.tellg();
    file.seekg(0);

    // Send to client size of file and count of packages.
    char *info = GetFileInfo(fileSize, packageCount);
    sBytes = sendto(serverSocket, info, 8, 0, (struct sockaddr*)&clientAddress, size);
    if (sBytes == -1)
    {
        printf("\nError: Don't get answer from Client (%d)", WSAGetLastError());
        return -1;
    }

    printf("\n\tStart sending file with size %d bytes (%d packages)\n\t\tProgress:  0%%", fileSize, packageCount);
    while (true)
    {
        // Receive answer.
        rBytes = recvfrom(serverSocket, buffer, 1, 0, (struct sockaddr*)&clientAddress, &size);
        if (rBytes == -1)
        {
            int e = GetLastError();
            printf("Error: Invalid receive answer (%d)\n", WSAGetLastError());
            if (e == WSAETIMEDOUT)
            {
                puts("- Time out!\n");
            }
            break;
        }
        else
        {
            if (file.eof())
            {
                break;
            }
            file.read(buffer, packageSize);
        }
        
        // Send package.
        if (fileSize - transmitedBytes > packageSize)
        {
            rBytes = packageSize;
        }
        else
        {
            rBytes = fileSize - transmitedBytes;
        }

        sBytes = sendto(serverSocket, buffer, rBytes, 0, (struct sockaddr*)&clientAddress, size);
        if (sBytes == -1)
        {
            int e = GetLastError();
            printf("\nError: File don't send (%d)", WSAGetLastError());
            if (e == WSAETIMEDOUT)
            {
                puts("- Time out!\n");
            }
            break;
        }

        packages++;
        transmitedBytes += sBytes;
        progress = transmitedBytes * 100 / fileSize;
        PutProgress(progress);
    }

    file.close();
    puts("\n\tEnd sending file.");
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

        char *buffer = new char[packageSize];

        int rBytes = recvfrom(serverSocket, buffer, packageSize, 0, (struct sockaddr*)&clientAddress, &size);
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
                printf("\n\tFile '%s' ready for send.", buffer);
                TransmitFile(serverSocket, buffer);
            }
        }
        delete buffer;
        closesocket(serverSocket);
    }
    WSACleanup();
}