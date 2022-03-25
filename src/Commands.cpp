#include "Server.hpp"
#include "NumericReplies.hpp"

void	Server::send_message(std::string numeric_reply)
{
	std::string message;

	if (numeric_reply == RPL_WELCOME)
	{
		Client->registered = 1;
		message = numeric_reply + " " + Client->nickname + " :Welcome to the Internet Relay Network <" + Client->nickname + ">!<" + Client->username + ">@<" + Client->hostname + ">";
	}
	else if (numeric_reply == RPL_YOUREOPER)
		message = numeric_reply + " :You are now an IRC operator";
	else if (numeric_reply == ERR_NOTREGISTERED)
		message = numeric_reply + " :You have not registered";
	else if (numeric_reply == ERR_NEEDMOREPARAMS)
		message = numeric_reply + " " + Client->split_packet[0] + " :Not enough parameters";
	else if (numeric_reply == ERR_ALREADYREGISTRED)
		message = numeric_reply + " :You may not reregister";
	else if (numeric_reply == ERR_NONICKNAMEGIVEN)
		message = numeric_reply + " :No nickname given";
	else if (numeric_reply == ERR_ERRONEUSNICKNAME)
		message = numeric_reply + " " + Client->split_packet[1] + " :Erroneus nickname";
	else if (numeric_reply == ERR_NICKNAMEINUSE)
		message = numeric_reply + " " + Client->split_packet[1] + " :Nickname is already in use";
	else if (numeric_reply == ERR_PASSWDMISMATCH)
		message = numeric_reply + " :Password incorrect";
	else if (numeric_reply == ERR_NOOPERHOST)
		message = numeric_reply + " :No O-lines for your host";

	message += CRLF;
	send(Client->fd, message.c_str(), message.size(), 0);
}

void	Server::PASS()
{
	std::cout << "pass command! from fd " << Client->fd << std::endl;
	Client->logged = 0; //les prochaines commandes pass overwrite les anciennes
	if (Client->registered)
		send_message(ERR_ALREADYREGISTRED);
	else if (Client->split_packet.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_packet[1] != _password)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		Client->logged = 1;
		std::cout << "fd " << Client->fd << " logged!" << std::endl;
		if (Client->nickname != "" && Client->username != "")
			send_message(RPL_WELCOME);
	}
}

int		Server::check_if_nickname_is_already_used(std::string nickname)
{
	for (int i = 0; i < nfds - 1; i++)
	{
		if (_Clients[i].nickname == nickname)
			return (1);
	}
	return (0);
}

int		Server::check_if_nickname_is_erroneous(std::string nickname)
{
	if (nickname.size() <= 9)
	{
		size_t invalid_char = nickname.find_first_not_of("AZERTYUIOPQSDFGHJKLMWXCVBNazertyuiopqsdfghjklmwxcvbn1234567890[]\\`_^{}|");
		if (invalid_char == std::string::npos)
			return (0);
	}
	return (1);
}

void	Server::NICK()
{
	std::cout << "nick command!" << std::endl;
	if (Client->split_packet.size() < 2)
		send_message(ERR_NONICKNAMEGIVEN);
	else if (check_if_nickname_is_erroneous(Client->split_packet[1]))
		send_message(ERR_ERRONEUSNICKNAME);
	else if (check_if_nickname_is_already_used(Client->split_packet[1]))
		send_message(ERR_NICKNAMEINUSE);
	else
	{
		Client->nickname = Client->split_packet[1];
		std::cout << "nickname: " << Client->nickname << std::endl;
		if (Client->logged && Client->username != "" && !Client->registered)
			send_message(RPL_WELCOME);
	}
}

void	Server::USER()
{
	std::cout << "user command!" << std::endl;
	if (Client->registered)
		send_message(ERR_ALREADYREGISTRED);
	else if (Client->split_packet.size() < 4)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		//on ignore _split_packet[2] && [3] parce que c'est pour les communications server-server
		Client->username = Client->split_packet[1];
		for (int i = 5; i < Client->split_packet.size(); i++)
		{
			Client->split_packet[4] += " ";
			Client->split_packet[4] += Client->split_packet[i];
		}
		Client->split_packet[4].erase(0, 1);
		Client->realname = Client->split_packet[4];
		std::cout << "username: " << Client->username << std::endl;
		std::cout << "realname: " << Client->realname << std::endl;
		if (Client->logged && Client->nickname != "")
			send_message(RPL_WELCOME);
	}
}

// faut mettre le mode au bon nickname, pas à celui qui lance la commande
void	Server::OPER()
{
	if (OPER_HOST == 0)
		send_message(ERR_NOOPERHOST);
	else if (Client->split_packet.size() < 3)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_packet[2] != OPER_PASSWD)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		Client->mode += 'o';
		send_message(RPL_YOUREOPER);
	}
}

void	Server::JOIN()
{
}

std::vector<std::string> Server::string_split(std::string s, const char delimiter)
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

int		Server::is_command()
{
	std::string command = Client->split_packet[0];
	for (int i = 0; g_commands_name[i]; i++)
	{
		if (command == "PASS" || command == "NICK" || command == "USER")
			return (1);
		else if (Client->registered && command == g_commands_name[i])
			return (1);
	}
	return (-1);
}

void	Server::do_command()
{
	std::string command = Client->split_packet[0];
	if (command == "PASS")
		PASS();
	else if (command == "NICK")
	    NICK();
	else if (command == "USER")
		USER();
	else if (command == "OPER")
		OPER();
	else if (command == "JOIN")
		JOIN();
}

void	Server::parse_client_packet(std::string packet)
{
	std::string current_command;
	int	newline_pos;

	Client = &_Clients[current_pfd];
	Client->packet += packet;
	while ((newline_pos = Client->packet.find("\n")) != std::string::npos)
	{
		current_command = Client->packet.substr(0, newline_pos);
		current_command.erase(std::remove(current_command.begin(), current_command.end(), '\r'));
		Client->packet.erase(0, newline_pos + 1);
		Client->split_packet = string_split(current_command, ' ');
		if (is_command() == 1)
			do_command();
		else
			send(Client->fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}