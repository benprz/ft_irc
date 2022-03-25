
#include "ClientsMonitoringList.hpp"

void	ClientsMonitoringList::send_message(std::string numeric_reply)
{
	std::string message;

	if (numeric_reply == RPL_WELCOME)
	{
		_registered = 1;
		message = numeric_reply + " " + _nickname + " :Welcome to the Internet Relay Network <" + _nickname + ">!<" + _username + ">@<" + _hostname + ">";
	}
	else if (numeric_reply == ERR_NOTREGISTERED)
		message = numeric_reply + " :You have not registered";
	else if (numeric_reply == ERR_ALREADYREGISTRED)
		message = numeric_reply + " :Unauthorized command (already registered)";
	else if (numeric_reply == ERR_NONICKNAMEGIVEN)
		message = numeric_reply + " :No nickname given";
	message += CRLF;
	send(_fd, message.c_str(), message.size(), 0);
}

void	ClientsMonitoringList::PASS(const std::vector<std::string> split_packet, const std::string password)
{
	std::cout << "pass command!" << std::endl; // del
	if (!_logged)
	{
		if (split_packet.size() >= 2)
		{
			if (split_packet[1] == password)
			{
				_logged = 1;
				std::cout << _fd << " logged!" << std::endl;
			}
		}
	}
	else
		send_message(ERR_ALREADYREGISTRED);
}

int		ClientsMonitoringList::check_if_nickname_is_already_used(std::string nickname)
{
	ClientsMonitoringList *Clients = this;

	while (Clients->prev)
		Clients = Clients->prev;
	while (Clients->next)
	{
		if (nickname == Clients->getNickname())
			return (1);
		Clients = Clients->next;
	}
	return (0);
}

int		ClientsMonitoringList::check_if_nickname_is_erroneous(std::string nickname)
{
	if (nickname.size() <= 9)
	{
		size_t invalid_char = nickname.find_first_not_of("AZERTYUIOPQSDFGHJKLMWXCVBNazertyuiopqsdfghjklmwxcvbn1234567890[]\\`_^{}|");
		if (invalid_char == std::string::npos)
			return (0);
	}
	return (1);
}

void	ClientsMonitoringList::NICK(std::vector<std::string> split_packet)
{
	std::cout << "nick command!" << std::endl; // del
	if (!_logged)
		send_message(ERR_NOTREGISTERED);
	else if (split_packet.size() < 2)
		send_message(ERR_NONICKNAMEGIVEN);
	else if (check_if_nickname_is_erroneous(split_packet[1]))
		send_message(ERR_ERRONEUSNICKNAME);
	else if (check_if_nickname_is_already_used(split_packet[1]))
		send_message(ERR_NICKNAMEINUSE);
	else
	{
		_nickname = split_packet[1];
		std::cout << "nickname: " << _nickname << std::endl;
		if (_username != "")
			send_message(RPL_WELCOME);
	}
}

void	ClientsMonitoringList::USER(std::vector<std::string> split_packet)
{
	std::cout << "user command!" << std::endl; // del
	if (!_logged)
		send_message(ERR_NOTREGISTERED);
	else if (split_packet.size() < 5)
		send_message(ERR_NEEDMOREPARAMS);
	else if (_registered || _username != "")
		send_message(ERR_ALREADYREGISTRED);
	else
	{
		_username = split_packet[1];
		_mode = split_packet[2];
		for (int i = 5; i < split_packet.size(); i++)
		{
			split_packet[4] += " ";
			split_packet[4] += split_packet[i];
		}
		split_packet[4].erase(0, 1);
		_realname = split_packet[4];
		std::cout << "username: " << _username << std::endl;
		std::cout << "realname: " << _realname << std::endl;
		if (_nickname != "")
			send_message(RPL_WELCOME);
	}

}

void	ClientsMonitoringList::QUIT(const std::vector<std::string> split_packet)
{
	std::cout << "quit command!" << std::endl; // del
	if (!_logged)
	{
		if (split_packet.size() >= 2)
		{
			// au lieu du nick, ecrire le message ajoute en parametre
		}
		else
			// ecrire le nick et qu'il a quitte le server mais comment retrouver le nick ? (avec un fd ?)
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


int		ClientsMonitoringList::is_command(std::string command)
{
	for (int i = 0; g_commands_name[i]; i++)
	{
		if (command == "PASS" || command == "NICK" || command == "USER")
			return (1);
		else if (_registered && command == g_commands_name[i])
			return (1);
	}
	return (-1);
}

void	ClientsMonitoringList::do_command(std::vector<std::string> split_packet, const std::string password)
{
	std::string c = split_packet[0];
	if (c == "PASS")
		PASS(split_packet, password);
	else if (c == "NICK")
		NICK(split_packet);
	else if (c == "USER")
		USER(split_packet);
	else if (c == "QUIT")
		QUIT(split_packet);
}

void	ClientsMonitoringList::parse_client_packet(std::string packet, const std::string password)
{
	std::string current_command;
	int	newline_pos;

	_current_packet += packet;
	while ((newline_pos = _current_packet.find("\n")) != std::string::npos)
	{
		current_command = _current_packet.substr(0, newline_pos);
		current_command.erase(std::remove(current_command.begin(), current_command.end(), '\r'));
		_current_packet.erase(0, newline_pos + 1);
		std::vector<std::string> split_packet = string_split(current_command, ' ');
		if (is_command(split_packet[0]) == 1)
			do_command(split_packet, password);
		else
			send(_fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}