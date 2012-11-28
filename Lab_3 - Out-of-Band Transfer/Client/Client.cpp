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
        printf("\nError: Invalid Socket (%d)\n", WSAGetLastError());
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
        printf("\nError: Invalid Connection (%d)\n", WSAGetLastError());
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