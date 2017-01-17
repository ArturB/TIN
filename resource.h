<<<<<<< Updated upstream
/*
 * Struktura reprezentująca zasób.
 * Miejsce dodawania bibliotek.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */
=======
/**
  * \file 
  * Struktura reprezentująca zasób posiadany przez klienta lokalnie.
  */
>>>>>>> Stashed changes

#ifndef RESOURCE_H
#define RESOURCE_H
#include "host.hpp"

<<<<<<< Updated upstream

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
=======
///Struktura reprezentująca zasób posiadany przez klienta lokalnie.
struct Resource {
	///Identyfikator
	FileID* id;
	///Fizyczna scieżka
	string filePathName;
	///Osoby, o których wiemy, że mają zasób
	deque<Host> peers;
	///Brakujące fragmenty
	vector<long> missingBlocks;
	///Czy zasób jest obecnie pobierany
>>>>>>> Stashed changes
	bool is_downloading;
};

#endif
