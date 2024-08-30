#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>

#include <vector>
#include <string>
#include <iostream>
#include <mutex>



#include "common.h"

struct SocketData {
    SOCKET socket;
    sockaddr_in socket_addr;
};

struct IoOperationData {
    static constexpr size_t SOCKET_BUFFER_SIZE = 60000;
    SocketData socket_data;
    WSABUF wsabuf;
    char buffer[SOCKET_BUFFER_SIZE];
    OVERLAPPED overalapped;
    unsigned long recieved_bytes;
    sockaddr_in from_addr;
    int from_len;
    DWORD flags;
};

class NetworkManager {
public:
    NetworkManager(int port_count, int start_port_number, std::mutex& mutex);
    bool ListenSockets();
    std::vector<int> GetOpenedSocketsPorts();
    void SetPortPairs(std::vector<std::pair<int, int>>* port_pairs);
    ~NetworkManager();
    void StopListenSockets();

private:
    bool CloseUdpSocket(SOCKET& socket);
    bool NetworkInit();
    bool PostAsyncRecieveOperations();
    bool CreateSocketPool(int port_count, int start_port_number);
    SOCKET CreateUdpSocket();
    bool BindSocket(SOCKET sock, sockaddr_in* addr);
    void ZeroIoOperationData(IoOperationData& io_op);
    bool ProcessIoOperation(IoOperationData& io_op, int recieved_bytes);
    void PrintMessage(IoOperationData& io_op, unsigned long recieved_bytes);

    static constexpr size_t kCountIoPacketsAsyncPick = 100;

    WSADATA wsa_data_{};
    HANDLE iocp_{};
    int ports_entries_count_{};
    std::vector<IoOperationData> io_ops_{};
    OVERLAPPED_ENTRY overlapped_entries_[kCountIoPacketsAsyncPick]{};
    std::vector<std::pair<int, int>>* port_pairs_{};
    std::vector<std::pair<int, int>> port_pairs_tmp_{};
    std::mutex& mutex_;
    bool update_loop_flag_{};
};

#endif // NETWORK_MANAGER_H