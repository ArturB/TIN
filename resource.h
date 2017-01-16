/*
 * Struktura reprezentująca zasób.
 * Miejsce dodawania bibliotek.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */

#ifndef RESOURCE_H
#define RESOURCE_H
#include "host.hpp"


struct Resource {
	//Identyfikator
	FileID* id;
	//Fizyczna scieżka
	string filePathName;
	//Osoby, o których wiemy, że mają zasób
	deque<Host> peers;
	//Brakujące fragmenty
	vector<long> missingBlocks;
	//Czy zasób jets obecnie pobierany
	bool is_downloading;
};

#endif
