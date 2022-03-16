#ifndef CLIENT_HPP
#define CLIENTSMONITORINGLIST_HPP

#pragma once

#include <iostream>
#include <vector>
#include <poll.h>

#define MAX_CLIENT_CONNEXIONS 100

class ClientsMonitoringList
{
	private:
        bool _registered;
        std::string _nickname;

	public:
        std::string current_packet;

	ClientsMonitoringList() {};
	~ClientsMonitoringList() {};
};

#endif