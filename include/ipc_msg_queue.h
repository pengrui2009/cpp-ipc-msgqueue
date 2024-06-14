/**
 * @file ipc_msg_queue.h
 * @author rui.peng (pengrui_2009@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef IPC_MSG_QUEUE_H
#define IPC_MSG_QUEUE_H

#include <signal.h>
#include <string.h>

#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstddef>

#include "libipc/ipc.h"

namespace Ipc {

enum QueueMode : std::uint8_t {
    kSenderMode,
    kReceiverMode,
};

class IpcMsgQueue {
public:
    using msg_que_t = ipc::chan<ipc::relat::single, ipc::relat::multi, ipc::trans::broadcast>;

    explicit IpcMsgQueue(std::string name) : queue_name_(name)
    {
        const char *queue_name = queue_name_.data();
        queue_ptr_ = std::make_shared<msg_que_t>(queue_name);
    }

    IpcMsgQueue(char* name) : queue_name_(name)
    {
        const char *queue_name = queue_name_.data();
        queue_ptr_  = std::make_shared<msg_que_t>(queue_name);
    }

    ~IpcMsgQueue()
    {
        if (is_running_.load(std::memory_order_acquire))
        {
            queue_ptr_->disconnect();
        }

        is_running_.store(false, std::memory_order_release);
        is_quit_.store(false, std::memory_order_release);
    }

    int Inititalize(QueueMode mode)
    {
        int result = 0;

        if (nullptr == queue_ptr_)
        {
            std::cerr << __func__ << ": queue_ptr is nullptr.\n";
            return -1;
        }

        if (QueueMode::kSenderMode == mode) {
            if (!queue_ptr_->reconnect(ipc::sender)) {
                std::cerr << __func__ << ": connect failed.\n";
                result = -1;
            }
        } else {
            if (!queue_ptr_->reconnect(ipc::receiver)) {
                std::cerr << __func__ << ": connect failed.\n";
                result = -1;
            }
        }

        if (0 == result)
        {
            is_running_.store(true, std::memory_order_release);
        }

        return result;
    }

    int DeInititalize()
    {
        int result = 0;
        
        queue_ptr_->disconnect();

        is_running_.store(false, std::memory_order_release);
        
        is_quit_.store(true, std::memory_order_release);


        return result;
    }

    int Transmit(void *data_ptr, size_t data_len)
    {
        int result = 0;

        if (nullptr == data_ptr)
        {
            std::cerr << __func__ << ": queue_ptr is nullptr.\n";
            return -1;
        }

        if (!is_running_.load(std::memory_order_acquire))
        {
            std::cerr << __func__ << ": queue is not initialize.\n";
            return -1;
        }

        if (!queue_ptr_->send(ipc::buff_t(data_ptr, data_len))) {
            std::cerr << __func__ << ": send failed.\n";
            std::cout << __func__ << ": waiting for receiver...\n";
            if (!queue_ptr_->wait_for_recv(1)) {
                std::cerr << __func__ << ": wait receiver failed.\n";
                is_quit_.store(true, std::memory_order_release);
                result = -1;
            } else {
                if (!queue_ptr_->send(ipc::buff_t(data_ptr, data_len))) {
                    std::cerr << __func__ << ": send failed.\n";
                    result = -1;
                }
            }
        }

        return result;
    }

    int Receive(void *data_ptr, size_t data_size, size_t &data_len)
    {
        int result = 0;

        if (nullptr == data_ptr)
        {
            std::cerr << __func__ << ": queue_ptr is nullptr.\n";
            return -1;
        }

        if (!is_running_.load(std::memory_order_acquire))
        {
            std::cerr << __func__ << ": queue is not initialize.\n";
            return -1;
        }

        auto msg = queue_ptr_->recv();
        if (!is_quit_.load(std::memory_order_acquire))
        {
            if (!msg.empty()) 
            {
                data_len = data_size > msg.size() ? msg.size() : data_size;
                memcpy(data_ptr, msg.data(), data_len);
            } 
        } else {
            data_len = 0;
            result = -1;
        }
        
        return result;
    }
private:
    std::string queue_name_;
    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_quit_{false};
    std::shared_ptr<msg_que_t> queue_ptr_{nullptr};
};
}

#endif /* IPC_MSG_QUEUE_H */
