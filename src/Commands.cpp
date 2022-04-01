#include "Server.hpp"
#include "NumericReplies.hpp"

void Server::send_message(int fd, std::string numeric_reply)
{
	std::string message;

	if (numeric_reply.length() != 3)
		message = numeric_reply;
	else
	{
		message = static_cast<std::string>(":") + HOSTNAME + " " + numeric_reply + " " + Client->nickname + " ";
		if (numeric_reply == RPL_WELCOME)
		{
			Client->registered = 1;
			message += ":Welcome to the Internet Relay Network " + Client->nickname + "!" + Client->username + "@" + HOSTNAME;
		}
		else if (numeric_reply == RPL_YOUREOPER)
			message += ":You are now an IRC operator";
		else if (numeric_reply == RPL_INVITING)
			message += Client->split_command[2] + " " + Client->split_command[1];
		else if (numeric_reply == RPL_NAMREPLY)
		{
			if (Client->split_command[0][0] == '*')
			{
				message += "* * :" + _Clients[0].nickname;
				for (int i = 1; i < _Clients.size(); i++)
					message += " " + _Clients[i].nickname;
			}
			else
			{
				int channel_id = get_channel_id(Client->split_command[0]);
				message += "= " + Client->split_command[0] + " :";
				for (int i = 0; i < _Channels[channel_id].users.size(); i++)
				{
					if (!_Clients[get_client_id(_Channels[channel_id].users[i])].is_invisible())
					{
						if (_Channels[channel_id].is_user_operator(_Channels[channel_id].users[i]))
							message += '@';
						message += _Clients[get_client_id(_Channels[channel_id].users[i])].nickname + " ";
					}
				}
			}
		}
		else if (numeric_reply == RPL_ENDOFNAMES)
			message += Client->split_command[0] + " :End of /NAMES list";
		else if (numeric_reply == RPL_LISTSTART)
			message += "Channel :Users  Name";
		else if (numeric_reply == RPL_LIST)
		{
			int channel_id = get_channel_id(Client->split_command[0]);
			message += Client->split_command[0] + " " + std::to_string(count_visible_users_on_channel(channel_id)) + " :" + _Channels[channel_id].topic;
		}
		else if (numeric_reply == RPL_LISTEND)
			message += ":End of /LIST";
		else if (numeric_reply == RPL_CHANNELMODEIS)
			message += Channel->name + " +" + Channel->modes;
		else if (numeric_reply == RPL_NOTOPIC)
			message += Client->split_command[0] + " :No topic is set";
		else if (numeric_reply == RPL_TOPIC)
		{
			int channel_id = get_channel_id(Client->split_command[0]);
			message += Client->split_command[0] + " :" + _Channels[channel_id].topic;
		}
		else if (numeric_reply == ERR_UNKNOWNCOMMAND)
			message += Client->current_command + " :Unknown command";
		else if (numeric_reply == ERR_NOTREGISTERED)
			message += ":You have not registered";
		else if (numeric_reply == ERR_NEEDMOREPARAMS)
			message += Client->split_command[0] + " :Not enough parameters";
		else if (numeric_reply == ERR_ALREADYREGISTRED)
			message += ":You may not reregister";
		else if (numeric_reply == ERR_NONICKNAMEGIVEN)
			message += ":No nickname given";
		else if (numeric_reply == ERR_ERRONEUSNICKNAME)
			message += Client->split_command[1] + " :Erroneus nickname";
		else if (numeric_reply == ERR_NICKNAMEINUSE)
			message += Client->split_command[1] + " :Nickname is already in use";
		else if (numeric_reply == ERR_PASSWDMISMATCH)
			message += ":Password incorrect";
		else if (numeric_reply == ERR_NOOPERHOST)
			message += ":No O-lines for your host";
		else if (numeric_reply == ERR_INVITEONLYCHAN)
			message += Client->split_command[0] + " :Cannot join channel (+i)";
		else if (numeric_reply == ERR_TOOMANYCHANNELS)
			message += Client->split_command[0] + " :You have joined too many channels";
		else if (numeric_reply == ERR_NOSUCHCHANNEL)
			message += Client->split_command[0] + " :No such channel";
		else if (numeric_reply == ERR_BADCHANNELKEY)
			message += Client->split_command[0] + " :Cannot join channel (+k)";
		else if (numeric_reply == ERR_CHANNELISFULL)
			message += Client->split_command[0] + " :Cannot join channel (+l)";
		else if (numeric_reply == ERR_NOTONCHANNEL)
			message += Client->split_command[0] + " :You're not on that channel";
		else if (numeric_reply == ERR_NOPRIVILEGES)
			message += ":Permission Denied- You're not an IRC operator";
		else if (numeric_reply == ERR_CHANOPRIVSNEEDED)
			message += Client->split_command[1] + " :You're not channel operator";
		else if (numeric_reply == ERR_KEYSET)
			message += Client->split_command[1] + " :Channel key already set";
		else if (numeric_reply == ERR_UNKNOWNMODE)
			message += Client->split_command[2][0] + static_cast<std::string>(" :is unknown mode char to me");
		else if (numeric_reply == ERR_USERSDONTMATCH)
			message += ":Cant change mode for other users";
		else if (numeric_reply == ERR_UMODEUNKNOWNFLAG)
			message += ":Unknown MODE flag";
		else if (numeric_reply == ERR_USERONCHANNEL)
			message += Client->split_command[1] + " " + Client->split_command[2] + " :is already on channel";
		else if (numeric_reply == ERR_NOSUCHNICK)
			message += Client->split_command[1] + " :No such nick/channel";
		else if (numeric_reply == ERR_NORECIPIENT)
			message += ":No recipient given (" + Client->current_command + ")";
		else if (numeric_reply == ERR_NOTEXTTOSEND)
			message += ":No text to send";

	}

	message += CRLF;
	send(fd, message.c_str(), message.size(), 0);
}

void Server::send_message(std::string numeric_reply)
{
	send_message(Client->fd, numeric_reply);
}

int Server::count_visible_users_on_channel(int channel_id)
{
	int count = 0;
	for (int i = 0; i < _Channels[channel_id].users.size(); i++)
	{
		if (!_Clients[get_client_id(_Channels[channel_id].users[i])].is_invisible())
			count++;
	}
	return (count);
}

void Server::PASS()
{
	Client->logged = 0;
	if (Client->registered)
		send_message(ERR_ALREADYREGISTRED);
	else if (Client->split_command.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_command[1] != _password)
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
	if (Client->split_command.size() < 2)
		send_message(ERR_NONICKNAMEGIVEN);
	else
	{
		std::string nick = Client->split_command[1];
		if (nick.size() <= 9 && nick.find_first_not_of(NICK_CHARSET) == std::string::npos)
		{
			if (get_client_id(Client->split_command[1]) == ERROR)
			{
				if (Client->registered)
					send_message(Client->get_prefix() + " NICK " + Client->split_command[1]);
				Client->nickname = Client->split_command[1];
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
	else if (Client->split_command.size() < 4)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		//on ignore _split_packet[2] && [3] parce que c'est pour les communications server-server
		Client->username = Client->split_command[1];
		for (int i = 5; i < Client->split_command.size(); i++)
		{
			Client->split_command[4] += " ";
			Client->split_command[4] += Client->split_command[i];
		}
		Client->split_command[4].erase(0, 1);
		Client->realname = Client->split_command[4];
		if (Client->logged && Client->nickname != "")
			send_message(RPL_WELCOME);
	}
}

void Server::OPER()
{
	if (OPER_HOST == 0)
		send_message(ERR_NOOPERHOST);
	else if (Client->split_command.size() < 3)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_command[2] != OPER_PASSWD)
		send_message(ERR_PASSWDMISMATCH);
	else
	{
		int client_id = get_client_id(Client->split_command[1]);

		if (client_id >= 0 && _Clients[client_id].mode.find('o') == std::string::npos)
		{
			_Clients[client_id].mode += 'o';
			send_message(_Clients[client_id].fd, RPL_YOUREOPER);
		}
	}
}

int	Server::add_or_remove_special_mode_channel(char action, char mode, std::string param)
{
	std::string ret;

	if (mode == 'o')
	{
		int client_fd = get_client_fd(param);
		if (client_fd >= 0)
		{
			if (action == '+')
				ret = Channel->add_operator(client_fd);
			else
				ret = Channel->remove_operator(client_fd);
		}
		else
			return (ERROR);
	}
	else if (mode == 'l')
	{
		if (action == '+')
		{
			for (int i = 0; i < param.size(); i++)
			{
				if (!std::isdigit(param[i]))
					return (ERROR);
			}
			Channel->users_limit = std::stoi(param);
			Channel->add_mode(mode);
			ret = "+l";
		}
		else
			ret = Channel->remove_mode(mode);
	}
	else if (mode == 'k')
	{
		if (action == '+')
		{
			Channel->key = param;
			Channel->add_mode(mode);
			ret = "+k";
		}
		else
			ret = Channel->remove_mode(mode);
	}
	if (ret != "")
		return (1);
	return (0);
}

void	Server::edit_channel_modes()
{
	if (Channel->is_user_operator(Client->fd))
	{
		char action = '+';
		char mode = 0;
		std::string edited_modes;
		int current_param = 3;
		std::string params_list;
		int ret;
		for (int i = 0; i < Client->split_command[2].size(); i++)
		{
			if (Client->split_command[2][i] == '+' || Client->split_command[2][i] == '-')
			{
				action = Client->split_command[2][i];
				continue ;
			}
			if (action == '+' || action == '-')
			{
				Client->split_command[2][0] = Client->split_command[2][i];
				mode = Client->split_command[2][i];
				if (!Channel->is_mode(mode))
					send_message(ERR_UNKNOWNMODE);
				else
				{
					if (mode == 'o' || mode == 'l' || mode == 'k')
					{
						if ((action == '+' && Client->split_command.size() < current_param) || \
							(ret = add_or_remove_special_mode_channel(action, mode, Client->split_command[current_param])) == ERROR)
							break ;
						else if (ret == 0)
							continue ;
						else if (action == '+')
							edited_modes += static_cast<std::string>("+") + mode;
						else
							edited_modes += static_cast<std::string>("-") + mode;
						if (mode == 'o' || action == '+')
						{
							params_list += " " + Client->split_command[current_param];
						}
						current_param++;
					}
					else if (action == '+')
						edited_modes += Channel->add_mode(mode);
					else
						edited_modes += Channel->remove_mode(mode);
				}
			}
		}
		if (edited_modes.size())
			send_message_to_channel(Client->get_prefix() + " MODE " + Channel->name + " " + edited_modes + params_list);
		printchannels();
	}
	else
		send_message(ERR_CHANOPRIVSNEEDED);
}

void	Server::MODE()
{
	if (Client->split_command.size() == 1)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_command.size() >= 2)
	{
		if (Client->split_command[1][0] == '#')
		{
			int channel_id = get_channel_id(Client->split_command[1]);
			if (channel_id >= 0)
			{
				Channel = &_Channels[channel_id];
				if (Client->split_command.size() == 2)
					send_message(RPL_CHANNELMODEIS);
				else
					edit_channel_modes();
			}
			else
				send_message(ERR_NOSUCHCHANNEL);
		}
	}
	/*
	else
	{
		int client_id = get_client_id(Client->split_command[1]);
		if (client_id >= 0 && _Clients[client_id].fd == Client->fd)
		{
			char action = Client->split_command[2][0];
			if (action == '+' || action == '-')
			{
				for (int i = 1; i < Client->split_command[2].size(); i++)
				{
					if (static_cast<std::string>(USER_MODES).find(Client->split_command[2][i]) != std::string::npos)
					{
						if (action == '+' && Client->split_command[2][i] == 'i')
							Client->mode += 'i';
						else if (action == '-')
						{
							int pos = Client->mode.find(Client->split_command[2][i]);
							if (pos != std::string::npos)
								Client->mode.erase(Client->mode.begin() + pos);
						}
						if (Client->split_command[2][i] != 'o' && action != '+')
							send_message(Client->nickname + " MODE " + action + Client->split_command[2][i]);
					}
					else
						send_message(ERR_UMODEUNKNOWNFLAG);
				}
			}
		}
		else
			send_message(ERR_USERSDONTMATCH);
	}
	*/
}

void	Server::PRIVMSG()
{
	if (Client->split_command.size() < 2)
		send_message(ERR_NORECIPIENT);
	else if (Client->split_command.size() < 3)
		send_message(ERR_NOTEXTTOSEND);
	else
	{
		if (Client->split_command[2][0] == ':')
		{
			std::string prefix = Client->get_prefix() + " PRIVMSG ";

			std::string text = Client->split_command[2];
			for (int i = 3; i < Client->split_command.size(); i++)
				text += " " + Client->split_command[i];

			std::vector<std::string> split_receivers = string_split(Client->split_command[1], ',');
			for (int i = 0; i < split_receivers.size(); i++)
			{
				Client->split_command[1] = split_receivers[i];
				std::string message = prefix + split_receivers[i] + " " + text;
				if (split_receivers[i][0] == '#')
				{
					int channel_id = get_channel_id(split_receivers[i]);
					if (channel_id >= 0)
						send_message_to_channel(channel_id, message);
				}
				else
				{
					int client_fd = get_client_fd(split_receivers[i]);
					if (client_fd >= 0)
						send_message(client_fd, message);
				}
			}
		}
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
			std::cout << "	mode=" << _Channels[i].modes << std::endl;
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
			std::cout << "	topic=" << _Channels[i].topic << std::endl;
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

int	Server::get_client_id(int fd)
{
	for (int i = 0; i < _Clients.size(); i++)
	{
		if (_Clients[i].fd == fd)
			return (i);
	}
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
	if (!_Channels[channel_id].is_restricted_to_outsiders() || (_Channels[channel_id].is_restricted_to_outsiders() && _Channels[channel_id].is_user_on_channel(Client->fd)))
	{
		if (!_Channels[channel_id].is_moderated() || (_Channels[channel_id].is_moderated() && _Channels[channel_id].is_user_operator(Client->fd)))
		{
			for (int j = 0; j < _Channels[channel_id].users.size(); j++)
				send_message(_Channels[channel_id].users[j], message);
		}
	}
}

void	Server::send_message_to_channel(std::string message)
{
	send_message_to_channel(get_channel_id(Channel->name), message);
}

void	Server::add_client_to_chan(int channel_id)
{
	_Channels[channel_id].add_user(Client->fd);
	Client->opened_channels++;
	send_message_to_channel(channel_id, Client->get_prefix() + " JOIN :" + _Channels[channel_id].name);
	if (_Channels[channel_id].topic != "")
		send_message(RPL_TOPIC);
	send_message(RPL_NAMREPLY);
	send_message(RPL_ENDOFNAMES);
	std::cout << "Added user fd=" << Client->fd << " to channel name=" << _Channels[channel_id].name << " chanid=" << channel_id << std::endl;
	printchannels();
}

void	Server::remove_client_from_chan(int channel_id, std::string reason)
{
	send_message_to_channel(channel_id, ":" + Client->get_prefix() + " PART " + _Channels[channel_id].name + reason);
	_Channels[channel_id].remove_user(Client->fd);
	Client->opened_channels--;
	if (_Channels[channel_id].users.size() == 0)
		_Channels.erase(_Channels.begin() + channel_id);
	std::cout << "Removed user fd=" << Client->fd << " from channel name=" << _Channels[channel_id].name << " chanid=" << channel_id << std::endl;
	printchannels();
}

void	Server::INVITE()
{
	if (Client->split_command.size() < 3)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		int channel_id = get_channel_id(Client->split_command[2]);
		if (channel_id >= 0)
		{
			Client->split_command[0] = Client->split_command[2];
			if (_Channels[channel_id].is_user_on_channel(Client->fd))
			{
				if (_Channels[channel_id].is_user_operator(Client->fd))
				{
					int client_fd = get_client_fd(Client->split_command[1]);
					if (client_fd >= 0)
					{
						if (!_Channels[channel_id].is_user_on_channel(client_fd))
						{
							_Channels[channel_id].add_user_to_invite_list(client_fd);
							send_message(client_fd, Client->nickname + " INVITE " + Client->split_command[1] + " " + Client->split_command[2]);
							send_message(RPL_INVITING);
						}
						else
							send_message(ERR_USERONCHANNEL);
					}
					else
						send_message(ERR_NOSUCHNICK);
				}	
				else
					send_message(ERR_CHANOPRIVSNEEDED);
			}
			else
				send_message(ERR_NOTONCHANNEL);
		}
		else
		{
			Client->split_command[1] = Client->split_command[2];
			send_message(ERR_NOSUCHNICK);
		}
	}
}

void	Server::PART()
{
	int channel_id;

	if (Client->split_command.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		std::vector<std::string> split_channels = string_split(Client->split_command[1], ',');
		for (int i = 0; i < split_channels.size(); i++)
		{
			Client->split_command[0] = split_channels[i];
			if ((channel_id = get_channel_id(split_channels[i])) >= 0)
			{
				if (_Channels[channel_id].is_user_on_channel(Client->fd))
				{
					std::string part_reason;
					if (Client->split_command.size() > 2)
					{
						part_reason += " :";
						for (int i = 2; i < Client->split_command.size(); i++)
						{
							part_reason += Client->split_command[i];
							if (i < Client->split_command.size() - 1)
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
	if (Client->split_command.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else if (Client->split_command[1] == "0")
		remove_client_from_all_chans();
	else
	{
		std::vector<std::string> split_channels = string_split(Client->split_command[1], ',');
		std::vector<std::string> split_keys;
		int channel_id;
		if (Client->split_command.size() == 3)
			split_keys = string_split(Client->split_command[2], ',');
		for (int i = 0; i < split_channels.size(); i++)
		{
			if (split_channels[i][0] != '#')
				send_message(ERR_NOSUCHCHANNEL);
			else
			{
				Client->split_command[0] = split_channels[i];
				if (Client->opened_channels == MAX_ALLOWED_CHANNELS_PER_CLIENT)
				{
					send_message(ERR_TOOMANYCHANNELS);
					break ;
				}
				if ((channel_id = get_channel_id(split_channels[i])) != ERROR)
				{
					std::string err;

					if (_Channels[channel_id].is_limited() && _Channels[channel_id].is_users_limit_reached())
						err = ERR_CHANNELISFULL;
					else if (_Channels[channel_id].is_invite_only() && !_Channels[channel_id].is_user_invited(Client->fd))
						err = ERR_INVITEONLYCHAN;
					else if (_Channels[channel_id].is_restricted_by_key() && (i >= split_keys.size() || _Channels[channel_id].key != split_keys[i]))
						err = ERR_BADCHANNELKEY;
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
					_Channels[channel_id].add_operator(Client->fd);
					add_client_to_chan(channel_id);
				}
			}
		}
	}
}

void Server::KILL()
{
	if (Client->split_command.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	if (!Client->is_operator())
		send_message(ERR_NOPRIVILEGES);
	else
	{
		int client_id = get_client_id(Client->split_command[1]);
		if (client_id >= 0)
			remove_client(client_id);
		else
			send_message(ERR_NOSUCHNICK);
	}
}

void Server::KICK(void)
{
	int channel_id = -1;
	std::vector<std::string> param = Client->split_command;
	std::size_t found = Client->mode.rfind("o");
	if (found != std::string::npos)
	{
		send_message(ERR_CHANOPRIVSNEEDED);
		return;
	}
	if (param.size() < 2)
	{
		send_message(ERR_NEEDMOREPARAMS);
		return;
	}
	channel_id = get_channel_id(param[0]);
	if (channel_id == -1)
	{
		send_message(ERR_BADCHANMASK);
		return;
	}
	int client_id = get_client_id(param[1]);
	int client_fd = _Clients[client_id].fd;
	if (_Channels[channel_id].is_user_on_channel(client_fd))
		_Channels[channel_id].remove_user(client_fd);
	else
		send_message(ERR_NOTONCHANNEL);
	if (param.size() == 3)
		std::cout << param[2] << std::endl;
}

void Server::QUIT(void)
{
	std::string message = Client->nickname + " QUIT";
	if (Client->split_command.size() > 1 && Client->split_command[1][0] == ':')
	{
		Client->split_command[1].erase(Client->split_command[1].begin());
		for (int i = 1; i < Client->split_command.size(); i++)
			message += " " + Client->split_command[i];
	}
	else
		message += " leaving the server";
	send_message(message);
	remove_client();
}

void Server::LIST()
{
	send_message(RPL_LISTSTART);
	if (Client->split_command.size() == 1)
	{
		for (int i = 0; i < _Channels.size(); i++)
		{
			if (_Channels[i].is_user_on_channel(Client->fd) || !_Channels[i].is_secret())
			{
				Client->split_command[0] = _Channels[i].name;
				send_message(RPL_LIST);	
			}
		}
	}
	else
	{
		std::vector<std::string> split_channels = string_split(Client->split_command[1], ',');
		int channel_id;
		for (int i = 0; i < split_channels.size(); i++)
		{
			if ((channel_id = get_channel_id(split_channels[i])) >= 0)
			{
				if (_Channels[channel_id].is_user_on_channel(Client->fd) || !_Channels[channel_id].is_secret())
				{
					Client->split_command[0] = split_channels[i];
					send_message(RPL_LIST);	
				}
			}
		}
	}
	send_message(RPL_LISTEND);
}

void Server::NAMES()
{
	int channel_id;

	if (Client->split_command.size() > 1)
	{
		std::vector<std::string> split_channels = string_split(Client->split_command[1], ',');
		for (int i = 0; i < split_channels.size(); i++)
		{
			if ((channel_id = get_channel_id(split_channels[i])) >= 0)
			{
				if (_Channels[channel_id].is_secret() == 0)
				{
					Client->split_command[0] = split_channels[i];
					send_message(RPL_NAMREPLY);
				}
			}
		}
		Client->split_command[0] = split_channels[0];
		for (int i = 1; i < split_channels.size(); i++)
			Client->split_command[0] += " " + split_channels[i];
	}
	else
	{
		for (int i = 0; i < _Channels.size(); i++)
		{
			if (_Channels[i].is_secret() == 0)
			{
				Client->split_command[0] = _Channels[i].name;
				send_message(RPL_NAMREPLY);
			}
		}
		Client->split_command[0] = '*';
	}
	Client->split_command[0] = "*";
	send_message(RPL_NAMREPLY);
	send_message(RPL_ENDOFNAMES);
}

void Server::TOPIC()
{
	if (Client->split_command.size() < 2)
		send_message(ERR_NEEDMOREPARAMS);
	else
	{
		int channel_id = get_channel_id(Client->split_command[1]);
		if (channel_id >= 0)
		{
			if (Client->split_command.size() > 2)
			{
				if (_Channels[channel_id].is_user_on_channel(Client->fd))
				{
					if (_Channels[channel_id].is_user_operator(Client->fd))
					{
						std::string topic;
						for (int i = 2; i < Client->split_command.size(); i++)
							topic += Client->split_command[i];
						_Channels[channel_id].topic = topic;
						send_message_to_channel(channel_id, Client->nickname + " " + Client->current_command);
					}
					else
						send_message(ERR_CHANOPRIVSNEEDED);
				}
				else
					send_message(ERR_NOTONCHANNEL);
			}
			else
			{
				Client->split_command[0] = Client->split_command[1];
				if (_Channels[channel_id].topic == "")
					send_message(RPL_NOTOPIC);
				else
					send_message(RPL_TOPIC);
			}
		}
	}
}

int Server::parse_command()
{
	std::string command = Client->split_command[0];
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
			else if (command == "MODE")
				MODE();
			else if (command == "INVITE")
				INVITE();
			else if (command == "KILL")
				KILL();
			else if (command == "KICK")
				KICK();
			else if (command == "NAMES")
				NAMES();
			else if (command == "PRIVMSG")
				PRIVMSG();
			else if (command == "NOTICE")
				PRIVMSG();
			else if (command == "LIST")
				LIST();
			else if (command == "TOPIC")
				TOPIC();
			else if (command == "PONG")
				;
			else
				return (ERROR);
		}
		else
			return (ERROR);
	}
	return (0);
}
