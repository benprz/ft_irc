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
        COMMAND(USER)	\
        COMMAND(NAME)	\
        COMMAND(LIST)	\
        COMMAND(JOIN)	\
        COMMAND(OPER)	\
        COMMAND(PART)	\
        COMMAND(QUIT)	\
        COMMAND(SQUIT)	\
        COMMAND(MODE)	\
        COMMAND(PING)	\
        COMMAND(KICK)	\
        COMMAND(KILL)	\
		COMMAND(NB_COMMANDS)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_FUNCTION(FUNCTION) (void (ClientsMonitoringList::*))#FUNCTION,

enum COMMAND_ENUM {
    FOREACH_COMMAND(GENERATE_ENUM)
};

static const char *g_commands_name[] = {
    FOREACH_COMMAND(GENERATE_STRING)
};

class ClientsMonitoringList
{
	private:
        bool _registered;
        std::string _nickname;
        std::string _current_packet;

	public:
		//segfault pour des raisons inconnues, donc inutile de l'utiliser pour l'instant
		void (ClientsMonitoringList::*commands_functions[NB_COMMANDS])(std::vector<std::string>);

		ClientsMonitoringList()
		{
			commands_functions[0] = &ClientsMonitoringList::PASS;
			commands_functions[1] = &ClientsMonitoringList::NICK;
		};

		~ClientsMonitoringList() {};

		void	parse_client_packet(int client_fd, std::string packet);
		int		get_command_index(std::string command);
		void	do_command(std::string command, std::vector<std::string> split_packet);
		int		is_command(std::string command);

		// commands
		void	PASS(std::vector<std::string> split_packet);
		void	NICK(std::vector<std::string> split_packet);
};

#endif