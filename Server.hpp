#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>

class Server
{
	private:

		int const			_port;
		std::string const	_password;
		Server();

	public:

		Server(int const port, std::string const password);
		Server(Server const &instance);
		~Server();
		Server &operator=(Server const &instance);

		int			getPort() const;
		std::string getPassword() const;
	

};

#endif