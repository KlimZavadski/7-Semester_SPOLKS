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

const string path = "d:\\1\\Files\\";
sockaddr_in serverAddress;
int result = 0;


void Initialize()
{
    puts("Initializing.");
    serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2000);

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

void TransmitFile(SOCKET clientSocket, string name)
{
    char *buffer = new char[blockSize];
    string filePath = path + name;
    
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
            sBytes = send(clientSocket, buffer, blockSize, 0);
        }
        else
        {
            sBytes = send(clientSocket, buffer, fileSize - transmitBytes, 0);
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
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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
    result = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR)
    {
        printf("Error: Invalid Bind (%d)\n", WSAGetLastError());
        return;
    }
    
    /* ��������� ��������� ������� ������ ��� ������������ ���������� � ���������
    s - ���������� ������, ������� ����� ��������� � ����� �������������
    backlog - ������������ ����� ���������, �� ��� �� �������� ����������. ������� ��������, ��� ��� �� ������������ ����� ������������� ���������� � ������ ������, � ���� ������������ ����� �������� ������������� ����������, ��������� � �������, ���� ���������� �� ������. (������ 5)
    */
    result = listen(serverSocket, 5);
    if (result == SOCKET_ERROR)
    {
        printf("Error: Invalid Listen (%d)\n", WSAGetLastError());
        return;
    }

    puts("Server started.");
    while (true)
    {
        char *buffer = new char[blockSize];
        sockaddr_in clientAddress;
        int size = sizeof(clientAddress);
        /* ���������� ����� ���������� �� ������ ����� ���������� � ���������  sockaddr_in, �� ������� ��������� �������� addr
        s - ���������� ������, ������� ����� ��������� � ����� �������������
        backlog - ������������ ����� ���������, �� ��� �� �������� ����������. ������� ��������, ��� ��� �� ������������ ����� ������������� ���������� � ������ ������, � ���� ������������ ����� �������� ������������� ����������, ��������� � �������, ���� ���������� �� ������. (������ 5)
        */
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &size);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("Error: Invalid Accept (%d)\n", WSAGetLastError());
            return;
        }

        int rButes = recv(clientSocket, buffer, blockSize, 0);
        if (rButes == -1)
        {
            printf("Error: Invalid Recieve (%d)\n", WSAGetLastError());
            return;
        }
        else
        {
            buffer[rButes] = '\0';
            if (IsFileExist(buffer))
            {
                TransmitFile(clientSocket, buffer);
            }
        }
        closesocket(clientSocket);
    }
    WSACleanup();
}