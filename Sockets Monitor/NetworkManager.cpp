#include "NetworkManager.h"


#pragma comment(lib, "ws2_32.lib")




NetworkManager::NetworkManager(int port_count, int start_port_number, std::mutex& mutex) :
    ports_entries_count_(port_count),
    mutex_(mutex)
{
    NetworkInit();
    CreateSocketPool(port_count, start_port_number);
}

bool NetworkManager::ListenSockets() {
    PostAsyncRecieveOperations();
    OVERLAPPED_ENTRY* overlapped_entries_ptr = overlapped_entries_;
    update_loop_flag_ = 1;
    while (update_loop_flag_) {
        unsigned long iops_count{};
        auto start_time = std::chrono::high_resolution_clock::now();
        while (update_loop_flag_ && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() < 20) {
            auto result = GetQueuedCompletionStatusEx(iocp_, overlapped_entries_ptr, kCountIoPacketsAsyncPick, &iops_count, 0, 0);
            if (!result) {
                if (GetLastError() == WAIT_TIMEOUT) {
                    continue;
                }
                else {
                    PRINT_LAST_ERROR(L"GetQueuedCompletionStatusEx loop without waiting failed");
                    return 1;
                }
            }
            if (iops_count) {
                for (size_t i = 0; i < iops_count; i++) {
                    ProcessIoOperation(io_ops_[overlapped_entries_[i].lpCompletionKey], overlapped_entries_[i].dwNumberOfBytesTransferred);
                }
                start_time = std::chrono::high_resolution_clock::now();
            }
        }

        if (!GetQueuedCompletionStatusEx(iocp_, overlapped_entries_ptr, kCountIoPacketsAsyncPick, &iops_count, INFINITE, 0)) {
            PRINT_LAST_ERROR(L"GetQueuedCompletionStatusEx infinite waiting failed");
            return 1;
        }
        if (iops_count) {
            for (size_t i = 0; i < iops_count; i++) {
                ProcessIoOperation(io_ops_[overlapped_entries_[i].lpCompletionKey], overlapped_entries_[i].dwNumberOfBytesTransferred);
            }
        }
    }
    return 0;
}

void NetworkManager::StopListenSockets() {
    update_loop_flag_ = 0;
}

std::vector<int> NetworkManager::GetOpenedSocketsPorts() {
    std::vector<int> result{};
    for (auto const& io_op : io_ops_) {
        result.push_back(static_cast<int>(ntohs(io_op.socket_data.socket_addr.sin_port)));
    }
    return result;
}

void NetworkManager::SetPortPairs(std::vector<std::pair<int, int>>* port_pairs) {
    port_pairs_ = port_pairs;
}

bool NetworkManager::NetworkInit() {
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data_)) {
        PRINT_LAST_ERROR(L"WSAStartup failed");
        return 1;
    };

    iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (!iocp_) {
        PRINT_LAST_ERROR(L"IOCP create failed");
        return 1;
    }
    return 0;
}

bool NetworkManager::PostAsyncRecieveOperations() {
    for (int i = 0; i < ports_entries_count_; i++) {
        auto result = WSARecvFrom(
            io_ops_[i].socket_data.socket,
            &io_ops_[i].wsabuf,
            1,
            &io_ops_[i].recieved_bytes,
            &io_ops_[i].flags,
            reinterpret_cast<sockaddr*>(&io_ops_[i].from_addr),
            &io_ops_[i].from_len,
            &io_ops_[i].overalapped,
            0
        );
        if (result != 0 && WSAGetLastError() != WSA_IO_PENDING) {
            PRINT_WSA_LAST_ERROR(L"WSARecvFrom start failed");
            return 1;
        }
    }
    return 0;
}

bool NetworkManager::CreateSocketPool(int port_count, int start_port_number) {
    io_ops_ = std::vector<IoOperationData>(port_count, IoOperationData{});
    for (int i = 0, port = start_port_number, count = port_count; i < count; i++) {
        if (io_ops_[i].socket_data.socket == 0) {
            io_ops_[i].socket_data.socket_addr.sin_family = AF_INET;
            io_ops_[i].socket_data.socket_addr.sin_addr.S_un.S_addr = 0;
            io_ops_[i].socket_data.socket_addr.sin_port = htons(port++);

            io_ops_[i].socket_data.socket = CreateUdpSocket();
            if (io_ops_[i].socket_data.socket == INVALID_SOCKET) {
                return 1;
            }
            u_long mode = 1;
            if (ioctlsocket(io_ops_[i].socket_data.socket, FIONBIO, &mode) == SOCKET_ERROR) {
                PRINT_LAST_ERROR(L"Set socket non-blocking mode failed");
                return 1;
            }
        }
        else {
            io_ops_[i].socket_data.socket_addr.sin_port = htons(port++);
        }
        io_ops_[i].wsabuf.buf = io_ops_[i].buffer;
        io_ops_[i].wsabuf.len = sizeof(io_ops_[i].buffer);
        io_ops_[i].from_len = sizeof(sockaddr);

        if (BindSocket(io_ops_[i].socket_data.socket, &io_ops_[i].socket_data.socket_addr)) {
            PRINT_LAST_ERROR(L"Port already in use");
            i--;
            continue;
        }
        if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(io_ops_[i].socket_data.socket), iocp_, i, 0)) {
            PRINT_LAST_ERROR(L"Bind socket to IOCP failed");
            return 1;
        }
    }
    return 0;
}

SOCKET NetworkManager::CreateUdpSocket() {
    SOCKET socket = WSASocketW(AF_INET, SOCK_DGRAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
    if (socket == INVALID_SOCKET) {
        PRINT_WSA_LAST_ERROR(L"Socket create failed");
        return INVALID_SOCKET;
    }
    return socket;
}

bool NetworkManager::CloseUdpSocket(SOCKET& socket) {
    if (socket != INVALID_SOCKET) {
        auto result = closesocket(socket);
        if (result == SOCKET_ERROR) {
            PRINT_WSA_LAST_ERROR(L"Socket close failed");
            return 1;
        }
        socket = 0;
    }
    return 0;
}

bool NetworkManager::BindSocket(SOCKET sock, sockaddr_in* addr) {
    if (bind(sock, reinterpret_cast<sockaddr*>(addr), sizeof(sockaddr))) {
        PRINT_WSA_LAST_ERROR(L"Socket bind failed");
        return 1;
    }
    return 0;
}

void NetworkManager::ZeroIoOperationData(IoOperationData& io_op) {
    ZeroMemory(io_op.buffer, sizeof(io_op.buffer));
    io_op.flags = 0;
    io_op.from_addr = sockaddr_in{};
    io_op.from_len = sizeof(sockaddr_in);
    io_op.recieved_bytes = 0;
    io_op.overalapped = OVERLAPPED{};
}

bool NetworkManager::ProcessIoOperation(IoOperationData& io_op, int recieved_bytes) {
    port_pairs_tmp_.push_back({
        static_cast<int>(ntohs(io_op.socket_data.socket_addr.sin_port)),
        static_cast<int>(ntohs(io_op.from_addr.sin_port))
        });

    for (int i = 0; i < 100; i++) {
        auto result = WSARecvFrom(
            io_op.socket_data.socket,
            &io_op.wsabuf,
            1,
            &io_op.recieved_bytes,
            &io_op.flags,
            reinterpret_cast<sockaddr*>(&io_op.from_addr),
            &io_op.from_len,
            0,
            0
        );
        if (result == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                break;
            }
            else {
                PRINT_WSA_LAST_ERROR(L"Get message from socket buffer");
                    return 1;
            }
        }
        else {
            port_pairs_tmp_.push_back({
                static_cast<int>(ntohs(io_op.socket_data.socket_addr.sin_port)),
                static_cast<int>(ntohs(io_op.from_addr.sin_port))
                });
            ZeroIoOperationData(io_op);
        }
    }

    {
        std::lock_guard<std::mutex> lg(mutex_);
        for (auto& port_pair : port_pairs_tmp_) {
            (*port_pairs_).push_back(port_pair);
        }
    }
    port_pairs_tmp_.clear();

    ZeroIoOperationData(io_op);
    auto result = WSARecvFrom(
        io_op.socket_data.socket,
        &io_op.wsabuf,
        1,
        &io_op.recieved_bytes,
        &io_op.flags,
        reinterpret_cast<sockaddr*>(&io_op.from_addr),
        &io_op.from_len,
        &io_op.overalapped,
        0
    );
    if (result != 0 && WSAGetLastError() != WSA_IO_PENDING) {
        PRINT_WSA_LAST_ERROR(L"WSARecvFrom post async IO failed");
        return 1;
    }
    return 0;
}

void NetworkManager::PrintMessage(IoOperationData& iop, unsigned long recievedBytes) {
    wchar_t from_addr[INET_ADDRSTRLEN];
    InetNtopW(AF_INET, &(iop.from_addr.sin_addr), from_addr, INET_ADDRSTRLEN);

    std::wstring output =
        std::wstring(L"Message recieved from ") +
        from_addr +
        L":" +
        std::to_wstring(ntohs(iop.from_addr.sin_port)) +
        L" on port " +
        std::to_wstring(ntohs(iop.socket_data.socket_addr.sin_port)) +
        L": ";

    if (recievedBytes < 80) {
        output += std::wstring(reinterpret_cast<wchar_t*>(iop.buffer), recievedBytes / sizeof(wchar_t));
    }
    else {
        output += L"message size too long (" + std::to_wstring(recievedBytes) + L")";
    }
    output += L'\n';
    std::wcout << output;
}

NetworkManager::~NetworkManager() {
    for (auto& io_op : io_ops_) {
        CloseUdpSocket(io_op.socket_data.socket);
        CloseHandle(iocp_);
    }
}




