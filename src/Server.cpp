#include "Server.hpp"

Server::Server() : _port(666), _password("dumbpassword")
{
}

Server::Server(int const port, std::string const password) : _port(port), _password(password)
{
	_server_fd = create_server_fd();
	bzero(pfds, sizeof(pfds));
	nfds = 0;
}

Server::~Server()
{
	close(_server_fd);
}

void	Server::printpfds() // debug
{
	std::cout << "\n#-------- pfds list ---------#\n\n";
	std::cout << "pfds[" << 0 << "]" << std::endl;
	std::cout << "	fd=" << pfds[0].fd << std::endl;
	std::cout << "	events=" << pfds[0].events << std::endl;
	std::cout << "	revents=" << pfds[0].revents << std::endl;
	for (int i = 1; i < MAX_ALLOWED_CLIENTS; i++)
	{
		if (pfds[i].events == 0)
			continue ;
		std::cout << std::endl << "pfds[" << i << "]" << std::endl;
		std::cout << "	fd=" << pfds[i].fd << std::endl;
		std::cout << "	events=" << pfds[i].events << std::endl;
		std::cout << "	revents=" << pfds[i].revents << std::endl;
	}
	if (_Clients.size())
	{
		std::cout << std::endl << "#-------- clients list -------#\n\n";
		std::cout << "Clients[" << 0 << "]" << std::endl;
		std::cout << "	fd=" << _Clients[0].fd << std::endl;
		std::cout << "	logged=" << _Clients[0].logged << std::endl;
		std::cout << "	registered=" << _Clients[0].registered << std::endl;
		std::cout << "	nickname=" << _Clients[0].nickname << std::endl;
		std::cout << "	username=" << _Clients[0].username << std::endl;
		std::cout << "	realname=" << _Clients[0].realname << std::endl;
		std::cout << "	mode=" << _Clients[0].modes << std::endl;
		std::cout << "	opened_channels=" << _Clients[0].opened_channels << std::endl;
		for (int i = 1; i < _Clients.size(); i++)
		{
			std::cout << std::endl << "Clients[" << i << "]" << std::endl;
			std::cout << "	fd=" << _Clients[i].fd << std::endl;
			std::cout << "	logged=" << _Clients[i].logged << std::endl;
			std::cout << "	registered=" << _Clients[i].registered << std::endl;
			std::cout << "	nickname=" << _Clients[i].nickname << std::endl;
			std::cout << "	username=" << _Clients[i].username << std::endl;
			std::cout << "	realname=" << _Clients[i].realname << std::endl;
			std::cout << "	mode=" << _Clients[i].modes << std::endl;
			std::cout << "	opened_channels=" << _Clients[i].opened_channels << std::endl;
		}
	}
	std::cout << "\n#-----------------------------#\n\n";
}

int Server::create_server_fd(void) const
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
	server_addr.sin_port = htons(_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

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
	if (listen(server_fd, MAX_ALLOWED_CLIENTS) == -1)
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

std::vector<std::string> Server::string_split(std::string s, const char delimiter)
{
	size_t start = 0;
	size_t end = s.find_first_of(delimiter);

	std::vector<std::string> output;

	while (end <= std::string::npos)
	{
		output.__emplace_back(s.substr(start, end - start));

		if (end == std::string::npos)
			break;

		start = end + 1;
		end = s.find_first_of(delimiter, start);
	}
	return output;
}

void    Server::add_client(int fd)
{
	std::cout << "Add client! fd=" << fd << std::endl;
	for (int i = 0; i < MAX_ALLOWED_CLIENTS; i++)
	{
		if (pfds[i].events == 0)
		{
			if (i > 0)
				_Clients.push_back(fd);
			pfds[i].fd = fd;
			pfds[i].events = POLLIN;
			nfds++;
			std::cout << "Added fd=" << fd << " to pfds[" << i << "] with an actual nfds=" << nfds + 1 << std::endl;
			printpfds();
			break ;
		}
	}
}

void	Server::remove_client_from_all_chans(int client_fd)
{
	for (int i = 0; i < _Channels.size(); i++)
	{
		if (_Channels[i].is_user_on_channel(client_fd))
			remove_client_from_chan(i, get_client_id(client_fd), "");
	}
}

void	Server::remove_client_from_all_chans()
{
	remove_client_from_all_chans(Client->fd);
}

int	Server::get_pfd_id(int fd)
{
	for (int i = 0; i < MAX_ALLOWED_CLIENTS; i++)
	{
		if (pfds[i].fd == fd)
			return (i);
	}
	return (ERROR);
}

void    Server::remove_client(int fd)
{
	int pfd_id = get_pfd_id(fd);
	int client_id = get_client_id(fd);

	if (pfd_id >= 0 && client_id >= 0)
	{
		remove_client_from_all_chans(fd);
		_Clients.erase(_Clients.begin() + client_id);

		close(pfds[pfd_id].fd);
		pfds[pfd_id].fd = -1;
		pfds[pfd_id].events = 0;
		pfds[pfd_id].revents = 0;

		printpfds();
	}
}

void    Server::remove_client()
{
	remove_client(Client->fd);
}

void Server::parse_client_packet(std::string packet)
{
	int newline_pos;

	Client = &_Clients[get_client_id(pfds[current_pfd].fd)];
	Client->packet += packet;
	while ((newline_pos = Client->packet.find("\n")) != std::string::npos)
	{
		Client->current_command = Client->packet.substr(0, newline_pos);
		Client->current_command.erase(std::remove(Client->current_command.begin(), Client->current_command.end(), '\r'));

		Client->packet.erase(0, newline_pos + 1);
		Client->split_command = string_split(Client->current_command, ' ');
		if (parse_command())
			send_message(ERR_UNKNOWNCOMMAND);
	}
}

void Server::launch(void)
{
    int nb_ready_clients, client_fd;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

	if (_server_fd != ERROR)
	{
		//on ajoute server_fd au tableau pollfd requis pour poll
		add_client(_server_fd);
		while (1)
		{
			if ((nb_ready_clients = poll(pfds, nfds, -1)) == -1)
			{
				std::cout << "Client monitoring error " << errno << " -> poll() : " << strerror(errno) << std::endl;
				break ;
			}
			for (current_pfd = 0; current_pfd < MAX_ALLOWED_CLIENTS; current_pfd++)
			{
				if (!nb_ready_clients)
						break ;
				if (pfds[current_pfd].revents & POLLIN)
				{
					nb_ready_clients--;
					// 0 étant l'index dans le tableau pfds pour server_fd
					if (current_pfd == 0)
					{
						// fcntl O_NONBLOCK du coup la fonction se bloque pas si elle a pas de nouvelles connexions
						if ((client_fd = accept(_server_fd, NULL, 0)) == -1)
						{
							std::cout << "Client monitoring error " << errno << " -> accept() : " << strerror(errno) << std::endl;
							break ;
						}
						add_client(client_fd);
					}
					else
					{
						recv_length = recv(pfds[current_pfd].fd, recv_buf, RECV_BUF_SIZE, 0); 
						//si y'a une erreur
						if (recv_length < 0)
						{
							std::cout << "Error connexion stopped with client fd=" << client_fd <<  " events=" << pfds[current_pfd].events << " revents=" << pfds[current_pfd].revents << std::endl;
							std::cout << "Client monitoring error " << errno << " -> recv() : " << strerror(errno) << std::endl;
							remove_client(pfds[current_pfd].fd);
							continue ;
						}
						//la connexion s'est coupée, EOF, on supprime donc le fd
						else if (recv_length == 0)
						{
							std::cout << "Connexion stopped with client_fd=" << pfds[current_pfd].fd << std::endl;
							remove_client(pfds[current_pfd].fd);
						}
						//on a reçu un paquet! on l'ouvre :-)
						else 
						{
							recv_buf[recv_length] = 0;
							std::cout << std::endl;
							std::cout << "# NEW PACKET FROM (current_pfd=" << current_pfd << ", fd=" << pfds[current_pfd].fd << ")" << std::endl;
							std::cout << recv_buf << std::endl;
							parse_client_packet(recv_buf);
						}
					}
				}
			}
		}
	}
}
