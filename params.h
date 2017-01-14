#ifndef PARAMS_H
#define PARAMS_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <dirent.h>
#include <sstream>
#include <vector>
#include <deque>

using namespace std;

const int PORT = 35787;
const int FIND_PORT = 35788;

const int HOST_NUMER = 10;
const int START_FRAGMENT_COUNT = 20;
const int TIMEOUT_SEC = 10;
const int TIMEOUT_SEC_FIND_AGAIN = 2;
const double GROW_PORTION = 1.2;
const int TOP_HOSTS = 3;



enum MsgType {UP_MSG, HAS_MSG, DEL_MSG, FORCE_DEL_MSG, FIND_MSG, DOWN_MSG, CANT_MSG, OWN_MSG, DONT_MSG};


#endif
