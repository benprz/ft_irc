#ifndef CLIENTSMONITORINGLIST_HPP
#define CLIENTSMONITORINGLIST_HPP

#pragma once

#include <iostream>
#include <vector>

class ClientsMonitoringList
{
	public:
		int fd;
		std::string packet;
		std::vector<std::string> split_packet;

		bool logged;
		bool registered;
		std::string nickname;
		std::string username;
		std::string realname;
		std::string hostname;
		std::string	mode;

		int opened_channels;

		ClientsMonitoringList(int fd);
		int	is_operator();
};

#endif