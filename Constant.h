#ifndef SERVER_CONST_H
#define SERVER_CONST_H

#include <cstdint>

const uint16_t PORT = 8052;
const int MAXCLIENTNUM = 20;

const int FILEBLOCKSIZE = 65536;

// Request op
const int REGISTEROP = 0;
const int LOGINOP = 1;
const int QUITOP = 2;
const int SEARCHOP = 3;
const int ADDOP = 4;
const int SENDMESSAGEOP = 5;
const int SENDFILEOP = 6;
const int SENDFILEDATASTARTOP = 7;
const int SENDFILEDATAOP = 8;
const int SENDFILEDATAENDOP = 9;
const int RECEIVEFILEOP = 10;
const int RECEIVEFILEDATASTARTOP = 11;
const int RECEIVEFILEDATAOP = 12;
const int RECEIVEFILEDATAENDOP = 13;

// Public status
const int SUCCESS = 0;

// Register status
const int USERNAMEEXIST = 2;

// Login status
const int USERNAMENOTEXIST = 2;
const int PASSWORDWRONG = 3;

#endif //SERVER_CONST_H
