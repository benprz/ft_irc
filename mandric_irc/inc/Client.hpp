#pragma once

#include <iostream>
#include <iostream>
#include <vector>
#include <map>
#include "main.hpp"
// #include "Server.hpp"
// #include "Message.hpp"

class Client
{

public:

Client();
Client(sockaddr_in inf, socklen_t len, int fd);
Client( Client const & src );
virtual ~Client();

Client &  operator=( Client const & rhs );

sockaddr_in getInfo(void) { return(this->info); }
std::string getNick(void) { return(this->nickname); }
std::string getPass(void) { return(this->password); }
std::string getReal(void) { return(this->realname); }
std::string getUser(void) { return(this->username); }
std::string getCprefix(void) { return(this->cprefix); }
bool getCorr(void) { return(this->corr_pass); }
bool getReg(void) { return(this->is_registered); }
bool getNickstatus(void) { return(this->nickSet); }
bool getUserstatus(void) { return(this->userSet); }
int getFd(void) { return(this->fd); }
int getOp(void) { return(this->op); }

void setReg(bool src) { this->is_registered = src; }
void setNickstatus(bool src) { this->nickSet = src; }
void setUserstatus(bool src) { this->userSet = src; }
void setCorr(bool src) { this->corr_pass = src; }
void setOp(bool src) { this->op = src; }
void setNick(std::string src) { this->nickname = src; }
void setUser(std::string src) { this->username = src; }
void setReal(std::string src) { this->realname = src; }
void setPass(std::string src) { this->password = src; }
void setInfo(sockaddr_in src) { this->info = src; }
void setFd(int src) { this->fd = src; }
void setCprefix(std::string src) { this->cprefix = src; }

private:
sockaddr_in info;
socklen_t len;
int fd;
std::string cprefix;
std::string nickname;
std::string username;
std::string realname;
std::string password;
bool  is_registered;
bool  corr_pass;
bool  nickSet;
bool  userSet;
bool  op;


};

// std::ostream &			operator<<( std::ostream & o, Client const & i );
