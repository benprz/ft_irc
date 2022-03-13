#pragma once

#include "main.hpp"
#include "Message.hpp"
#include "Client.hpp"
#include "Replies.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

#define DATA_BUFFER 5000
#define OPERATOR_PW std::string("passweird")
#define MAX_CONNECTIONS 1024

class Server
{
private:
Server();
//Real Server Param
fd_set read_fd_set;
int const _portnum;
std::string const _password;
char _buf[DATA_BUFFER];
Message mess;
std::vector<int> _socket_fd;     // i->0 = Server fd
struct sockaddr_in _saddr;

//tools for Server
std::vector<Client> _v_clients;
std::map<int, std::string> _m_fdbuffer;
std::map<int, Client> _m_fd2client; // from prefix to definitive clients


public:
// HOW TO USE : chan["#channel"][10] -> accees au 10eme client du chan "#channel"
std::map<std::string, std::vector<Client*> > chan;
std::map<std::string, std::string> chan_flag; // see todo pour flags possible sur channel
std::map<std::string, std::vector<Client*> > chan_oper;
std::map<std::string, std::string> chan_pass; // la on store les mdp

Server(int port, std::string pass);
Server(Server const & src);
virtual ~Server();
Server &  operator=( Server const & rhs );

std::string   ft_itoa(int nbr);
int create_tcp_server_socket(int port);
int launch(void);
void loop(void);
void on_connection(sockaddr_in new_addr, socklen_t addrlen, int new_fd);
void on_message(std::vector<int>::iterator it_fd, int* ret_val);
std::string executionner(std::string buf_str, Message &message, int fd);
int send_privmsg(int fd, std::string str);
void send_err(int fd, std::string err, std::string msg);
Client find_CfromFd(int fd);


//getters & setters
std::vector<Client> getClients(void) const {
        return(this->_v_clients);
}
void setClients(std::vector<Client> src) {
        this->_v_clients = src;
}
int getPort(void) const {
        return(this->_portnum);
}
std::string getPass(void) const {
        return(this->_password);
}

//Functions
int modecmd(Message &msg, int fd);
int partcmd(Message &msg, int fd);
int quitcmd(int fd);
int squitcmd(Message &msg, int fd);
int namecmd(Message &msg, int fd);
int kickcmd(Message &msg, int fd);
int listcmd(Message &msg, int fd);
void pingcmd(Message & msg, int fd);
std::string getmsg(Message &msg, int fd, std::string name);
void sendtoAll(int fd, std::vector<Client*> at, std::string msg);
void sendtoAllbutme(int fd, std::vector<Client*> at, std::string msg);
void privmsg(Message &msg, int fd);
void clearClient(int fd);
bool nick_check(std::string &nick, int fd);
int  nickcmd(Message msg, int fd);
int passcmd(Message & msg, int fd);
int usercmd(Message &msg, int fd);
int do_cmd(Message msg, int fd);
int joincmd(Message &msg, int fd);
int opercmd(Message &msg, int fd);
int killcmd(Message &msg, int fd);
bool  find_Cinchan(int fd, std::vector<Client*> vect);
std::list<std::string> split_every_char(std::string str, char separator);
void dispMemberName(int fd, std::string chan_name);
bool bufferComplete(int fd, char buf[5000]);
void remove_Cinchans(int fd);
void noticeNickInChans(int fd, std::string old_pref, std::string new_nick);
void updateOper(std::string old_nick, std::string new_nick, int fd);

};

// std::ostream &			operator<<( std::ostream & o, Server const & i );
