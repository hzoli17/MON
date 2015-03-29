#ifndef UDP_SERVER_H
#define UDP_SERVER_H
namespace UDP_SERVER
{
    #ifdef WINDOWS
    #include <Windows.h>
    #include <winsock2.h>
    typedef SOCKET  Socket;
    #else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    typedef int Socket;
    #endif
    typedef struct
    {
        struct sockaddr_in client;
        unsigned int lenght;
        char *  data;
    } packet;
    Socket createUDPServer(unsigned int port);
    bool   sendMSG(Socket sock, struct sockaddr_in client, char * buff, int buff_len);
    packet recvMSG(Socket sock);
    char * getIP(struct sockaddr_in client);
    unsigned int getPort(struct sockaddr_in client);
    packet getTarget(const char * addr, unsigned int port);
    void   close(Socket sock);
}

#endif // UDP_SERVER_H
