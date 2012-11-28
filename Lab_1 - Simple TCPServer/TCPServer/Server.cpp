// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <iomanip>
using namespace std;


char buffer[50];
int result = 0;
sockaddr_in serverAddress;


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

        int rButes = recv(clientSocket, buffer, 50, 0);
        if (rButes == -1)
        {
            printf("Error: Invalid Recieve (%d)\n", WSAGetLastError());
            return;
        }
        else
        {
            buffer[rButes] = '\0';
            printf ("\n\n\tRecieve result: %s\n", buffer);
            string str = buffer;

            if (str == "time")
            {
                time_t seconds = time(NULL);
                tm* timeinfo = localtime(&seconds);
                strcpy(buffer, asctime(timeinfo));
                
                printf("\tSend datetime now: %s\n", buffer);
                int sBytes = send(clientSocket, buffer, strlen(buffer), 0);
                if (sBytes == -1)
                {
                    printf("Error: Invalid Send (%d)\n", WSAGetLastError());
                    return;
                }
            }
        }
        closesocket(clientSocket);
    }
    WSACleanup();
}