#ifndef CLIENTSMONITORINGLIST_HPP
#define CLIENTSMONITORINGLIST_HPP

#pragma once

#include <iostream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>

#define MAX_CLIENT_CONNEXIONS 100
#define RPL_WELCOME "001"
#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTRED "462"

#define CRLF "\r\n"

static const char *g_commands_name[] = {
	"PASS",
	"NICK",
	"USER",
	NULL
};

class ClientsMonitoringList
{
	private:
		int _fd;
        std::string _current_packet;

		bool _logged;
        bool _registered;
        std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _hostname;
		std::string	_mode;

	public:
	 	class ClientsMonitoringList *prev;
	 	class ClientsMonitoringList *next;

		std::string	getNickname() { return _nickname; };
		void		setFd(int fd) { _fd = fd; };
		~ClientsMonitoringList() {};

		void	parse_client_packet(std::string packet, const std::string password);
		int		get_command_index(std::string command);
		void	do_command(std::vector<std::string> split_packet, const std::string password);
		int		is_command(std::string command);
		void	send_message(std::string error);
		int		check_if_nickname_is_erroneous(std::string split_packet);
		int		check_if_nickname_is_already_used(std::string nickname);

		// commands
		void	PASS(const std::vector<std::string> split_packet, const std::string password);
		void	NICK(std::vector<std::string> split_packet);
		void	USER(std::vector<std::string> split_packet);
};

#endif