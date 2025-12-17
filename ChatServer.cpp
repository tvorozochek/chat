#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

SOCKET Connections[100];
string Names[100]; // Массив для имен
int Counter = 0;

enum Packet { Pack, PrivatePack, Test };

DWORD WINAPI ServerThread(LPVOID param) {
    int index = *(int*)param;
    delete (int*)param; // Очищаем память, выделенную под индекс
    SOCKET Con = Connections[index];

    while (true) {
        Packet packettype;
        if (recv(Con, (char*)&packettype, sizeof(Packet), NULL) <= 0) break;

        if (packettype == Pack) {
            int msg_size;
            recv(Con, (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1];
            msg[msg_size] = 0;
            recv(Con, msg, msg_size, NULL);

            string messageStr = msg;
            cout << "[" << Names[index] << "]: " << messageStr << endl; // Сервер видит всё

            // Проверка на личное сообщение: @Имя Текст
            if (messageStr[0] == '@') {
                size_t spacePos = messageStr.find(' ');
                if (spacePos != string::npos) {
                    string targetName = messageStr.substr(1, spacePos - 1);
                    string privateMsg = "(Private) " + Names[index] + ": " + messageStr.substr(spacePos + 1);

                    int p_size = privateMsg.size();
                    bool found = false;
                    for (int i = 0; i < Counter; i++) {
                        if (Names[i] == targetName) {
                            Packet pType = Pack;
                            send(Connections[i], (char*)&pType, sizeof(Packet), NULL);
                            send(Connections[i], (char*)&p_size, sizeof(int), NULL);
                            send(Connections[i], privateMsg.c_str(), p_size, NULL);
                            found = true;
                            break;
                        }
                    }
                }
            }
            else {
                // Обычный Broadcast (всем)
                string broadcastMsg = Names[index] + ": " + messageStr;
                int b_size = broadcastMsg.size();
                for (int i = 0; i < Counter; i++) {
                    if (i == index) continue;
                    Packet pType = Pack;
                    send(Connections[i], (char*)&pType, sizeof(Packet), NULL);
                    send(Connections[i], (char*)&b_size, sizeof(int), NULL);
                    send(Connections[i], broadcastMsg.c_str(), b_size, NULL);
                }
            }
            delete[] msg;
        }
    }
    return 0;
}

int main() {
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(123);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, 10);

    cout << "Server started..." << endl;

    for (int i = 0; i < 100; i++) {
        int sizeofaddr = sizeof(addr);
        SOCKET newCon = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

        if (newCon != INVALID_SOCKET) {
            // Сначала получаем имя клиента
            int name_size;
            recv(newCon, (char*)&name_size, sizeof(int), NULL);
            char* name = new char[name_size + 1];
            name[name_size] = 0;
            recv(newCon, name, name_size, NULL);

            Connections[Counter] = newCon;
            Names[Counter] = name;

            cout << "Client Connected: " << Names[Counter] << endl;

            int* index = new int(Counter);
            CreateThread(NULL, NULL, ServerThread, index, NULL, NULL);
            Counter++;
            delete[] name;
        }
    }
    return 0;
}