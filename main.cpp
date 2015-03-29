#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#include <climits>
#include <vector>
#include <map>
#include "udp_server.h"
#include "thread.h"
#include "MON.h"
#include "OS.h"
#define SOFTWARE    "MON Server"
#define VERSION     "2.0"
#define COPYRIGHT   "Herczeg Zoltán 2015"
using namespace std;
int serverPort = 1998;
int timeout = 5;
string banner;
typedef struct
{
string addr;
unsigned int port;
string nick;
string deviceName;
time_t usedConnect;
} client;
typedef struct
{
    unsigned int id;
    UDP_SERVER::Socket sock;
    UDP_SERVER::packet pack;
} clientSock;
typedef struct
{
    char * buff;
    int length;
} MON_MSG;
vector<client> clients;
map<unsigned int, clientSock> csm;
bool isConnected(string addr, unsigned int port)
{
    for (vector<client>::iterator it = clients.begin();it!=clients.end();++it)
    {
        if (((client)(*it)).addr == addr && ((client)(*it)).port == port) return true;
    }
    return false;
}
bool addClient(string addr, unsigned int port)
{
    client c;
    if (isConnected(addr, port)) return false;
    c.addr=addr;
    c.port=port;
    c.deviceName="?";
    c.nick=addr;
    time(&c.usedConnect);
    clients.push_back(c);
    return true;
}
bool delClient(string addr, unsigned int port)
{
    for (vector<client>::iterator it = clients.begin();it!=clients.end();++it)
    {
        if (((client)(*it)).addr == addr && ((client)(*it)).port == port)
        {
            clients.erase(it);
            return true;
        }
    }
    return false;
}
bool setDeviceName(string addr, unsigned int port, string name)
{
    for (vector<client>::iterator it = clients.begin();it!=clients.end();++it)
    {
        if (((client)(*it)).addr == addr && ((client)(*it)).port == port)
        {
            (*it).deviceName = name;
            return true;
        }
    }
    return false;
}
MON_MSG getMSG(char status, char * data, int len)
{
    MON_MSG m;
    m.length=sizeof(char)+len;
    m.buff=calloc(sizeof(char), m.length);
    m.buff[0]=status;
    memcpy(m.buff+1, data, len);
    return m;
}
bool sendMON(UDP_SERVER::Socket socket, UDP_SERVER::packet dest, MON_MSG msg)
{
    return UDP_SERVER::sendMSG(socket, dest.client, msg.buff, msg.length);
}
bool broadcastMON(UDP_SERVER::Socket socket, string exceptionAddr, unsigned int exceptionPort, MON_MSG msg)
{
    for (vector<client>::iterator it=clients.begin();it!=clients.end();++it)
    {
        if (((client)(*it)).addr != exceptionAddr && ((client)(*it)).port != exceptionPort)
        {
            if (!sendMON(socket, UDP_SERVER::getTarget((*it).addr.c_str(), (*it).port), msg)) return false;
        }
    }
    return true;
}
bool refresh(string addr, unsigned int port)
{
    for (vector<client>::iterator it = clients.begin();it!=clients.end();++it)
    {
        if (((client)(*it)).addr == addr && ((client)(*it)).port == port)
        {
            time(&(*it).usedConnect);
            return true;
        }
    }
    return false;
}
void check()
{
    time_t now, ut;
    while (1)
    {
    again: time(&now);
    for (vector<client>::iterator it=clients.begin();it!=clients.end();++it)
    {
        ut = (*it).usedConnect;
        if (now - ut > 60 * timeout)
        {
            cout<<"Disconnected client (timeout): "<<((client)(*it)).addr<<":"<<dec<<(int)((client)(*it)).port<<endl;
            clients.erase(it);
            goto again;
        }
    }
    Thread::sleep(1000000);
    }
}
void acceptMON(unsigned int id)
{
    UDP_SERVER::Socket sock;
    UDP_SERVER::packet pack;
    MON_MSG data;
    sock=csm[id].sock;
    pack=csm[id].pack;
    int cnum;
    if (isConnected(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client)))
    {
        data.buff = pack.data;
        data.length = pack.lenght;
        refresh(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client));
        switch (pack.data[0])
        {
        case STATUS_OK:
            sendMON(sock, pack, getMSG(STATUS_OK, "Hey man, I see you have a good day! :D", sizeof("Hey man, I see you have a good day! :D")));
            break;
        case STATUS_CLOSE:
            delClient(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client));
            break;
        case STATUS_MIDIMSG:
        case STATUS_SYSEXMSG:
            broadcastMON(sock, UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client), data);
            break;
        case STATUS_GETINFO:
            sendMON(sock, pack, getMSG(STATUS_SENDHELO, banner.c_str(), banner.length()));
            break;
        case STATUS_WARNING:
            cout<<"Received STATUS_WARNING from "<<UDP_SERVER::getIP(pack.client)<<":"<<dec<<(int)UDP_SERVER::getPort(pack.client)<<": "<<string(pack.data).substr(1)<<endl;
            break;
        case STATUS_ERROR:
            cout<<"Received STATUS_ERROR from "<<UDP_SERVER::getIP(pack.client)<<":"<<dec<<(int)UDP_SERVER::getPort(pack.client)<<": "<<string(pack.data).substr(1)<<endl;
            break;
        case STATUS_RESET:
            delClient(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client));
            addClient(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client));
            sendMON(sock, pack, getMSG(STATUS_GETINFO, "", 0));
            break;
        case STATUS_SENDHELO:
            setDeviceName(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client), string(pack.data).substr(1));
            cout<<"Set name for "<<UDP_SERVER::getIP(pack.client)<<":"<<dec<<(int)UDP_SERVER::getPort(pack.client)<<": "<<string(pack.data).substr(1)<<endl;
            break;
        case STATUS_GETNUMCL:
            sendMON(sock, pack, getMSG(STATUS_GETNUMCL, (char*)clients.size(), sizeof(int)));
            break;
        case STATUS_GETCLNM:
            cnum = (int)(pack.data+1);
            if (cnum>-1 && clients.size()>cnum)
            {
                sendMON(sock, pack, getMSG(STATUS_GETCLNM, clients.at(cnum).deviceName.c_str(), clients.at(cnum).deviceName.length()));
            }
            else
            {
                sendMON(sock, pack, getMSG(STATUS_ERROR, "Client not found!", sizeof("Client not found!")));
            }
            break;
        case STATUS_GETCLIP:
            cnum = (int)(pack.data+1);
            if (cnum>-1 && clients.size()>cnum)
            {
                sendMON(sock, pack, getMSG(STATUS_GETCLIP, clients.at(cnum).addr.c_str(), clients.at(cnum).addr.length()));
            }
            else
            {
                sendMON(sock, pack, getMSG(STATUS_ERROR, "Client not found!", sizeof("Client not found!")));
            }
            break;
        case STATUS_DEMO:
                sendMON(sock, pack, getMSG(STATUS_DEMO, "Picky people pick Peter Pan Peanut-Butter, 'tis the peanut-butter picky people pick.", sizeof("Picky people pick Peter Pan Peanut-Butter, 'tis the peanut-butter picky people pick.")));
            break;
        default:
        sendMON(sock, pack, getMSG(STATUS_ERROR, "?", 1));
        cout<<"WARNING! Get unimplemented status code (0x"<<hex<<(int)pack.data[0]<<") command from: "<<UDP_SERVER::getIP(pack.client)<<":"<<dec<<(int)UDP_SERVER::getPort(pack.client)<<endl;
        break;
        }
    }
    else
    {
        if (strcmp(pack.data, HEADER) == 0)
        {
        sendMON(sock, pack, getMSG(STATUS_OK, "", 0));
        addClient(UDP_SERVER::getIP(pack.client), UDP_SERVER::getPort(pack.client));
        cout<<"Client connected: "<<UDP_SERVER::getIP(pack.client)<<":"<<dec<<(int)UDP_SERVER::getPort(pack.client)<<endl;
        }
        else
        {
        sendMON(sock, pack, getMSG(STATUS_ERROR, "", 0));
        cout<<"Get bad packets from: "<<UDP_SERVER::getIP(pack.client)<<":"<<dec<<(int)UDP_SERVER::getPort(pack.client)<<endl;
        }
    }
    csm.erase(csm.find(id));
}
int main(int argc, char * argv[])
{
    UDP_SERVER::Socket sock;
    UDP_SERVER::packet pack;
    clientSock  c;
    stringstream sstr;
    unsigned int i=0;
    sstr<<SOFTWARE<<"/"<<VERSION<<"("<<PLATFORM<<")";
    banner=sstr.str();
    cout<<banner<<endl
        <<COPYRIGHT<<endl<<endl;
    if (argc>1)
    {
        for (int i=1;i<argc;i++)
        {
            if (strcmp(argv[i], "--port") == 0)
            {
                if (argc>i+1)
                {
                    serverPort = atoi(argv[i+1]);
                    i++;
                }
                else
                {
                    cout<<"--port required more arguments!"<<endl;
                    return 1;
                }
            }
            else
            {
                if (strcmp(argv[i], "--banner") == 0)
                {
                    if (argc>i+1)
                    {
                        banner = string(argv[i+1]);
                        i++;
                    }
                    else
                    {
                        cout<<"--banner required more arguments!"<<endl;
                        return 1;
                    }
                }
                else
                {
                cout<<"Available commands:"<<endl<<endl
                    <<"--help       \tThis help"<<endl
                    <<"--port number\tServer listen port"<<endl
                    <<"--banner text\tServer welcome banner text"<<endl
                    <<endl<<endl;
                return 1;
                }
            }
        }
    }
    cout<<"Start server in "<<dec<<serverPort<<"...";
    sock = UDP_SERVER::createUDPServer(serverPort);
    if (sock == NULL)
    {
        cout<<"[ERR]"<<endl;
        perror("createUDPServer()");
        return 1;
    }
    cout<<"[OK]"<<endl;
    c.sock=sock;
    Thread::create(check, NULL);
    while (1)
    {
        pack = UDP_SERVER::recvMSG(sock);
        if (pack.data == NULL)
        {
            perror("recvMSG error");
        }
        else
        {
            c.id=i;
            c.pack=pack;
            csm[i]=c;
            Thread::create(acceptMON, (void*)i);
            if (i<UINT_MAX) i++; else i=0;
        }
    }
    return 0;
}
