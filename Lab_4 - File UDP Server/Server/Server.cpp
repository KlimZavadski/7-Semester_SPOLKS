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

void PutProgress(int progress)
{
    if (progress > 9)
    {
        printf("%c", 0x08);
    }
    printf("%c%c%d%%", 0x08, 0x08, progress);
}

void TransmitFile(SOCKET serverSocket, string name)
{
    char *buffer = new char[blockSize];
    string filePath = path + name;
    
    DWORD tr = timeoutRead;
    DWORD tw = timeoutWrite;

    int sBytes;
    long transmitBytes = 0;
    ifstream file;
    file.open(filePath.c_str(), ios::in | ios::binary);
    file.seekg(0, ios_base::end);
    long fileSize = file.tellg();
    file.seekg(0);

    printf("\n\tStart sending file with size %d bytes\n\t\tProgress:  0%%", fileSize);
    while(!file.eof())
    {
        file.read(buffer, blockSize);
        if (fileSize - transmitBytes > blockSize)
        {
            sBytes = sendto(serverSocket, buffer, blockSize, 0, (struct sockaddr*)&clientAddress, size);
        }
        else
        {
            sBytes = sendto(serverSocket, buffer, fileSize - transmitBytes, 0, (struct sockaddr*)&clientAddress, size);
        }
        if (sBytes == -1)
        {
            printf("Error: Invalid Send (%d)\n", WSAGetLastError());
            return;
        }

        transmitBytes += sBytes;
        PutProgress(transmitBytes * 100 / fileSize);
    }
    file.close();
    puts("\tEnd sending file.");
}


void main()
{
    Initialize();

    puts("Server started.");
    while (true)
    {
        /* �������� �����.
        domain:
            AF_INET - internet
            AF_LOCAL - ������������� �������������� (IPC)
        type:
            SOCK_STREAM - ������������ �������� ���������� �������� �� ������ ������������ ����������� ���������� (TCP)
            SOCK_DGRAM - ������������ ���������� ������ �������� ��������� (UDP)
            SOCK_RAW - ������������� ������ � ��������� ����������� �� ������ ��������� IP. ��� ������������ � ������ �������, �������� ��� ��������� ���� IC��-���������.
        protocol:
            0 - (auto) ������������ ����� ������
            raw - ������� �����
        */
        SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (serverSocket == INVALID_SOCKET)
        {
            printf("Error: Invalid Socket (%d)\n", WSAGetLastError());
            return;
        }

        /* �������� ������ ���������� � ������ ����� � ��������������� ������
        s - ���������� ��������������� ������
        name - ���� � ������� ���������. ������ � �������� ������ �������� ��������� INADDR_ANY. ��� ��������, ��� ����� ������� ����������, ������������� �� ������ ����������. ���� ����� � ����������� �������� �������� ����� ��������� ���������� ������ �� ������ ����������, �� ������� ������� IP-����� ����� ����������
        namelen - ������ ���������
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