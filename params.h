/*
 * Plik z parametrami z których korzystamy w programie.
 * Miejsce dodawania bibliotek.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */

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

//Porty używane w programie.
const int PORT = 35787;
const int FIND_PORT = 35788;

//Liczbe maszyn od których pobieramy jednoczesnie jeden plik.
const int HOST_NUMER = 10;
//Startowa liczba fragmentów w zapytaniu DOWN.
const int START_FRAGMENT_COUNT = 20;
//Czas oczekiwania na najbliższy fragment w procesie pobierania.
const int TIMEOUT_SEC = 10;
//Czas oczekiwania na rozesłanie zapytania FIND w procesie pobierania z którym nie są skojarzone żadne maszyny.
const int TIMEOUT_SEC_FIND_AGAIN = 2;
//Współczynnik zwiększania liczności fragmentów w zapydaniu DOWN.
const double GROW_PORTION = 1.2;
//Liczba najlepszych maszyn, którym zwiększamy liczbe fragmentów w najbliższych zapytaniach DOWN.
const int TOP_HOSTS = 3;
//Liczba kolejkowanych połączeń na accept.
const int TCP_ONE_TIME_CONNECTION = 5;
//Czas oczekiwania na odpowiedzi w polecniu FIND
const int TIMEOUT_FIND = 2;

//Możliwe typy wiadomości.
enum MsgType {UP_MSG, HAS_MSG, DEL_MSG, FORCE_DEL_MSG, FIND_MSG, DOWN_MSG, CANT_MSG, OWN_MSG, DONT_MSG};


#endif
