#include <iostream>
#include <sstream>
#include <string>
#include <bits/stdc++.h>
#define _WIN32_WINNT 0x501
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>

using std::cerr;
using namespace std;

string base_path;
char* enter_page = "pages\\enterpage.html";

string readFromFile(const char* f1)
{
    ifstream file(f1);
    string res = "";
    if (file.is_open())
    {
        string cur;
        while (getline(file, cur))
        {
            res += cur;
        }
    }
    else
        cout << "Can't open file" << endl;
    return res;
}

string convertToString(vector<char> ar)
{
    string res = "";
    for (int i = 0; i < ar.size(); i++)
    {
        if (ar[i] == '"')
            res += "\"";
        else
            res += ar[i];
    }
}

int main(int argc, char **argv)
{
    string c = argv[0];
    size_t idx = c.find(".");
    base_path = c.substr(0, idx);
    WSADATA wsaData; // для использования ws2_32.dll
    if (!WSAStartup(MAKEWORD(2, 2), &wsaData)) // MAKEWORD(2, 2) инициализирует версию WS 2.2
    {
        struct addrinfo* address = NULL;
        struct addrinfo hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
        int result = getaddrinfo("192.168.1.105", "80", &hints, &address);
        if (result != 0)
        {
            WSACleanup();
            return 1;
        }
        int listen_socket = socket(address->ai_family, address->ai_socktype,
                                   address->ai_protocol);
        if (listen_socket == INVALID_SOCKET)
        {
            freeaddrinfo(address);
            WSACleanup();
            return 1;
        }
        result = bind(listen_socket, address->ai_addr, (int)address->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            freeaddrinfo(address);
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }
        if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }
        const int max_client = 1024;
        char buf[max_client];
        int client_socket = INVALID_SOCKET;
        while (1)
        {
            client_socket = accept(listen_socket, NULL, NULL);
            if (client_socket == INVALID_SOCKET)
            {
                closesocket(listen_socket);
                WSACleanup();
                return 1;
            }
            result = recv(client_socket, buf, max_client, 0);
            string answer = "";
            for (int i = 0; i < sizeof(buf); i++)
                answer += buf[i];

            string cmd1 = answer.substr(0, answer.find("\r\n"));
            vector<string> tokens;
            string token;
            istringstream tstream(cmd1);
            while (getline(tstream, token, ' '))
                tokens.push_back(token);
            string url = tokens[1];
            stringstream response;
            stringstream response_body;

            if (result == SOCKET_ERROR)
                closesocket(client_socket);
            else if (result > 0)
            {
                buf[result];
                cout << url << endl;
                string first = readFromFile(enter_page);
                if (url == "/")
                    response_body << first;
                if (url.substr(0, 5) == "/send")
                {
                    cout << url.substr(6, url.length()-6) << endl;
                    response_body << url.substr(6, url.length()-6);
                }
                response << "HTTP/1.1 200 OK\r\n" << "Version: HTTP/1.1\r\n" << "Content-Type: text/html; charset=utf-8\r\n"
                         << "Content-Length: " << response_body.str().length() << "\r\n\r\n" << response_body.str();
                result = send(client_socket, response.str().c_str(),
                              response.str().length(), 0);
                if (result == SOCKET_ERROR)
                {
                    cerr << "Не удалось отправить ответ" << '\n';
                }
                closesocket(client_socket);
            }
        }
        closesocket(listen_socket);
        freeaddrinfo(address);
        WSACleanup();
    }
    return 0;
}
