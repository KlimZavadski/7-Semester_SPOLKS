// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>


char buffer[50] = "time";
int result = 0;
sockaddr_in clientAddress;


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
    puts("Do you want to send some information to the Server?");
    if (getch() == 'n') return;

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
    int sBytes = send(clientSocket, buffer, strlen(buffer), 0);
    if (sBytes == -1)
    {
        printf("Error: Invalid Send (%d)\n", WSAGetLastError());
        return;
    }
    else
    {
        printf("\n\n\tSend data: %s\n", buffer);
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
        printf ("\tRecieve result: %s\n\n", buffer);
    }
    closesocket(clientSocket);
}