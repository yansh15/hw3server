#ifndef SERVER_CONST_H
#define SERVER_CONST_H

#include <cstdint>

const uint16_t PORT = 8040;
const int MAXCLIENTNUM = 20;

// Request op
const int REGISTEROP = 0;
const int LOGINOP = 1;
const int QUITOP = 2;
const int SEARCHOP = 3;
const int ADDOP = 4;
const int SENDMESSAGEOP = 5;

// Public status
const int SUCCESS = 0;

// Register status
const int USERNAMEEXIST = 2;

// Login status
const int USERNAMENOTEXIST = 2;
const int PASSWORDWRONG = 3;

#endif //SERVER_CONST_H
