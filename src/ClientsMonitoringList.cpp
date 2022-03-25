
#include "ClientsMonitoringList.hpp"

void	ClientsMonitoringList::send_message(std::string numeric_reply)
{
	std::string message;

	if (numeric_reply == RPL_WELCOME)
	{
		_registered = 1;
		message = numeric_reply + " " + _nickname + " :Welcome to the Internet Relay Network <" + _nickname + ">!<" + _username + ">@<" + _hostname + ">";
	}
	else if (numeric_reply == RPL_YOUREOPER)
		message = numeric_reply + " :You are now an IRC operator";
	else if (numeric_reply == ERR_NOTREGISTERED)
		message = numeric_reply + " :You have not registered";
	else if (numeric_reply == ERR_NEEDMOREPARAMS)
		message = numeric_reply + " " + _split_packet[0] + " :Not enough parameters";
	else if (numeric_reply == ERR_ALREADYREGISTRED)
		message = numeric_reply + " :You may not reregister";
	else if (numeric_reply == ERR_NONICKNAMEGIVEN)
		message = numeric_reply + " :No nickname given";
	else if (numeric_reply == ERR_ERRONEUSNICKNAME)
		message = numeric_reply + " " + _split_packet[1] + " :Erroneus nickname";
	else if (numeric_reply == ERR_NICKNAMEINUSE)
		message = numeric_reply + " " + _split_packet[1] + " :Nickname is already in use";
	else if (numeric_reply == ERR_PASSWDMISMATCH)
		message = numeric_reply + " :Password incorrect";
	else if (numeric_reply == ERR_NOOPERHOST)
		message = numeric_reply + " :No O-lines for your host";

	message += CRLF;
	send(_fd, message.c_str(), message.size(), 0);
}

void	ClientsMonitoringList::PASS(const std::string password)
{
	std::cout << "pass command!" << std::endl;
	_logged = 0; //les prochaines commandes pass overwrite les anciennes
	if (_registered)
		send_message(ERR_ALREADYREGISTRED);
	else if (_split_packet.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (_split_packet[1] != password)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		_logged = 1;
		std::cout << _fd << " logged!" << std::endl;
		if (_nickname != "" && _username != "")
			send_message(RPL_WELCOME);
	}
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

void	ClientsMonitoringList::NICK()
{
	std::cout << "nick command!" << std::endl;
	if (_split_packet.size() < 2)
		send_message(ERR_NONICKNAMEGIVEN);
	else if (check_if_nickname_is_erroneous(_split_packet[1]))
		send_message(ERR_ERRONEUSNICKNAME);
	else if (check_if_nickname_is_already_used(_split_packet[1]))
		send_message(ERR_NICKNAMEINUSE);
	else
	{
		_nickname = _split_packet[1];
		std::cout << "nickname: " << _nickname << std::endl;
		if (_logged && _username != "" && !_registered)
			send_message(RPL_WELCOME);
	}
}

void	ClientsMonitoringList::USER()
{
	std::cout << "user command!" << std::endl;
	if (_registered)
		send_message(ERR_ALREADYREGISTRED);
	else if (_split_packet.size() < 4)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		//on ignore _split_packet[2] && [3] parce que c'est pour les communications server-server
		_username = _split_packet[1];
		for (int i = 5; i < _split_packet.size(); i++)
		{
			_split_packet[4] += " ";
			_split_packet[4] += _split_packet[i];
		}
		_split_packet[4].erase(0, 1);
		_realname = _split_packet[4];
		std::cout << "username: " << _username << std::endl;
		std::cout << "realname: " << _realname << std::endl;
		if (_logged && _nickname != "")
			send_message(RPL_WELCOME);
	}
}

// faut mettre le mode au bon nickname, pas à celui qui lance la commande
void	ClientsMonitoringList::OPER()
{
	if (OPER_HOST == 0)
		send_message(ERR_NOOPERHOST);
	else if (_split_packet.size() < 3)
		send_message(ERR_NEEDMOREPARAMS);
	else if (_split_packet[2] != OPER_PASSWD)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		_mode += 'o';
		send_message(RPL_YOUREOPER);
	}
}

std::vector<std::string> ClientsMonitoringList::string_split(std::string s, const char delimiter)
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


int		ClientsMonitoringList::is_command()
{
	std::string command = _split_packet[0];
	for (int i = 0; g_commands_name[i]; i++)
	{
		if (command == "PASS" || command == "NICK" || command == "USER")
			return (1);
		else if (_registered && command == g_commands_name[i])
			return (1);
	}
	return (-1);
}

void	ClientsMonitoringList::do_command(const std::string password)
{
	std::string command = _split_packet[0];
	if (command == "PASS")
		PASS(password);
	else if (command == "NICK")
		NICK();
	else if (command == "USER")
		USER();
	else if (command == "OPER")
		OPER();
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
		_split_packet = string_split(current_command, ' ');
		if (is_command() == 1)
			do_command(password);
		else
			send(_fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}