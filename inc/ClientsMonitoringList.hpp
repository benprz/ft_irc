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
		std::string current_command;
		std::vector<std::string> split_command;

		bool logged;
		bool registered;
		std::string nickname;
		std::string username;
		std::string realname;
		std::string hostname;
		std::string	modes;

		int opened_channels;

		ClientsMonitoringList(int fd);
		std::string get_prefix();
		int	is_operator();
		int	is_invisible();
};

#include "Server.hpp"

#endif