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
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Error: Invalid Socket (%d)\n", WSAGetLastError());
        return;
    }

    /* ”станавливаем соединение с сервером
    s - это дескриптор сокета, который вернул системный вызов
    peer - указывает на структуру, в которой хранитс€ адрес удаленного хоста и некотора€ дополнительна€ информаци€
    peer_len - содержит размер структуры в байтах
    */
    result = connect(clientSocket, (struct sockaddr*) &clientAddress, sizeof(clientAddress));
    if (result != 0)
    {
        printf("Error: Invalid Connection (%d)\n", WSAGetLastError());
        return;
    }
    
    /* ѕередача данных
    flags:
        MSG_OOB - следует послать или прин€ть срочные данные
        MSG_PEEK - используетс€ дл€ просмотра поступивших данных без их удалени€ из приемного буфера. ѕосле возврата из системного вызова данные еще  могут быть получены при последующем вызове read или recv
        MSG_DONTROUTE - сообщает €дру, что не надо выполн€ть обычный алгоритм маршрутизации.  ак правило, используетс€ программами маршрутизации  или дл€ диагностических целей
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