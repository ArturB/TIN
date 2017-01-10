#ifndef RESOURCE_H
#define RESOURCE_H
#include "host.hpp"

struct Resource {
	FileID* id;
	string filePathName;
	deque<Host> peers;
	vector<long> missingBlocks;
	bool is_downloading;
};

#endif
