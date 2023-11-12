#pragma once

#include <map>
#include "uintr.h"
#include <unistd.h>

class UINTRAII {
   private:
    int m_fd_uintr = -1;
    std::map<int, int> m_receiver_index_map;  // key: fd, value: index
   public:
    UINTRAII() : m_fd_uintr(-1) {}
    virtual ~UINTRAII() {
        if (m_fd_uintr >= 0) {
            ::close(m_fd_uintr);
            m_fd_uintr = -1;
        }
    }
    bool registerUintrHandler(void* handler, uint8_t is_enclave) {
        if (uintr_register_receiver(handler, is_enclave)) return false;
        return true;
    }
    /**
     * @brief Create a uintrfd for the sender. Before calling this function,
     * you should have called registerUintrHandler.
    */
    bool create(int sender_id) {
        m_fd_uintr = uintr_create_fd(sender_id);
        if (m_fd_uintr < 0) return false;
        return true;
    }
    /**
     * @brief Register a receiver for the sender
     */
    bool registerUintrReceiver(int fd_receiver) {
        int receiver_index = uintr_register_sender(fd_receiver);
        if (receiver_index < 0) return false;
        m_receiver_index_map[fd_receiver] = receiver_index;
        return true;
    }
    int getUintrFd() const { return m_fd_uintr; }
    int getReceiverIndex(int fd) const { return m_receiver_index_map.at(fd); }
};
