#include "ChannelsList.hpp"

ChannelsList::ChannelsList(std::string name)
{
	this->name = name;
	mode += 'n';
	users_limit = 0;
}

void	ChannelsList::add_user(int client_fd)
{
	if (!is_user_on_channel(client_fd))	
	{
		if (is_invite_only())
			remove_user_from_invite_list(client_fd);
		users.push_back(client_fd);
	}
}

void	ChannelsList::remove_user(int client_fd)
{
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == client_fd)
			users.erase(users.begin() + i);
	}
}

void	ChannelsList::add_operator(int client_fd)
{
	if (!is_user_operator(client_fd))
		operators.push_back(client_fd);
}

void	ChannelsList::remove_operator(int client_fd)
{
	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] == client_fd)
			operators.erase(operators.begin() + i);
	}
}

int		ChannelsList::has_mode(char mode)
{
	if (this->mode.find(mode) != std::string::npos)
		return (1);
	return (0);
}

int ChannelsList::is_user_invited(int client_fd)
{
	for (int i = 0; i < invited_users.size(); i++)
	{
		if (invited_users[i] == client_fd)
			return (1);
	}
	return (0);
}

int	ChannelsList::is_user_on_channel(int client_fd)
{
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == client_fd)
			return (1);
	}
	return (0);
}

int	ChannelsList::is_user_operator(int client_fd)
{
	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] == client_fd)
			return (1);
	}
	return (0);
}

int	ChannelsList::is_users_limit_reached()
{
	if (users.size() == users_limit)
		return (1);
	return (0);
}

void	ChannelsList::add_user_to_invite_list(int client_fd)
{
	invited_users.push_back(client_fd);
}


void	ChannelsList::remove_user_from_invite_list(int client_fd)
{
	 for (int i = 0; i < invited_users.size(); i++)
	 {
		 if (invited_users[i] == client_fd)
		 {
			invited_users.erase(invited_users.begin() + i);
			break ;
		 }
	 }
}

void ChannelsList::set_key(std::string key)
{
	this->key = key;
	mode += 'k';
}

std::string	ChannelsList::add_or_remove_mode(char action, char mode, std::string third_param, Server &serv)
{
	std::cout << "action=" << action << " mode=" << mode << std::endl;

	if (static_cast<std::string>(CHANNEL_MODES).find(mode) == std::string::npos)
		return (ERR_UNKNOWNMODE);
	else
	{
		int mode_pos = this->mode.find(mode);

		if (mode == 'o')
		{
			int client_fd = serv.get_client_fd(third_param);
			if (client_fd >= 0)
			{
				if (action == '+')
					add_operator(client_fd);
				else
					remove_operator(client_fd);
			}
		}
		else if (mode == 'l')
		{
			if (action == '+')
			{
				if (!is_limited() && third_param != "")
				{
					for (int i = 0; i < third_param.size(); i++)
					{
						if (!std::isdigit(third_param[i]))
							return "";
					}
					users_limit = std::stoi(third_param);
					this->mode += 'l';
				}
			}
			else if (is_limited())
				this->mode[mode_pos] = ' ';
		}
		else if (mode == 'k')
		{
			if (action == '+')
			{
				if (is_restricted_by_key())
					return (ERR_KEYSET);
				else if (third_param == "")
					return (ERR_NEEDMOREPARAMS);
				else
					set_key(third_param);
			}
			else if (is_restricted_by_key())
				this->mode[mode_pos] = ' ';
		}
		else
		{
			if (action == '+')
			{
				if (!has_mode(mode))
					this->mode += mode;
			}
			else if (has_mode(mode))
				this->mode[mode_pos] = ' ';
		}

	}
	this->mode.erase(std::remove_if(this->mode.begin(), this->mode.end(), ::isspace));
	return ("");
}