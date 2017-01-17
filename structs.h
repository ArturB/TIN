<<<<<<< Updated upstream
/*
 * Plik z strukturami wykorzystywanymi w programie.
 * Miejsce dodawania bibliotek.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */
=======
/**
  * \file structs.h
  * Różne struktury wykorzystywane w programie. 
  */
>>>>>>> Stashed changes

#ifndef STRUCTS_H
#define STRUCTS_H
#include "params.h"

<<<<<<< Updated upstream
//Struktura identyfikująca zasób.
=======
///Struktura identyfikująca zasób sieciowy (nie lokalny). 
>>>>>>> Stashed changes
typedef struct FileIDStruct {
	const char* name;
	const char* owner;
	uint64_t size;
	time_t time;
} FileID;

<<<<<<< Updated upstream
//Struktura będąca kontenerem zasobów.
=======
///Struktura będąca kontenerem zasobów sieciowych. 
>>>>>>> Stashed changes
typedef struct FileIDsStruct {
	FileID* ids;
	unsigned size;
} FileIDs;

<<<<<<< Updated upstream
//Struktura reprezentująca nagłówek wiadomości.
=======
///Struktura reprezentująca nagłówek komunikatu protokołu AAP.
>>>>>>> Stashed changes
typedef struct ResourceHeader{
	char name[256];
	///Rozmiar danych w postaci zmiennej C++ lub binarnej
	union size_union
	{
		char byteNum[8];
		uint64_t longNum;
	} size;
	///Czas dodania zasobu w postaci zmiennej C++ lub binarnej. 
	union time_union
	{
		char byteTime[8];
		time_t ttime;
	} time;
	char owner[6];	
	MsgType type;
} ResourceHeader;

<<<<<<< Updated upstream
//Struktura reprzentująca pojedyńczy przedział numerów fragmentów z zapytania DOWN.
=======
///truktura reprzentująca pojedynczy przedział numerów bloków z zapytania DOWN.
>>>>>>> Stashed changes
typedef struct Interval
{
	long start;
	long end;
} Interval;

<<<<<<< Updated upstream
//Struktura reprzentująca wiadomość DOWN.
=======
///Struktura reprzentująca wiadomość DOWN.
>>>>>>> Stashed changes
typedef struct DownMsg{
	ResourceHeader header;
	Interval fragments[64];
} DownMsg;

<<<<<<< Updated upstream
//Struktura przechowująca dane pojedyńczego fragmentu zasobu.
=======
///Struktura przechowująca dane pojedynczego bloku pliku.
>>>>>>> Stashed changes
typedef struct FileFragment{
	uint8_t byteArray[1024];
	long number;
} FileFragment;

<<<<<<< Updated upstream
//Stałe
=======
///Maksymalny możliwy rozmiar wiadomości
>>>>>>> Stashed changes
const int MAX_MSG_SIZE = sizeof(struct DownMsg);
///Minimalny możlwy rozmiar wiadomości. 
const int MIN_MSG_SIZE = sizeof(struct ResourceHeader);

#endif
