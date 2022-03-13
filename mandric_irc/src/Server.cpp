#include "../inc/Server.hpp"


Server::Server() : _portnum(6667), _password("qwe"), read_fd_set(), mess(), _saddr(){
        _v_clients.clear();
        _socket_fd.clear();
        chan.clear();
        chan_oper.clear();
        chan_flag.clear();
        chan_pass.clear();
}

Server::Server(int port, std::string pass) : _portnum(port), _password(pass)
{
        _v_clients.clear();
        _socket_fd.clear();
        chan.clear();
        chan_oper.clear();
        chan_flag.clear();
        chan_pass.clear();
}

Server::Server( Server const & src ) : _v_clients(src.getClients()), _portnum(src.getPort()), _password(src.getPass()){

}

Server::~Server(){
}

Server &    Server::operator=(Server const & rhs ){
        this->_v_clients = rhs._v_clients;
        return(*this);
}

void Server::on_connection(sockaddr_in new_addr, socklen_t addrlen, int new_fd) {
        Client* new_user = new Client(new_addr, addrlen, new_fd);
        std::cout << "Accepted a new connection with fd " << new_fd << std::endl;
        this->_socket_fd.push_back(new_fd);
        this->_v_clients.push_back(*new_user);
        std::cout << inet_ntoa(_v_clients.back().getInfo().sin_addr) << " -> Ip adress of new_user" << std::endl;
				delete new_user;
}

bool Server::bufferComplete(int fd, char buf[5000]) {
        std::string s(buf);

        _m_fdbuffer[fd] += s;
        if (_m_fdbuffer[fd].find('\n') == std::string::npos) {
                return (false);
        }
        return(true);

}

void Server::loop(void)
{
        int ret_val;
        int new_fd;

        struct sockaddr_in new_addr;
        socklen_t addrlen;

        while (420)
        {
                FD_ZERO(&read_fd_set); /* Set the fd_set before passing it to the select call */
                for (std::vector<int>::iterator it = _socket_fd.begin(); it != _socket_fd.end(); it++)
                        FD_SET(*it, &read_fd_set);

                /* Invoke select() and then wait! */
                std::cout << "Poll with select to listen for entry" << std::endl;
                ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
                /* select() woke up. Identify the fd that has events */
                std::cout << "ret_val: " << ret_val << '\n';
                if (ret_val >= 0)
                {
                        std::cout << "Return of select : " << ret_val << std::endl;
                        /* Check if the fd with event is the server fd */
                        if (FD_ISSET(_socket_fd[0], &read_fd_set))
                        {
                                std::cout << "The server find a new connection" << std::endl;
                                new_fd = accept(_socket_fd[0], (struct sockaddr*)&new_addr, &addrlen);
                                if (new_fd >= 0 && _socket_fd.size() <= 1024)
                                {
                                        on_connection(new_addr, addrlen, new_fd);
                                }
                                else
                                {
                                        std::cerr << "accept failed " << strerror(errno) << std::endl;
                                        //Throw Here.
                                }
                                ret_val--;
                                if (!ret_val) continue;
                        }

                        /* Check if the fd with event is a non-server fd */
                        for (std::vector<int>::iterator it_fd = _socket_fd.begin() + 1; it_fd != _socket_fd.end(); it_fd++)
                        {
                                if (FD_ISSET(*it_fd, &read_fd_set))
                                {
                                        // on_message(it_fd, &ret_val);
                                        /* read incoming data */
                                        std::cout << "Listening from fd-> " << *it_fd << std::endl;
                                        if (_m_fdbuffer[*it_fd].find('\n') != std::string::npos) {
                                                _m_fdbuffer[*it_fd].erase();
                                        }
                                        ret_val = recv(*it_fd, _buf, DATA_BUFFER, 0);
                                        /*ret_val >= 2 ? _buf[ret_val] = 0 : */ _buf[ret_val] = 0;
                                        if (ret_val == 0)
                                        {
                                                std::cout << "Closing connection for fd-> " << *it_fd << std::endl;
                                                remove_Cinchans(*it_fd);
                                                _m_fd2client.erase(*it_fd);
                                                clearClient(*it_fd);
                                                close(*it_fd);
                                                FD_CLR(*it_fd, &read_fd_set);
                                                _socket_fd.erase(it_fd); /* Connection is now closed */
                                                break;
                                        }
                                        if (ret_val > 0)
                                        {
                                                std::cout << _buf << std::endl;
                                                if (bufferComplete(*it_fd, _buf)) {
                                                        _m_fdbuffer[*it_fd].erase(std::remove(_m_fdbuffer[*it_fd].begin(), _m_fdbuffer[*it_fd].end(), '\r'), _m_fdbuffer[*it_fd].end());
                                                        std::string response = executionner(_m_fdbuffer[*it_fd], mess, *it_fd);
                                                }
                                                break;
                                        }
                                        if (ret_val == -1)
                                        {
                                                std::cerr << "recv() failed for fd-> " << *it_fd << " : " << strerror(errno) << std::endl;
                                                break;
                                        }
                                        memset(_buf, '\0', DATA_BUFFER);
                                }
                                ret_val--;
                                if (!ret_val) continue;
                        } /* for-loop */
                } /* (ret_val >= 0) */
                else {
                        return;
                }
        } /* while(1) */
}

void Server::clearClient(int fd) {
        std::vector<Client>::iterator it = _v_clients.begin();
        for (; it != _v_clients.end(); it++) {
                if (it->getFd() == fd) {
                        _v_clients.erase(it);
                        return;
                }
        }
}

int Server::launch(void)
{

        Message mess_test;
        struct sockaddr_in new_addr;
        int server_fd, new_fd, ret_val, i;
        socklen_t addrlen;

        std::string response = "";

        /* Get the socket server fd */
        server_fd = create_tcp_server_socket(this->_portnum);
        if (server_fd == -1)
        {
                std::cerr << "Failed to create a server" << std::endl;
                return -1;
        }
        _socket_fd.push_back(server_fd);

        loop();

        /* Last step: Close all the sockets */
        for (std::vector<int>::iterator it_fd = _socket_fd.begin(); it_fd != _socket_fd.end(); it_fd++)
                close(*it_fd);
        _socket_fd.clear();

        return 0;
}


int Server::create_tcp_server_socket(int port)
{
        int fd, ret_val;

        /* Step1: create a TCP socket */
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        std::cout << "server fd is " << fd <<  ":D" << std::endl;
        if (fd == -1) {
                fprintf(stderr, "socket failed [%s]\n", strerror(errno));
                return -1;
        }
        printf("Created a socket with fd: %d\n", fd);

        /* Initialize the socket address structure */
        _saddr.sin_family = AF_INET;
        _saddr.sin_port = htons(port);
        _saddr.sin_addr.s_addr = INADDR_ANY;

        /* Step2: bind the socket to port 7000 on the local host */
        ret_val = bind(fd, (struct sockaddr *)&_saddr, sizeof(struct sockaddr_in));
        if (ret_val != 0)
        {
                fprintf(stderr, "bind failed [%s]\n", strerror(errno));
                close(fd);
                return -1;
        }

        /* Step3: listen for incoming connections */
        ret_val = listen(fd, 5);
        if (ret_val != 0)
        {
                fprintf(stderr, "listen failed [%s]\n", strerror(errno));
                close(fd);
                return -1;
        }
        return fd;
}

std::string Server::executionner(std::string buf_str, Message &message, int fd)
{
        if (buf_str.size() >= 2)
        {
                std::list<std::string> list_cmd;
                std::list<std::string>::iterator it;

                list_cmd = split_every_char(buf_str, '\n');
                it = list_cmd.begin();
                for (; it != list_cmd.end(); it++)
                {
                        message.parsing_cmd(*it);
                        message.display();
                        do_cmd(message, fd);
                }
        }
        return(""); // sert a rien ?
}

std::string Server::ft_itoa(int nbr){
        std::string s;
        std::stringstream out;
        out << nbr;
        s = out.str();
        return(s);
}

std::list<std::string> Server::split_every_char(std::string str, char separator) {
        size_t pos = 0;
        std::list<std::string> list_temp;

        while ((pos = str.find(separator)) != std::string::npos)
        {
                std::string temp = str.substr(0, pos);
                list_temp.push_back(temp);
                str = str.erase(0, pos + 1);
        }
        if (pos == std::string::npos) {
                std::string temp = str.substr(0, pos);
                if (temp.length() != 0) {
                        list_temp.push_back(temp);
                }
        }
        return(list_temp);
}
