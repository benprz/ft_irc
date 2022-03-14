#ifndef CLIENT_HPP
#define CLIENT_HPP

#pragma once

#include <iostream>
#include <vector>
#include <poll.h>

#define MAX_CLIENT_CONNEXIONS 100

class ClientsMonitoringList : public pollfd
{
    private:
        bool _registered;
        std::string _nickname;

    public:
        std::string current_packet;

        ClientsMonitoringList() : _registered(0), _nickname("") {};
        void reset()
        {
            fd = -1;
            events = 0;
            revents = 0;

            _registered = 0;
            _nickname = "";
        };
        ~ClientsMonitoringList() {};
};

#endif