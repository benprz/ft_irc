#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

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
#define MAX_ALLOWED_CHANNELS_PER_CLIENT 10
#define OPER_HOST 1
#define OPER_PASSWD "oper123"

#define USER_MODES "io" // "iswo" mais s et w ne sont pas utilisés
#define NICK_CHARSET "AZERTYUIOPQSDFGHJKLMWXCVBNazertyuiopqsdfghjklmwxcvbn1234567890[]\\`_^{}|"
#define CRLF "\r\n"

#include "ClientsMonitoringList.hpp"
#include "ChannelsList.hpp"

class Server
{
	private:

		int					_server_fd;
		int const			_port;
		std::string const	_password;

		std::vector<ClientsMonitoringList> 	_Clients;
		std::vector<ChannelsList>			_Channels;

		Server();

	public:
		struct pollfd 			pfds[MAX_ALLOWED_CLIENTS];
		nfds_t					nfds;
		nfds_t					current_pfd;
		ClientsMonitoringList	*Client;
		ChannelsList			*Channel;

		Server(int const port, std::string const password);
		Server(Server const &instance);
		~Server();
		Server &operator=(Server const &instance);

		void launch(void);
		int	create_server_fd(void) const;
		void add_client(int fd);
		void remove_client(int fd);
		void remove_client();
		void remove_client_from_all_chans(int client_fd);
		void remove_client_from_all_chans();
		void add_client_to_chan(int channel_id);
		void remove_client_from_chan(int channel_id, std::string reason);
		void remove_client_from_chan(std::string reason);
		void printpfds(); // debug
		void printchannels(); //debug

		void	parse_client_packet(std::string packet);
		std::vector<std::string> string_split(std::string s, const char delimiter);
		int	parse_command();
		void send_message(std::string error);
		void send_message(int fd, std::string numeric_reply);
		void send_message_to_channel(int channel_id, std::string message);
		void send_message_to_channel(std::string message);
		void send_message_to_client_channels(std::string message);

		int		get_pfd_id(int fd);
		int		get_channel_id(std::string channel);
		int		get_client_id(std::string nick);
		int		get_client_id(int fd);
		int		get_client_fd(std::string nick);
		int 	count_visible_users_on_channel(int channel_id);
		void	edit_channel_modes();
		int		add_or_remove_special_mode_channel(char action, char mode, std::string param);

		// commands
		void	PASS();
		void	NICK();
		void	USER();
		void	OPER();
		void	JOIN();
		void	PART();
		void	KILL();
		void	KICK();
		void	QUIT();
		void	MODE();
		void	INVITE();
		void	NAMES();
		void	PRIVMSG();
		void	LIST();
		void	TOPIC();
};

#endif