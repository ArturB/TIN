/*
 * Plik z strukturami wykorzystywanymi w programie.
 * Miejsce dodawania bibliotek.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */

#ifndef STRUCTS_H
#define STRUCTS_H
#include "params.h"

//Struktura identyfikująca zasób.
typedef struct FileIDStruct {
	const char* name;
	const char* owner;
	uint64_t size;
	time_t time;
} FileID;

//Struktura będąca kontenerem zasobów.
typedef struct FileIDsStruct {
	FileID* ids;
	unsigned size;
} FileIDs;

//Struktura reprezentująca nagłówek wiadomości.
typedef struct ResourceHeader{
	char name[256];
	union size_union
	{
		char byteNum[8];
		uint64_t longNum;
	} size;
	union time_union
	{
		char byteTime[8];
		time_t ttime;
	} time;
	char owner[6];	
	MsgType type;
} ResourceHeader;

//Struktura reprzentująca pojedyńczy przedział numerów fragmentów z zapytania DOWN.
typedef struct Interval
{
	long start;
	long end;
} Interval;

//Struktura reprzentująca wiadomość DOWN.
typedef struct DownMsg{
	ResourceHeader header;
	Interval fragments[64];
} DownMsg;

//Struktura przechowująca dane pojedyńczego fragmentu zasobu.
typedef struct FileFragment{
	uint8_t byteArray[1024];
	long number;
} FileFragment;

//Stałe
const int MAX_MSG_SIZE = sizeof(struct DownMsg);
const int MIN_MSG_SIZE = sizeof(struct ResourceHeader);

#endif
