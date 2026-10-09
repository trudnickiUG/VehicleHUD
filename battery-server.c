#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

int main(void)
{
    WSADATA wsa;
    SOCKET server, client;
    struct sockaddr_in addr;
    char request[1024];
    char body[128];
    char response[512];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8765);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server, (struct sockaddr *)&addr,
             sizeof(addr)) == SOCKET_ERROR) {
        printf("Bind failed. Port 8765 may already be in use.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    if (listen(server, 5) == SOCKET_ERROR) {
        printf("Listen failed.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    printf("Battery server running at 127.0.0.1:8765\n");
    printf("Keep this window open while using the webpage.\n");

    while (1) {
        client = accept(server, NULL, NULL);

        if (client == INVALID_SOCKET)
            continue;

        int received = recv(client, request,
                            sizeof(request) - 1, 0);

        if (received > 0) {
            SYSTEM_POWER_STATUS ps;

            request[received] = '\0';

            if (GetSystemPowerStatus(&ps)) {
                snprintf(body, sizeof(body),
                    "{\"level\":%u,\"charging\":%s}",
                    (unsigned int)ps.BatteryLifePercent,
                    (ps.BatteryFlag & 8) ? "true" : "false");
            } else {
                snprintf(body, sizeof(body),
                    "{\"level\":255,\"charging\":false}");
            }

            snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Cache-Control: no-store\r\n"
                "Connection: close\r\n"
                "Content-Length: %u\r\n"
                "\r\n%s",
                (unsigned int)strlen(body), body);

            send(client, response,
                 (int)strlen(response), 0);
        }

        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}