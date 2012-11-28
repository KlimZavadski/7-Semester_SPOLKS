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
        printf("WSAStartup failed (%d)\n", result);
        return;
    }
}


void main()
{
    Initialize();
    printf("Please, input name of file: ");
    char *name = new char[blockSize];
    gets(name);

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
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Error: Invalid Socket (%d)\n", WSAGetLastError());
        return;
    }

    /* ������������� ���������� � ��������
    s - ��� ���������� ������, ������� ������ ��������� �����
    peer - ��������� �� ���������, � ������� �������� ����� ���������� ����� � ��������� �������������� ����������
    peer_len - �������� ������ ��������� � ������
    */
    result = connect(clientSocket, (struct sockaddr*) &clientAddress, sizeof(clientAddress));
    if (result != 0)
    {
        printf("Error: Invalid Connection (%d)\n", WSAGetLastError());
        return;
    }
    
    /* �������� ������
    flags:
        MSG_OOB - ������� ������� ��� ������� ������� ������
        MSG_PEEK - ������������ ��� ��������� ����������� ������ ��� �� �������� �� ��������� ������. ����� �������� �� ���������� ������ ������ ���  ����� ���� �������� ��� ����������� ������ read ��� recv
        MSG_DONTROUTE - �������� ����, ��� �� ���� ��������� ������� �������� �������������. ��� �������, ������������ ����������� �������������  ��� ��� ��������������� �����
    */
    int sBytes = send(clientSocket, name, strlen(name), 0);
    if (sBytes == -1)
    {
        printf("Error: Invalid Send (%d)\n", WSAGetLastError());
        return;
    }

    DWORD timeout = 5000;
    string filePath = path + name;
    ofstream file;
    file.open(filePath.c_str(), ios::out | ios::binary);
    while (true)
    {
        char *buffer = new char[blockSize];
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)); // Setup time out for receive calls.

        int rButes = recv(clientSocket, buffer, blockSize, 0);
        if (rButes == -1)
        {
            int e = GetLastError();
            if (e == WSAETIMEDOUT)
            {
                printf("\tTime out!\n");
            }
            printf("\tError: File don't recieve (%d)\n\n", e);
            break;
        }
        if (rButes == 0)
        {
            puts("\t\nFile recieve correctly!");
            break;
        }
        file.write(buffer, rButes);
        delete buffer;
    }
    file.close();
    closesocket(clientSocket);
    WSACleanup();    
}