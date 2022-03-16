#include "Server.hpp"

#include <iostream>
#include <vector>

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "ClientsMonitoringList.hpp"

#define ERROR -1 // je trouve -1 plus logique perso
#define PROTO "TCP"
#define PORT 16385
#define SOCK_DOMAIN AF_INET
#define IP_ADDR "127.0.0.1"
#define SOCK_TYPE SOCK_STREAM
#define RECV_BUF_SIZE 1024
#define PASSWORD "sylben123"

#define ERR_PASSWDMISMATCH 464

int	create_server_descriptor(void)
{
	int	server_fd, sockopt_reuseaddr_val;
    protoent *proto;
	struct sockaddr_in server_addr;

    // on récupère l'index du protocole TCP dans /etc/protocols
	// (sous UNIX seulement)
    if ((proto = getprotobyname(PROTO)) == NULL)
    {
        std::cout << "Couldn't find protocol: " << PROTO << std::endl;
        return (ERROR);
    }

    // on crée un socket sur le domaine d'internet
    if ((server_fd = socket(SOCK_DOMAIN, SOCK_TYPE, proto->p_proto)) == -1)
    {
        std::cout << "Server socket error -> socket() : " << strerror(errno) << std::endl;
        return (ERROR);
    }

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = SOCK_DOMAIN;
	server_addr.sin_port = htons(PORT);
	inet_pton(SOCK_DOMAIN, IP_ADDR, &server_addr.sin_addr);

    // setsockopt SO_REUSEADDR permet de réutiliser des adresses déjà utilisées
	// (ça permet de fix le fait que parfois les fd des sockets ne sont pas tout
	// le temps complètements supprimés, du coup ça met une erreur au moment du
	// bind())
    sockopt_reuseaddr_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_reuseaddr_val, sizeof(sockopt_reuseaddr_val));

    // on bind l'adresse du serveur au socket
    if (bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	// faire cast c++ | il faudra unlink selon le man "Binding a name in the
	// UNIX domain creates a socket in the file system that must be deleted
	// by the caller when it is no longer needed (using unlink(2))."
    {
        std::cout << "Server socket error " << errno << " -> bind() : " << strerror(errno) << std::endl;
        return (ERROR);
    }

    // on peut listen sur le socket, écouter les connexions entrantes
    if (listen(server_fd, MAX_CLIENT_CONNEXIONS) == -1)
    {
        std::cout << "Server socket error " << errno << " -> listen() : " << strerror(errno) << std::endl;
        return (ERROR);
    }

    // on set le server_fd en O_NONBLOCK pour que accept ne loop pas en
	// attendant une connexion et on surveille les I/O des sockets avec poll en
	// mettant le timeout à -1 pour que ça soit lui qui attende indéfiniment une
	// connexion
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "Server socket error " << errno << " -> fcntl(*, F_SETFL, O_NONBLOCK) : " << strerror(errno) << std::endl;
        return (ERROR);
    }

	return (server_fd);
}

std::vector<std::string> string_split(std::string s, const char delimiter)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);
    
    std::vector<std::string> output;
    
    while (end <= std::string::npos)
    {
	    output.__emplace_back(s.substr(start, end-start));

	    if (end == std::string::npos)
	    	break;

    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
    return output;
}

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

int	get_command_index(std::string command)
{
	for (int i = 0; i < NB_COMMANDS; i++)
	{
		if (command == g_commands_name[i])
			return (i);
	}
	return (-1);
}

void	pass_command(std::vector<std::string> split_packet)
{
	std::cout << "pass command!" << std::endl;
}

void	nick_command(std::vector<std::string> split_packet)
{
	std::cout << "nick command!" << std::endl;
}

void	(*g_commands_functions[NB_COMMANDS])(std::vector<std::string>) = {
	pass_command,
	nick_command
};

void	parse_client_packet(ClientsMonitoringList &Client, int client_fd, std::string packet)
{
	int	command_index;
	std::string current_command;
	int	newline_pos;

	Client.current_packet += packet;
	while ((newline_pos = Client.current_packet.find('\n')) != std::string::npos)
	{
		current_command = Client.current_packet.substr(0, newline_pos);
		//current_command.erase(current_command.find('\r'));
		//std::cout << "(" << Client.current_packet << ")" << std::endl;
		Client.current_packet.erase(0, newline_pos + 1);
		std::vector<std::string> split_packet = string_split(current_command, ' ');
		// Je suis pas sûr qu'on ait besoin de check ça
		// if (split_packet.size() > 0)
		// {
			if ((command_index = get_command_index(split_packet[0])) >= 0)
				g_commands_functions[command_index](split_packet);
			else
				send(client_fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
		// }
		// else
		// 	send(client_fd, "Error: no input command\n", strlen("Error: no input command\n"), 0);
	}
}

int main(int argc, char **argv)
{
	int ret;
	setbuf(stdout, NULL); // debug, pour éviter que printf ou cout print les lignes que quand y'a un /n, à la place il print à chaque caractère

	if (argc == 3)
	{
		Server sylbenirc(atoi(argv[1]), argv[2]);
		ret = sylbenirc.launch();
	}
	else
		std::cout << "Usage : ./ircserv <port> <password>" << std::endl;
    
	return (ret);
}
