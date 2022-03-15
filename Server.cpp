#include "Server.hpp"

Server::Server() : _port(666), _password("dumbpassword")
{
	return;
}

Server::Server(int const port, std::string const password) : _port(port), _password(password)
{
	std::cout << "Intance's port = " << getPort() << " & password = " << getPassword() << std::endl; // del
}

Server::~Server()
{
	return;
}

int Server::getPort() const
{
	return (this->_port);
}

std::string Server::getPassword() const
{
	return (this->_password);
}