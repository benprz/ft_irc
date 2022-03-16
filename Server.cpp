#include "Server.hpp"

Server::Server() : _port(666), _password("dumbpassword")
{
	return;
}

Server::Server(int const port, std::string const password) : _port(port), _password(password)
{
	// std::cout << "Intance's port = " << getPort() << " & password = " << getPassword() << std::endl; // del
}

Server::~Server()
{
	return;
}

// int Server::getPort() const
// {
// 	return (this->_port);
// }

// std::string Server::getPassword() const
// {
// 	return (this->_password);
// }

void	print_pfds(struct pollfd *pfds, nfds_t npfds)
{
	std::cout << "\n#-------- pfds list ---------#\n";
	std::cout << "pfds[" << 0 << "]" << std::endl;
	std::cout << "	fd=" << pfds[0].fd << std::endl;
	std::cout << "	events=" << pfds[0].events << std::endl;
	std::cout << "	revents=" << pfds[0].revents << std::endl;
	for (int i = 1; i < npfds; i++)
	{
		if (pfds[i].fd == -1)
		{
			npfds++;
			continue ;
		}
		std::cout << std::endl << "pfds[" << i << "]" << std::endl;
		std::cout << "	fd=" << pfds[i].fd << std::endl;
		std::cout << "	events=" << pfds[i].events << std::endl;
		std::cout << "	revents=" << pfds[i].revents << std::endl;
	}
	std::cout << "#----------------------------#\n\n";
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
int Server::create_server_descriptor(void) const
{
	int server_fd, sockopt_reuseaddr_val;
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

//overwrite sur un descriptor qui a ete remove (mis a -1)
void    Server::add_descriptor_to_poll(int fd, ClientsMonitoringList *Clients, struct pollfd *pfds, nfds_t &nb_pfds)
{
	nfds_t i = 0;

	while (i < nb_pfds)
	{
		if (pfds[i].fd == -1)
			break ;
		i++;
	}
	std::cout << "Added fd=" << fd << " to pfds[" << i << "] with an actual nb_pfds=" << nb_pfds + 1 << std::endl;
	if (Clients)
		bzero(&Clients[i - 1], sizeof(ClientsMonitoringList));
    pfds[i].fd = fd;
    pfds[i].events = POLLIN;
    nb_pfds++;
}

void    Server::remove_descriptor_from_poll(struct pollfd &pfds, nfds_t &nb_pfds)
{
	pfds.fd = -1;
	pfds.events = 0;
	pfds.revents = 0;
	nb_pfds--;
}
int		Server::monitor_clients(int server_fd)
{
	struct pollfd pfds[MAX_CLIENT_CONNEXIONS];
	ClientsMonitoringList Clients[MAX_CLIENT_CONNEXIONS - 1];
	nfds_t nb_clients = 0;
    int nb_ready_clients, client_fd;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

    //on ajoute server_fd au tableau pollfd requis pour poll
	add_descriptor_to_poll(server_fd, NULL, pfds, nb_clients);
	print_pfds(pfds, nb_clients);
	while (1)
	{
		if ((nb_ready_clients = poll(pfds, nb_clients, -1)) == -1)
		{
			std::cout << "Client monitoring error " << errno << " -> poll() : " << strerror(errno) << std::endl;
			break ;
		}
		for (nfds_t i = 0; i < nb_clients; i++)
		{
			if (pfds[i].revents & POLLIN)
			{
				// 0 étant l'index dans le tableau pfds pour server_fd
				if (i == 0)
				{
                    // fcntl O_NONBLOCK du coup la fonction se bloque pas si elle a pas de nouvelles connexions
					if ((client_fd = accept(server_fd, NULL, 0)) == -1)
					{
						std::cout << "Client monitoring error " << errno << " -> accept() : " << strerror(errno) << std::endl;
						break ;
					}
					add_descriptor_to_poll(client_fd, Clients, pfds, nb_clients);
					print_pfds(pfds, nb_clients);
				}
				else
				{
					recv_length = recv(pfds[i].fd, recv_buf, RECV_BUF_SIZE, 0); 
					//si y'a une erreur
					if (recv_length < 0)
					{
						std::cout << "Error connexion stopped with client fd=" << client_fd <<  " events=" << pfds[i].events << " revents=" << pfds[i].revents << std::endl;
						std::cout << "Client monitoring error " << errno << " -> recv() : " << strerror(errno) << std::endl;
						close(pfds[i].fd);
						remove_descriptor_from_poll(pfds[i], nb_clients);
						print_pfds(pfds, nb_clients);
						continue ;
					}
					//la connexion s'est coupée, EOF, on supprime donc le fd
					else if (recv_length == 0)
					{
						std::cout << "Connexion stopped with client_fd=" << client_fd << std::endl;
						close(pfds[i].fd);
						remove_descriptor_from_poll(pfds[i], nb_clients);
						print_pfds(pfds, nb_clients);
					}
					//on a reçu un paquet! on l'ouvre :-)
					else 
					{
						recv_buf[recv_length] = 0;
						parse_client_packet(Clients[i - 1], client_fd, recv_buf);
					}
				}
			}
		}
	}
	return (ERROR);
}

int Server::launch(void)
{
	int server_fd;

	if ((server_fd = create_server_descriptor()) >= 3)
	{
		if (monitor_clients(server_fd) == 0)
			return (0);
	}
	return (ERROR);
}
