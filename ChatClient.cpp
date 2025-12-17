#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

SOCKET Connection;
enum Packet { Pack, PrivatePack, Test };

DWORD WINAPI ClientThread(LPVOID sock) {
    SOCKET Con = *(SOCKET*)sock;
    while (true) {
        Packet packettype;
        if (recv(Con, (char*)&packettype, sizeof(Packet), NULL) <= 0) break;

        if (packettype == Pack) {
            int msg_size;
            recv(Con, (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1];
            msg[msg_size] = 0;
            recv(Con, msg, msg_size, NULL);
            cout << msg << endl;
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

    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        cout << "Error: failed connect to server.\n";
        return 1;
    }

    cout << "Connected! Enter your name: ";
    string name;
    getline(cin, name);

    // Отправляем имя серверу первым делом
    int name_size = name.size();
    send(Connection, (char*)&name_size, sizeof(int), NULL);
    send(Connection, name.c_str(), name_size, NULL);

    CreateThread(NULL, NULL, ClientThread, &Connection, NULL, NULL);

    cout << "To send private msg use: @Name message" << endl;
    string msg;
    while (true) {
        getline(cin, msg);
        int msg_size = msg.size();
        Packet type = Pack;
        send(Connection, (char*)&type, sizeof(Packet), NULL);
        send(Connection, (char*)&msg_size, sizeof(int), NULL);
        send(Connection, msg.c_str(), msg_size, NULL);
    }
    return 0;
}