#ifndef CLIENTSMONITORINGLIST_HPP
#define CLIENTSMONITORINGLIST_HPP

#pragma once

#include <iostream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>

#define MAX_CLIENT_CONNEXIONS 100

#define FOREACH_COMMAND(COMMAND) \
        COMMAND(PASS)   \
        COMMAND(NICK)	\
		COMMAND(NB_COMMANDS)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum COMMAND_ENUM {
    FOREACH_COMMAND(GENERATE_ENUM)
};

static const char *g_commands_name[] = {
    FOREACH_COMMAND(GENERATE_STRING)
};

/*
void	(ClientsMonitoringList::*g_commands_functions[NB_COMMANDS])(std::vector<std::string>) = {
	ClientsMonitoringList::pass_command,
	ClientsMonitoringList::nick_command
};
*/

class ClientsMonitoringList
{
	private:
        bool _registered;
        std::string _nickname;
        std::string _current_packet;

	public:

		ClientsMonitoringList() {};
		~ClientsMonitoringList() {};

		void	parse_client_packet(int client_fd, std::string packet);
		void	nick_command(std::vector<std::string> split_packet);
		void	pass_command(std::vector<std::string> split_packet);
		void	do_command(std::string command, std::vector<std::string> split_packet);
		int		is_command(std::string command);
};

#endif