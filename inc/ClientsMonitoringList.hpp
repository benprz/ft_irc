#ifndef CLIENTSMONITORINGLIST_HPP
#define CLIENTSMONITORINGLIST_HPP

#pragma once

#include <iostream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>

#define MAX_CLIENT_CONNEXIONS 100
#define OPER_HOST 1
#define OPER_PASSWD "oper123"

#define RPL_WELCOME "001"
#define RPL_YOUREOPER "381"

#define ERR_NONICKNAMEGIVEN "431"
#define ERR_ERRONEUSNICKNAME "432"
#define ERR_NICKNAMEINUSE "433"
#define ERR_NOTREGISTERED "451"
#define ERR_NEEDMOREPARAMS "461"
#define ERR_ALREADYREGISTRED "462"
#define ERR_PASSWDMISMATCH "464"
#define ERR_NOOPERHOST "491"

#define CRLF "\r\n"

static const char *g_commands_name[] = {
	"PASS",
	"NICK",
	"USER",
	"OPER",
	NULL
};

class ClientsMonitoringList
{
	private:
		int _fd;
        std::string _current_packet;
		std::vector<std::string> _split_packet;

		bool _logged;
        bool _registered;
		bool _oper;
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
		std::vector<std::string> string_split(std::string s, const char delimiter);
		void	do_command(const std::string password);
		int		is_command();
		void	send_message(std::string error);
		int		check_if_nickname_is_erroneous(std::string split_packet);
		int		check_if_nickname_is_already_used(std::string nickname);

		// commands
		void	PASS(const std::string password);
		void	NICK();
		void	USER();
		void	OPER();
};

#endif