#include "Server.hpp"

#define ERR_PASSWDMISMATCH 464

//overwrite sur un descriptor qui a ete remove (mis a -1)
/*void	add_descriptor_to_poll(int fd, ClientsMonitoringList *Clients, nfds_t *nb_clients)
{
	nfds_t i = 0;

	while (i < *nb_clients)
	{
		if (Clients[i].fd == -1)
			break ;
		i++;
	}
	Clients[i].fd = fd;
	Clients[i].events = POLLIN;
	Clients[i].revents = 0;
	*nb_clients += 1;
					std::cout << fd << " " << *nb_clients << std::endl;
}

void	remove_descriptor_from_poll(int fd, ClientsMonitoringList *Clients, nfds_t *nb_clients)
{
	for (nfds_t i = 0; i < *nb_clients; i++)
	{
		if (Clients[i].fd == fd)
		{
			Clients->reset();
			*nb_clients -= 1;
			break ;
		}
	}
}

void	send_msg_to_all_fds(struct pollfd *pfds, char *buf, ssize_t length, nfds_t nb_pfds, int sender_fd)
{
	std::cout << "sender_fd=" << sender_fd << std::endl;
	for (nfds_t i = 1; i < nb_pfds; i++)
	{
		if (sender_fd != pfds[i].fd)
			send(pfds[i].fd, buf, length, 0);
	}
}
*/

//overwrite sur un descriptor qui a ete remove (mis a -1)
void    add_descriptor_to_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
{
    pfds[*nb_pfds].fd = fd;
    pfds[*nb_pfds].events = POLLIN;
    pfds[*nb_pfds].revents = 0;
    *nb_pfds += 1;
}

void    remove_descriptor_from_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
{
    for (int i = 0; i < *nb_pfds; i++)
    {
        if (pfds[i].fd == fd)
        {
            pfds[i].fd = -1;
            pfds[i].events = 0;
            pfds[i].revents = 0;
            nb_pfds -= 1;
            break ;
        }
    }
}

void    send_msg_to_all_fds(struct pollfd *pfds, char *buf, ssize_t length, nfds_t nb_pfds, int sender_fd)
{
    std::cout << "sender_fd=" << sender_fd << std::endl;
    for (int i = 1; i < nb_pfds; i++)
    {
        if (sender_fd != pfds[i].fd)
            send(pfds[i].fd, buf, length, 0);
    }
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

void	parse_client_packet(ClientsMonitoringList &Client, std::string packet)
{
	int	command_index;

	if (packet.find('\n') != std::string::npos)
	{
		packet.push_back('\n');
		packet.push_back('\r');
		std::cout << "A" << std::endl;
		std::vector<std::string> split_packet = string_split(packet, ' ');
		if (split_packet.size() > 0)
		{
			std::cout << "B " << packet << std::endl;
			if ((command_index = get_command_index(split_packet[0])) >= 0)
				g_commands_functions[command_index](split_packet);
			else
				send(Client.fd, "Error: unknown command", strlen("Error: unknown command"), 0);
		}
		else
			send(Client.fd, "Error: no input command", strlen("Error: no input command"), 0);
	}
	else
		Client.current_packet += packet;
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
