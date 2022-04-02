#include "ChannelsList.hpp"

ChannelsList::ChannelsList(std::string name)
{
	this->name = name;
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
		{
			users.erase(users.begin() + i);
			remove_operator(client_fd);
		}
	}
}

std::string	ChannelsList::add_operator(int client_fd)
{
	if (!is_user_operator(client_fd))
	{
		operators.push_back(client_fd);
		return (static_cast<std::string>("+") + 'o');
	}
	return "";
}

std::string	ChannelsList::remove_operator(int client_fd)
{
	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] == client_fd)
		{
			operators.erase(operators.begin() + i);
			return (static_cast<std::string>("-") + 'o');
		}
	}
	return "";
}

int		ChannelsList::is_mode(char mode)
{
	if (static_cast<std::string>(CHANNEL_MODES).find(mode) != std::string::npos)
		return (1);
	return (0);
}

int		ChannelsList::has_mode(char mode)
{
	if (this->modes.find(mode) != std::string::npos)
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
	modes += 'k';
}

std::string	ChannelsList::add_mode(char mode)
{
	if (!has_mode(mode))
	{
		this->modes += mode;
		return (static_cast<std::string>("+") + mode);
	}
	return "";
}

std::string	ChannelsList::remove_mode(char mode)
{
	if (has_mode(mode))
	{
		this->modes.erase(this->modes.begin() + this->modes.find(mode));
		return (static_cast<std::string>("-") + mode);
	}
	return "";
}