#include "Server.hpp"
#include "NumericReplies.hpp"

void Server::send_message(int fd, std::string numeric_reply)
{
	std::string message;

	if (numeric_reply.length() != 3)
		message = numeric_reply;
	else
	{
		message = static_cast<std::string>(":") + HOSTNAME + " " + numeric_reply + " ";
		if (numeric_reply == RPL_WELCOME)
		{
			Client->registered = 1;
			message += Client->nickname + " :Welcome to the Internet Relay Network " + Client->nickname + "!" + Client->username + "@" + HOSTNAME;
		}
		else if (numeric_reply == RPL_YOUREOPER)
			message += ":You are now an IRC operator";
		else if (numeric_reply == ERR_NOTREGISTERED)
			message += ":You have not registered";
		else if (numeric_reply == ERR_NEEDMOREPARAMS)
			message += Client->split_packet[0] + " :Not enough parameters";
		else if (numeric_reply == ERR_ALREADYREGISTRED)
			message += ":You may not reregister";
		else if (numeric_reply == ERR_NONICKNAMEGIVEN)
			message += ":No nickname given";
		else if (numeric_reply == ERR_ERRONEUSNICKNAME)
			message += Client->split_packet[1] + " :Erroneus nickname";
		else if (numeric_reply == ERR_NICKNAMEINUSE)
			message += Client->split_packet[1] + " :Nickname is already in use";
		else if (numeric_reply == ERR_PASSWDMISMATCH)
			message += ":Password incorrect";
		else if (numeric_reply == ERR_NOOPERHOST)
			message += ":No O-lines for your host";
		else if (numeric_reply == ERR_INVITEONLYCHAN)
			message += Client->split_packet[1] + " :Cannot join channel (+i)";
		else if (numeric_reply == ERR_TOOMANYCHANNELS)
			message += Client->split_packet[1] + " :You have joined too many channels";
		else if (numeric_reply == ERR_NOSUCHCHANNEL)
			message += Client->split_packet[1] + " :No such channel";
		else if (numeric_reply == ERR_BADCHANNELKEY)
			message += Client->split_packet[1] + " :Cannot join channel (+k)";
		else if (numeric_reply == ERR_CHANNELISFULL)
			message += Client->split_packet[1] + " :Cannot join channel (+l)";
		else if (numeric_reply == ERR_NOTONCHANNEL)
			message += Client->split_packet[1] + " :You're not on that channel";
		else if (numeric_reply == ERR_NOPRIVILEGES)
			message += ":Permission Denied- You're not an IRC operator";
		else if (numeric_reply == ERR_CHANOPRIVSNEEDED)
			message += Client->split_packet[1] + " :You're not channel operator";
		else if (numeric_reply == ERR_KEYSET)
			message += Client->split_packet[1] + " :Channel key already set";
		else if (numeric_reply == ERR_UNKNOWNMODE)
			message += Client->split_packet[2][0] + static_cast<std::string>(" :is unknown mode char to me");
	}

	message += CRLF;
	send(fd, message.c_str(), message.size(), 0);
}

void Server::send_message(std::string numeric_reply)
{
	send_message(Client->fd, numeric_reply);
}

void Server::PASS()
{
	Client->logged = 0;
	if (Client->registered)
		send_message(ERR_ALREADYREGISTRED);
	else if (Client->split_packet.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_packet[1] != _password)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		Client->logged = 1;
		if (Client->nickname != "" && Client->username != "")
			send_message(RPL_WELCOME);
	}
}

void Server::NICK()
{
	if (Client->split_packet.size() < 2)
		send_message(ERR_NONICKNAMEGIVEN);
	else
	{
		std::string nick = Client->split_packet[1];
		if (nick.size() <= 9 && nick.find_first_not_of(NICK_CHARSET) == std::string::npos)
		{
			if (get_client_id(Client->split_packet[1]) == ERROR)
			{
				Client->nickname = Client->split_packet[1];
				if (Client->logged && Client->username != "" && !Client->registered)
					send_message(RPL_WELCOME);
			}
			else
				send_message(ERR_NICKNAMEINUSE);
		}
		else
			send_message(ERR_ERRONEUSNICKNAME);
	}
}

void Server::USER()
{
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
		if (Client->logged && Client->nickname != "")
			send_message(RPL_WELCOME);
	}
}

// faut mettre le mode au bon nickname, pas à celui qui lance la commande
void Server::OPER()
{
	if (OPER_HOST == 0)
		send_message(ERR_NOOPERHOST);
	else if (Client->split_packet.size() < 3)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_packet[2] != OPER_PASSWD)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		int client_id = get_client_id(Client->split_packet[1]);

		if (client_id >= 0 && _Clients[client_id].mode.find('o') == std::string::npos)
		{
			_Clients[client_id].mode += 'o';
			send_message(RPL_YOUREOPER);
		}
	}
}

void	Server::MODE()
{
	if (Client->split_packet.size() < 3)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_packet[1][0] == '#')
	{
		int channel_id = get_channel_id(Client->split_packet[1]);
		if (channel_id >= 0)
		{
			if (_Channels[channel_id].is_user_operator(Client->fd))
			{
				if (Client->split_packet[2][0] == '+' || Client->split_packet[2][0] == '-')
				{
					std::string	err;
					char action = Client->split_packet[2][0];
					std::string third_param = Client->split_packet.size() > 3 ? Client->split_packet[3] : "";
					for (int i = 1; i < Client->split_packet[2].size(); i++)
					{
						Client->split_packet[2][0] = Client->split_packet[2][i];
						if ((err = _Channels[channel_id].add_or_remove_mode(action, Client->split_packet[2][i], third_param, *this)) != "")
						{
							send_message(err);
							break ;
						}
					}
					printchannels();
				}
			}
			else
				send_message(ERR_CHANOPRIVSNEEDED);
		}
		else
			send_message(ERR_NOSUCHCHANNEL);
	}
}

void	Server::printchannels()
{
	std::cout << std::endl << "@@@@@@@@@ Channels list @@@@@@@@@@\n\n";
	for (int i = 0; i < _Channels.size(); i++)
	{
		if (_Channels[i].name != "")
		{
			std::cout << "_Channels[" << i << "]" << std::endl;
			std::cout << "	name=" << _Channels[i].name << std::endl;
			std::cout << "	key=" << _Channels[i].key << std::endl;
			std::cout << "	mode=" << _Channels[i].mode << std::endl;
			std::cout << "	users=";
			for (int j = 0; j < _Channels[i].users.size(); j++)
			{
				std::cout << _Channels[i].users[j] << " ";
			}
			std::cout << "\n";
			std::cout << "	operators=";
			for (int j = 0; j < _Channels[i].operators.size(); j++)
			{
				std::cout << _Channels[i].operators[j] << " ";
			}
			std::cout << "\n";
			std::cout << "	users_limit=" << _Channels[i].users_limit << std::endl;
			std::cout << "\n";
		}
	}
	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n";
}

int	Server::get_client_fd(std::string nick)
{
	int client_id = get_client_id(nick);

	if (client_id >= 0)
		return (_Clients[client_id].fd);
	return (ERROR);
}

int	Server::get_client_id(std::string nick)
{
	for (int i = 0; i < _Clients.size(); i++)
	{
		if (_Clients[i].nickname == nick)
			return (i);
	}
	return (ERROR);
}

int	Server::get_channel_id(std::string channel)
{
	for (int i = 0; i < _Channels.size(); i++)
	{
		if (_Channels[i].name == channel)
			return (i);
	}
	return (ERROR);
}

void	Server::send_message_to_channel(int channel_id, std::string message)
{
	for (int j = 0; j < _Channels[channel_id].users.size(); j++)
		send_message(_Channels[channel_id].users[j], message);
}

void	Server::add_client_to_chan(int channel_id)
{
	std::cout << "yo\n";
	_Channels[channel_id].add_user(Client->fd);
	Client->opened_channels++;
	send_message_to_channel(channel_id, ":" + get_current_client_prefix() + " JOIN " + _Channels[channel_id].name);
	std::cout << "Added user fd=" << Client->fd << " to channel name=" << _Channels[channel_id].name << " chanid=" << channel_id << std::endl;
	printchannels();
}

void	Server::remove_client_from_chan(int channel_id, std::string reason)
{
	send_message_to_channel(channel_id, ":" + get_current_client_prefix() + " PART " + _Channels[channel_id].name + reason);
	_Channels[channel_id].remove_user(Client->fd);
	Client->opened_channels--;
	if (_Channels[channel_id].users.size() == 0)
		_Channels.erase(_Channels.begin() + channel_id);
	std::cout << "Removed user fd=" << Client->fd << " from channel name=" << _Channels[channel_id].name << " chanid=" << channel_id << std::endl;
	printchannels();
}

void	Server::PART()
{
	int channel_id;

	if (Client->split_packet.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		std::vector<std::string> split_channels = string_split(Client->split_packet[1], ',');
		for (int i = 0; i < split_channels.size(); i++)
		{
			Client->split_packet[1] = split_channels[i];
			if ((channel_id = get_channel_id(split_channels[i])) >= 0)
			{
				if (_Channels[channel_id].is_user_on_channel(Client->fd))
				{
					std::string part_reason;
					if (Client->split_packet.size() > 2)
					{
						part_reason += " :";
						for (int i = 2; i < Client->split_packet.size(); i++)
						{
							part_reason += Client->split_packet[i];
							if (i < Client->split_packet.size() - 1)
								part_reason += " ";
						}
					}
					remove_client_from_chan(channel_id, part_reason);
				}
				else
					send_message(ERR_NOTONCHANNEL);
			}
			else
				send_message(ERR_NOSUCHCHANNEL);
		}
	}
}

void	Server::JOIN()
{
	if (Client->split_packet.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->opened_channels > MAX_ALLOWED_CHANNELS_PER_CLIENT)
		send_message(ERR_TOOMANYCHANNELS);
	else if (Client->split_packet[1] == "0")
		remove_client_from_all_chans();
	else
	{
		std::vector<std::string> split_channels = string_split(Client->split_packet[1], ',');
		std::vector<std::string> split_keys;
		int channel_id;
		if (Client->split_packet.size() == 3)
			split_keys = string_split(Client->split_packet[2], ',');
		for (int i = 0; i < split_channels.size(); i++)
		{
			Client->split_packet[1] = split_channels[i];
			if (split_channels[i][0] != '#')
				send_message(ERR_NOSUCHCHANNEL);
			else
			{
				if ((channel_id = get_channel_id(split_channels[i])) != ERROR)
				{
					std::string err;

					if (_Channels[channel_id].is_limited())
					{
						if (_Channels[channel_id].is_users_limit_reached())
							err = ERR_CHANNELISFULL;
					}
					else if (_Channels[channel_id].is_invite_only())
					{
						if (!_Channels[channel_id].is_user_invited(Client->fd))
							err = ERR_INVITEONLYCHAN;
					}
					else if (_Channels[channel_id].is_restricted_by_key())
					{
						if (i >= split_keys.size() || _Channels[channel_id].key != split_keys[i])
							err = ERR_BADCHANNELKEY;
					}
					if (err != "")
					{
						send_message(err);
						continue ;
					}
					else
						add_client_to_chan(channel_id);
				}
				else
				{
					_Channels.push_back(split_channels[i]);
					channel_id = _Channels.size() - 1;
					if (i < split_keys.size())
						_Channels[channel_id].set_key(split_keys[i]);
					add_client_to_chan(channel_id);
					_Channels[channel_id].add_operator(Client->fd);
				}
			}
		}
	}
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

/*
void Server::KILL(void)
{
	std::size_t found = Client->mode.rfind("o");
	if (Client->split_packet.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (found != std::string::npos)
	{
		int i = 1;
		while (_Clients[i].nickname != Client->split_packet[1] && i <= MAX_ALLOWED_CLIENTS)
			i++;
		if (i <= MAX_ALLOWED_CLIENTS)
		{
			std::cout << Client->split_packet[2] << std::endl;
			remove_client(i);
		}
		else
			send_message(ERR_NOSUCHNICK);
	}
	else
		send_message(ERR_NOPRIVILEGES);
}
*/

void Server::QUIT(void)
{
	std::vector<std::string> param = Client->split_packet;
	if (param.size() >= 2)
	{
		for (int i = 1; i < param.size(); i++)
		{
			std::cout << param[i];
			if (i < param.size() - 1)
				std::cout << " ";
		}
		std::cout << std::endl;
	}
	else
		std::cout << Client->nickname << " quits the server" << std::endl;
	remove_client();
}

void Server::SQUIT(void)
{
	exit(1);
}

int Server::parse_command()
{
	std::string command = Client->split_packet[0];
	if (command == "PASS")
		PASS();
	else if (command == "NICK")
		NICK();
	else if (command == "USER")
		USER();
	else
	{
		if (Client->registered)
		{
			if (command == "OPER")
				OPER();
			else if (command == "JOIN")
				JOIN();
			else if (command == "PART")
				PART();
			else if (command == "QUIT")
				QUIT();
			else if (command == "SQUIT")
				SQUIT();
			else if (command == "MODE")
				MODE();
			else
				return (ERROR);
		}
		else
			return (ERROR);
	}
	return (0);
}

void Server::parse_client_packet(std::string packet)
{
	std::string current_command;
	int newline_pos;

	Client = &_Clients[current_pfd - 1];
	Client->packet += packet;
	while ((newline_pos = Client->packet.find("\n")) != std::string::npos)
	{
		current_command = Client->packet.substr(0, newline_pos);
		current_command.erase(std::remove(current_command.begin(), current_command.end(), '\r'));

		std::cout << std::endl;
		std::cout << "# NEW PACKET FROM (current_pfd=" << current_pfd << ", fd=" << Client->fd << ", nick=" << Client->nickname << ")"<< std::endl;
		std::cout << Client->packet << std::endl;

		Client->packet.erase(0, newline_pos + 1);
		Client->split_packet = string_split(current_command, ' ');
		if (parse_command())
			send(Client->fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}