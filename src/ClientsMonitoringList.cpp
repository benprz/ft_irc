#include "ClientsMonitoringList.hpp"

ClientsMonitoringList::ClientsMonitoringList(int fd)
{
	this->fd = fd; 
	logged = 0;
	registered = 0;
	opened_channels = 0;
}

int	ClientsMonitoringList::is_operator()
{
	return (mode.find('o') != std::string::npos ? 1 : 0);
}