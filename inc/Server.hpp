#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <iterator>

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <stdio.h>
#include <sys/errno.h>
#include <unistd.h>

#define ERROR -1
#define PROTO "TCP"
#define HOSTNAME "localhost"
#define SOCK_DOMAIN AF_INET
#define SOCK_TYPE SOCK_STREAM
#define RECV_BUF_SIZE 512

#define MAX_ALLOWED_CLIENTS 100
#define OPER_HOST 1
#define OPER_PASSWD "oper123"

#define CRLF "\r\n"

#include "ChannelsList.hpp"

class ClientsMonitoringList
{
	public:
		int fd;
		std::string packet;
		std::vector<std::string> split_packet;

		bool logged;
		bool registered;
		bool oper;
		std::string nickname;
		std::string username;
		std::string realname;
		std::string hostname;
		std::string	mode;

		int opened_channels;

		//~ClientsMonitoringList() {};
};

class Server
{
	private:

		int					_server_fd;
		int const			_port;
		std::string const	_password;

		ClientsMonitoringList 	_Clients[MAX_ALLOWED_CLIENTS]; //Le premier client est à [1], le [0] est vide c pour le serveur
		ChannelsList			_Channels[MAX_ALLOWED_CHANNELS];

		Server();

	public:
		struct pollfd 			pfds[MAX_ALLOWED_CLIENTS];
		nfds_t					nfds;
		nfds_t					current_pfd;
		ClientsMonitoringList	*Client;
		int						nchannels;

		Server(int const port, std::string const password);
		Server(Server const &instance);
		~Server();
		Server &operator=(Server const &instance);

		void launch(void);
		int	create_server_fd(void) const;
		void add_client(int fd);
		void remove_client();
		void remove_client(nfds_t kill_pfd);
		void printpfds(); // debug

		void	parse_client_packet(std::string packet);
		std::vector<std::string> string_split(std::string s, const char delimiter);
		int	parse_command();
		void		send_message(std::string error);

		int			check_if_nickname_is_erroneous(std::string split_packet);
		int			check_if_nickname_is_already_used(std::string nickname);
		std::string	get_current_client_prefix() { return (Client->nickname + "!" + Client->username + "@" + HOSTNAME); };
		int			get_channel_id(std::string channel);

		// commands
		void	PASS();
		void	NICK();
		void	USER();
		void	OPER();
		void	JOIN();
		void	KILL();
		void	KICK();
		void	QUIT();
		void	SQUIT();
};

#endif