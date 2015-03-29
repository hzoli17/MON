#include <cstddef>
#include <cstring>
#include <cstdlib>
#include "udp_server.h"
#define BUFLEN  512
bool initiated;
bool init()
{
#ifdef WINDOWS
    WSADATA wsaData = {0};
    int ret = 0;
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) return false;
#endif
    initiated=true;
    return true;
}
UDP_SERVER::Socket UDP_SERVER::createUDPServer(unsigned int port)
{
    UDP_SERVER::Socket sock;
    struct sockaddr_in s_server;
    if (!initiated)
    {
        if (!init()) return NULL;
    }
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) return NULL;
    memset((char*)&s_server, 0, sizeof(s_server));
    s_server.sin_family = AF_INET;
    s_server.sin_port = htons(port);
    s_server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr*)&s_server, sizeof(s_server)) == -1) return NULL;
    return sock;
}
bool UDP_SERVER::sendMSG(Socket sock, struct sockaddr_in client, char *buff, int buff_len)
{
    if (sendto(sock, buff, buff_len, 0, (struct sockaddr*)&client, sizeof(client)) == -1) return false;
    return true;
}
UDP_SERVER::packet UDP_SERVER::recvMSG(Socket sock)
{
    UDP_SERVER::packet pack;
    int slen = sizeof(pack.client);
    pack.data = calloc(sizeof(char), BUFLEN);
    if ((pack.lenght = recvfrom(sock, pack.data, BUFLEN, 0, (struct sockaddr *)&pack.client, &slen)) == -1) pack.data=NULL;
    return pack;
}
char * UDP_SERVER::getIP(sockaddr_in client)
{
    return inet_ntoa(client.sin_addr);
}
unsigned int UDP_SERVER::getPort(sockaddr_in client)
{
    return ntohs(client.sin_port);
}
UDP_SERVER::packet UDP_SERVER::getTarget(const char * addr, unsigned int port)
{
    UDP_SERVER::packet pack;
    struct sockaddr_in s;
    struct in_addr ia;
    inet_aton(addr, &ia);
    s.sin_family=AF_INET;
    s.sin_port=htons(port);
    s.sin_addr = ia;
    pack.client = s;
    return pack;
}

void UDP_SERVER::close(Socket sock)
{
    close(sock);
}
