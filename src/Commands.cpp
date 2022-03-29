#include "Server.hpp"
#include "NumericReplies.hpp"

void Server::send_message(std::string numeric_reply)
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
		else if (numeric_reply == ERR_NOTONCHNANEL)
			message += Client->split_packet[1] + " :You're not on that channel";
	}

	message += CRLF;
	send(Client->fd, message.c_str(), message.size(), 0);
}

void Server::PASS()
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

int Server::check_if_nickname_is_already_used(std::string nickname)
{
	for (int i = 0; i < nfds - 1; i++)
	{
		if (_Clients[i].nickname == nickname)
			return (1);
	}
	return (0);
}

int Server::check_if_nickname_is_erroneous(std::string nickname)
{
	if (nickname.size() <= 9)
	{
		size_t invalid_char = nickname.find_first_not_of("AZERTYUIOPQSDFGHJKLMWXCVBNazertyuiopqsdfghjklmwxcvbn1234567890[]\\`_^{}|");
		if (invalid_char == std::string::npos)
			return (0);
	}
	return (1);
}

void Server::NICK()
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

void Server::USER()
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
		int i = 1;
		while (_Clients[i].nickname != Client->split_packet[1] && i <= MAX_ALLOWED_CLIENTS)
			i++;
		if (i <= MAX_ALLOWED_CLIENTS)
		_Clients[i].mode += 'o';
		send_message(RPL_YOUREOPER);
		std::cout << _Clients[i].nickname << " " << _Clients[i].mode << std::endl; // del
	}
}

void	Server::printchannels()
{
	std::cout << std::endl << "|@@@@@@@@@@@@@@@@@@@@@@@@@\n";
	for (int i = 0; i < MAX_ALLOWED_CHANNELS; i++)
	{
		if (_Channels[i].name != "")
		{
			std::cout << "name=" << _Channels[i].name << std::endl;
			std::cout << "password=" << _Channels[i].password << std::endl;
			std::cout << "mode=" << _Channels[i].mode << std::endl;
			std::cout << "users=";
			for (int j = 0; j < _Channels[i].users.size(); j++)
			{
				std::cout << _Channels[i].users[j] << ", ";
			}
			std::cout << "\n-\n";
		}
	}
	std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@|\n\n";
}

int	Server::get_channel_id(std::string channel)
{
	for (int i = 0; i < MAX_ALLOWED_CHANNELS; i++)
	{
		if (_Channels[i].name == channel)
			return (i);
		if (_Channels[i].name == "")
			break ;
	}
	return (ERROR);
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
				if (_Channels[channel_id].is_user_on_channel(current_pfd))
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
					std::string part_message = ":" + get_current_client_prefix() + " PART " + _Channels[channel_id].name + part_reason;
					for (int j = 0; j < _Channels[channel_id].users.size(); j++)
					{
						Client = &_Clients[_Channels[channel_id].users[j]];
						send_message(part_message);
					}
					std::cout << "Removed user=" << current_pfd << " from channel name=" << _Channels[channel_id].name << " chanid=" << channel_id << std::endl;
					_Channels[channel_id].remove_user(current_pfd);
					printchannels();
				}
				else
					send_message(ERR_NOTONCHNANEL);
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
	else
	{
		std::vector<std::string> split_channels = string_split(Client->split_packet[1], ',');
		std::vector<std::string> split_passwords;
		int channel_id;
		if (Client->split_packet.size() == 3)
			split_passwords = string_split(Client->split_packet[2], ',');
		for (int i = 0; i < split_channels.size(); i++)
		{
			Client->split_packet[1] = split_channels[i];
			if (split_channels[i][0] != '#')
				send_message(ERR_NOSUCHCHANNEL);
			else
			{
				std::string join_message = ":" + get_current_client_prefix() + " JOIN " + split_channels[i];
				if ((channel_id = get_channel_id(split_channels[i])) != ERROR)
				{
					std::cout << "join channel_id=" << channel_id << std::endl;
					std::string err;

					if (_Channels[channel_id].mode.find('l') != std::string::npos)
					{
						if (_Channels[channel_id].is_users_limit_reached())
							err = ERR_CHANNELISFULL;
					}
					else if (_Channels[channel_id].is_invite_only())
					{
						if (!_Channels[channel_id].is_user_invited(current_pfd))
							err = ERR_INVITEONLYCHAN;
					}
					else if (_Channels[channel_id].mode.find('k') != std::string::npos)
					{
						if (i >= split_passwords.size() || _Channels[channel_id].password != split_passwords[i])
							err = ERR_BADCHANNELKEY;
					}
					if (err == "")
					{
						_Channels[channel_id].add_user(current_pfd);
						for (int j = 0; j < _Channels[channel_id].users.size(); j++)
						{
							Client = &_Clients[_Channels[channel_id].users[j]];
							send_message(join_message);
						}
						Client->opened_channels++;
					}
					else
					{
						send_message(err);
						continue ;
					}
				}
				else
				{
					if ((channel_id = get_channel_id("")) >= 0)
					{
						_Channels[channel_id].name = split_channels[i];
						_Channels[channel_id].mode += 'n';
						if (i < split_passwords.size())
						{
							_Channels[channel_id].password = split_passwords[i];
							_Channels[channel_id].mode += 'k';
						}
						_Channels[channel_id].add_user(current_pfd);
						send_message(join_message);
						//_Channels[channel_id].operators.push_back(current_pfd);
						Client->opened_channels++;
					}
					else
						send_message(ERR_TOOMANYCHANNELS);
				}
			}
		}
	}
	printchannels();
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

	Client = &_Clients[current_pfd];
	Client->packet += packet;
	while ((newline_pos = Client->packet.find("\n")) != std::string::npos)
	{
		current_command = Client->packet.substr(0, newline_pos);
		current_command.erase(std::remove(current_command.begin(), current_command.end(), '\r'));
		std::cout << "packet=\"" << Client->packet << "\"\n";
		Client->packet.erase(0, newline_pos + 1);
		Client->split_packet = string_split(current_command, ' ');
		if (parse_command())
			send(Client->fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}