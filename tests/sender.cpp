#include "ipc_msg_queue.h"

#include <unistd.h>

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
    if (msgqueue_ptr->Inititalize(Ipc::QueueMode::kSenderMode))
    {
        std::cerr << "msg queue Inititalize failed!\n" ;
        return -1;
    }

    while (!is_quit.load(std::memory_order_acquire))
    {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();

        std::array<uint8_t, 100> send_buffer = {0};
        memcpy(send_buffer.data(), &timestamp, sizeof(timestamp));
        if (msgqueue_ptr->Transmit(send_buffer.data(), send_buffer.size()))
        {
            std::cerr << "msg queue Transmit failed!\n" ;
        } else {
            std::cout << "msg queue Transmit success" << std::endl ;
        }
        sleep(1);
    }
    
    return 0;
}

