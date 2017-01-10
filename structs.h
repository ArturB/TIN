#ifndef STRUCTS_H
#define STRUCTS_H
#include "params.h"

typedef struct FileIDStruct {
	const char* name;
	const char* owner;
	uint64_t size;
	time_t time;
} FileID;


typedef struct FileIDsStruct {
	FileID* ids;
	unsigned size;
} FileIDs;


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

typedef struct Interval
{
	long start;
	long end;
} Interval;

typedef struct DownMsg{
	ResourceHeader header;
	Interval fragments[64];
} DownMsg;

typedef struct FileFragment{
	uint8_t byteArray[1024];
	long number;
} FileFragment;

const int MAX_MSG_SIZE = sizeof(struct DownMsg);
const int MIN_MSG_SIZE = sizeof(struct ResourceHeader);

#endif
