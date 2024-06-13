#include "ipc_msg_queue.h"

std::shared_ptr<Ipc::IpcMsgQueue> msgqueue_ptr{std::make_shared<Ipc::IpcMsgQueue>("hello")};
std::atomic<bool> is_quit{false};
int main()
{
    

    auto exit = [](int) {
        is_quit.store(true, std::memory_order_release);
        msgqueue_ptr->DeInititalize();
    };
    ::signal(SIGINT  , exit);
    ::signal(SIGABRT , exit);
    ::signal(SIGSEGV , exit);
    ::signal(SIGTERM , exit);
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || \
    defined(WINCE) || defined(_WIN32_WCE)
    ::signal(SIGBREAK, exit);
#else
    ::signal(SIGHUP  , exit);
#endif
    // msgqueue_ptr = std::make_shared<Ipc::IpcMsgQueue>("hello");
    if (msgqueue_ptr->Inititalize(Ipc::QueueMode::kReceiverMode))
    {
        std::cerr << "msg queue Inititalize failed!\n" ;
        return -1;
    }

    while (!is_quit.load(std::memory_order_acquire))
    {
        std::array<uint8_t, 100> received_buffer = {0};
        size_t received_len = 0;
        if (msgqueue_ptr->Receive(received_buffer.data(), received_buffer.size(), received_len))
        {
            std::cerr << "msg queue Receive failed!\n" ;
        } else {
            auto now = std::chrono::system_clock::now();
            auto cur_timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count();
            
            uint64_t msg_timestamp = 0;
            memcpy(&msg_timestamp, received_buffer.data(), sizeof(msg_timestamp));
            std::cout << "msg timestamp:" << msg_timestamp << std::endl ;
            std::cout << "cur timestamp:" << cur_timestamp << std::endl ;
        }
    }
    
    return 0;
}

