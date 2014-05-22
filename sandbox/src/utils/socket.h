#ifndef SOCKET_H
#define SOCKET_H

#include <WinSock2.h>
#include <stdint.h>
#include <cstdio>
#include <cassert>
#include <queue>

#undef near
#undef far

class IPAddr
{
    uint32_t mAddress;
    uint16_t mPort;

    template<typename T> bool parse(const T* str, const T* format, int (*sscanf)(const T*, const T*, ...), bool hasPort)
    {
        char buffer[512];
        int read = sscanf(str, format, buffer, &mPort);

        mAddress = inet_addr(buffer);
        if (mAddress == INADDR_NONE)
        {
            hostent* host = gethostbyname(buffer);
            uint32_t wsaddr = *(uint32_t*)(host->h_addr_list[0]);

            if (wsaddr == INADDR_NONE)
            {
                fprintf(stderr, "invalid address: %s\n", buffer);
                return false;
            }
        }

        if (hasPort && mPort > 65535)
        {
            fprintf(stderr, "invalid port: %u\n", mPort);
            return false;
        }

        return true;
    }
public:
    IPAddr();
    IPAddr(const char* addr);
    IPAddr(const wchar_t* addr);
    IPAddr(const char* addr, uint16_t port);
    IPAddr(const wchar_t* addr, uint16_t port);

    const char* getIPString() const;
    const char* getFullString() const;
    uint16_t getPort() const;
};

IPAddr::IPAddr()
{
    memset(this, 0, sizeof(*this));
}

IPAddr::IPAddr(const char* addr)
{
    if (!parse(addr, "%[^:]:%u", sscanf, true))
        memset(this, 0, sizeof(*this));
}

IPAddr::IPAddr(const wchar_t* addr)
{
    if (!parse(addr, L"%[^:]:%u", swscanf, true))
        memset(this, 0, sizeof(*this));
}

IPAddr::IPAddr(const char* addr, uint16_t port)
{
    if (parse(addr, "%s", sscanf, false))
        mPort = port;
    else
        memset(this, 0, sizeof(*this));
}

IPAddr::IPAddr(const wchar_t* addr, uint16_t port)
{
    if (parse(addr, L"%s", swscanf, false))
        mPort = port;
    else
        memset(this, 0, sizeof(*this));
}

const char* IPAddr::getIPString() const
{
    return inet_ntoa(*(in_addr*)&mAddress);
}

const char* IPAddr::getFullString() const
{
    static char buffer[22]; // 3*255., 1*255:, 1*65536, 1*zero
    sprintf(buffer, "%s:%u", inet_ntoa(*(in_addr*)&mAddress), (uint32_t)mPort);
    return buffer;
}

uint16_t IPAddr::getPort() const
{
    return mPort;
}

class Socket
{
protected:
    static bool mWSAInitialised;
    SOCKET mSock;
    IPAddr mAddress;
    bool mBlocking;

    static void handleError(const char* from = NULL, int error = 0);
    Socket();
    ~Socket();
public:
    enum AddressFamily {
        IPv4 = AF_INET,
        IPv6 = AF_INET6
    };

    enum Type {
        Stream = SOCK_STREAM,
        Datagram = SOCK_DGRAM,
        Raw = SOCK_RAW
    };

    enum Protocol {
        ICMP = IPPROTO_ICMP,
        IGMP = IPPROTO_IGMP,
        TCP = IPPROTO_TCP,
        UDP = IPPROTO_UDP,
        ICMPv6 = IPPROTO_ICMPV6
    };

    uint32_t read(void* buf, uint32_t bufSize);
    bool write(const void* buf, uint32_t bufSize);

    const IPAddr& getAddress();
    bool setBlocking(bool blocking);
};

class ServerSocket;

class ClientSocket: public Socket
{
private:
public:
    bool connect(const IPAddr& address, Protocol proto = TCP, AddressFamily af = IPv4, Type type = Stream);
    bool close();
};

class AcceptSocket: public Socket
{
private:
    ServerSocket* mParent;

    AcceptSocket(SOCKET sock, const IPAddr& address, ServerSocket* parent);
    ~AcceptSocket() {}
public:

    bool isValid();
    bool close();

friend class ServerSocket;
};

class ServerSocket: public Socket
{
private:
    AcceptSocket* mClients;
    uint32_t mMaxClients;

    HANDLE mMutex;
    char mMutexName[16];

    HANDLE mListenThread;
    std::queue<AcceptSocket*> mAccepted;

    static unsigned int __stdcall listenThreadFunc(void* thisPtr);
    void resetThread();
    void onClientClose(AcceptSocket* socket);
    void accept(SOCKET sock, const IPAddr& address);
public:
    ServerSocket();
    ~ServerSocket();

    bool listen(uint16_t port, uint32_t maxConnections, Protocol proto = TCP, AddressFamily af = IPv4, Type type = Stream);
    bool close();
    bool clientsPending();
    AcceptSocket* getNewClient();

friend bool AcceptSocket::close();
};


bool Socket::mWSAInitialised;

void Socket::handleError(const char* from, int error)
{
    char* message = NULL;
    if (error == 0)
        error = ::WSAGetLastError();
    int size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, NULL, (LPSTR)&message, 0, NULL);
    fprintf(stderr, "error %u (0x%08x): \"%s\" in %s\n", msg, msg, message, from ? from : "undefined");
    ::LocalFree(message);
}


Socket::Socket():
    mSock(0),
    mBlocking(true);
{
    if (!mWSAInitialised)
    {
        WSADATA wsaData;
        ::WSAStartup(MAKEWORD(2, 2), &wsaData);

        mWSAInitialised = true;
    }
}

Socket::~Socket()
{
}

uint32_t Socket::read(void* buf, uint32_t bufSize)
{
    if (!mSock)
        return 0;

    int ret = ::recv(mSock, (char*)buf, (int)bufSize, 0);
    if (ret == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK)
            return 0;

        handleError("Socket::Read", error);
    }

    return (ret >= 0 ? ret : 0);
}

bool Socket::write(const void* buf, uint32_t bufSize)
{
    if (!mSock)
        return false;

    int ret = ::send(mSock, (const char*)buf, (int)bufSize, 0);
    if (ret == SOCKET_ERROR)
        handleError("Socket::Write");

    return ret == bufSize;
}

const IPAddr& Socket::getAddress()
{
    return mAddress;
}

bool Socket::setBlocking(bool blocking)
{
    mBlocking = blocking;

    if (mSock)
    {
        u_long block = (int)blocking;
        if (::ioctlsocket(mSock, FIOASYNC, &block) == SOCKET_ERROR)
        {
            HandleError("Socket::setBlocking");
            return false;
        }
    }

    return true;
}

// ClientSocket ------------------------------------------------------------ //

bool ClientSocket::connect(const IPAddr& address, Protocol proto, AddressFamily af, Type type)
{
    if (mSock)
        close();

    mSock = ::socket((int)af, (int)type, (int)proto);
    if (mSock == INVALID_SOCKET)
        handleError("ClientSocket::Connect@socket");
    else
    {
        // set async mode if needed
        setBlocking(mBlocking);

        switch (af)
        {
        case IPv4:
            {
                sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = af;
                addr.sin_port = htons(address.getPort());
                fprintf(stderr, "using address %s\n", address.getIPString());
                addr.sin_addr.s_addr = inet_addr(address.getIPString());

                int ret = ::connect(mSock, (const sockaddr*)&addr, sizeof(addr));
                if (ret == SOCKET_ERROR)
                    handleError("ClientSocket::connect@connect");
                else
                {
                    mAddress = address;
                    fprintf(stderr, "connected to %s\n", mAddress.getFullString());
                }

                break;
            }
        case IPv6:
            {
                assert(!"not implemented");
                break;
            }
        }
    }

    return mSock != INVALID_SOCKET;
}

bool ClientSocket::close()
{
    int ret = ::closesocket(mSock);
    if (ret == SOCKET_ERROR)
        handleError("ClientSocket::close");
    else
        mSock = 0;

    mAddress = IPAddr();

    return mSock == 0;
}

// AcceptSocket ------------------------------------------------------------ //

AcceptSocket::AcceptSocket(SOCKET sock, const IPAddr& address, ServerSocket* parent):
    Socket(),
    mParent(parent)
{
    mSock = sock;
    mAddress = address;

    fprintf(stderr, "AcceptSocket: %s\n", mAddress.getIPString());
}

AcceptSocket::~AcceptSocket()
{
    close();
}

bool AcceptSocket::isValid()
{
    return mSocket != NULL;
}

bool AcceptSocket::close()
{
    int ret = ::closesocket(mSock);
    if (ret == SOCKET_ERROR)
        handleError("AcceptSocket::close");
    else
        mSock = 0;

    mParent->onClientClose(this);
    mAddress = IPAddr();

    return mSock == 0;
}

// ServerSocket ------------------------------------------------------------ //

unsigned int ServerSocket::listenThreadFunc(void* thisPtr)
{
    ServerSocket* _this = (ServerSocket*) thisPtr;

    while (true)
    {
        printf("listening...\n");

        sockaddr_in addr;
        int addrSize = sizeof(addr);

        SOCKET ret = ::accept(_this->mSock, (sockaddr*)&addr, &addrSize);
        if (ret == INVALID_SOCKET)
        {
            int error = ::WSAGetLastError();

            if (error == WSAEWOULDBLOCK)
                ::Sleep(0);
            else
                handleError("ServerSocket::ListenThreadFunc", error);
        }
        else
            _this->accept(ret, IPAddr(inet_ntoa(addr.sin_addr), addr.sin_port));
    }

    return 0;
}

void ServerSocket::onClientClose(AcceptSocket* sock)
{
    sock->~AcceptSocket();
}

void ServerSocket::accept(SOCKET sock, const IPAddr& address)
{
    AcceptSocket* socket = mClients;
    while (socket->isValid() && socket < &mClients[mMaxClients])
        ++socket;

    if (socket < &mClients[mMaxClients])
    {
        socket = new(socket) AcceptSocket(sock, address, this);

        HANDLE mutex = ::OpenMutexA(MUTEX_MODIFY_STATE, FALSE, mMutexName);
        {
            mAccepted.push(socket);
        }
        ::ReleaseMutex(mutex);
    }
    else
    {
        fprintf(stderr, "something went terribly wrong, this shouldn't be accepted");
        acceptSocket(sock, address, this).close();
    }
}

void ServerSocket::resetThread()
{
    if (mListenThread)
    {
        ::TerminateThread(mListenThread, 0);
        ::CloseHandle(mListenThread);
        mListenThread = 0;
    }
    if (mMutex)
    {
        ::CloseHandle(mMutex);
        mMutex = 0;
    }

}

ServerSocket::ServerSocket():
    mClients(NULL),
    mMaxClients(0),
    mMutex(NULL),
    mListenThread(NULL)
{
    for (uint32_t i = 0; i < 15; ++i)
        mMutexName[i] = (char)(rand() % 24) + 'a';
    mMutexName[15] = '\0';

    fprintf(stderr, "mutex name: %s\n", mMutexName);
}

ServerSocket::~ServerSocket()
{
    Close();
}

bool ServerSocket::close()
{
    resetThread();

    if (mClients)
    {
        for (uint32_t i = 0; i < mMaxClients; ++i)
            if (mClients[i].isValid())
                mClients[i].close();

        free(mClients);
        mClients = NULL;
        mMaxClients = 0;
    }

    int ret = ::closesocket(mSock);
    if (ret == SOCKET_ERROR)
        handleError("ServerSocket::Close");
    else
        mSock = 0;

    return mSock == 0;
}

bool ServerSocket::listen(uint16_t port, uint32_t maxClients, Protocol proto, AddressFamily af, Type type)
{
    if (mClients)
        return false;

    if (!mSock)
    {
        mSock = ::socket(af, type, proto);
        if (mSock == INVALID_SOCKET)
        {
            handleError("ServerSocket::listen@socket");
            return false;
        }
    }

    mClients = (AcceptSocket*)::calloc(maxClients, sizeof(AcceptSocket));
    mMaxClients = maxClients;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = af;
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addr.sin_port = port;

    int ret = ::bind(mSock, (const sockaddr*)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
        handleError("ServerSocket::listen@bind");

    ret = ::listen(mSock, maxClients);
    if (ret == SOCKET_ERROR)
        handleError("ServerSocket::listen@listen");

    resetThread();
    mListenThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ServerSocket::listenThreadFunc, this, NULL, NULL);
    mMutex = ::CreateMutexA(NULL, FALSE, mMutexName);

    return ret != SOCKET_ERROR;
}

bool ServerSocket::clientsPending()
{
    return mAccepted.size() > 0;
}

AcceptSocket* ServerSocket::getNewClient()
{
    AcceptSocket* ret = mAccepted.front();

    HANDLE mutex = ::OpenMutexA(MUTEX_MODIFY_STATE, FALSE, mMutexName);
    {
        mAccepted.pop();
    }
    ::ReleaseMutex(mutex);

    return ret;
}

#endif //SOCKET_H
