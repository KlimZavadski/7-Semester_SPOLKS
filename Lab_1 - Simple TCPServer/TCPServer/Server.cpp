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
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Error: Invalid Socket (%d)\n", WSAGetLastError());
        return;
    }

    /* Привязка адреса интерфейса и номера порта к прослушивающему сокету
    s - дескриптор прослушивающего сокета
    name - порт и сетевой интерфейс. Обычно в качестве адреса задается константа INADDR_ANY. Это означает, что будет принято соединение, запрашиваемое по любому интерфейсу. Если хосту с несколькими сетевыми адресами нужно принимать соединении только по одному интерфейсу, то следует указать IP-адрес этого интерфейса
    namelen - размер структуры
    */
    result = bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR)
    {
        printf("Error: Invalid Bind (%d)\n", WSAGetLastError());
        return;
    }
    
    /* Прослушка известных серверу портов для установления соединения с клиентами
    s - дескриптор сокета, который нужно перевести в режим прослушивания
    backlog - максимальное число ожидающих, но еще не принятых соединений. Следует отметить, что это не максимальное число одновременных соединений с данным портом, а лишь максимальное число частично установленных соединений, ожидающих в очереди, пока приложение их примет. (Обычно 5)
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
        /* Возвращает адрес приложения на другом конце соединения в структуре  sockaddr_in, на которую указывает параметр addr
        s - дескриптор сокета, который нужно перевести в режим прослушивания
        backlog - максимальное число ожидающих, но еще не принятых соединений. Следует отметить, что это не максимальное число одновременных соединений с данным портом, а лишь максимальное число частично установленных соединений, ожидающих в очереди, пока приложение их примет. (Обычно 5)
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