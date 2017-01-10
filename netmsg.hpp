#ifndef NETMSG_H
#define NETMSG_H


#include "structs.h"

class NetMsg {
	public:
		ResourceHeader header;
		vector<long> blocks;
		struct sockaddr_in* sender;
		socklen_t* addres_length;

		NetMsg()
		{

		}

		~NetMsg()
		{
		}

		NetMsg(ResourceHeader* h, struct sockaddr_in* sender_addr, socklen_t* addr_length)
		{
			memcpy(&header, h, sizeof(header));	
			sender = sender_addr;
			addres_length = addr_length;
		}


		NetMsg(DownMsg* d, struct sockaddr_in* sender_addr, socklen_t* addr_length)
		{
			memcpy(&header, &(d->header), sizeof(header));
			sender = sender_addr;
			addres_length = addr_length;

			for(int i = 0; i < 64 && d->fragments[i].start != 0; ++i)
			{
				for(int j = d->fragments[i].start; j <= d->fragments[i].end; ++j)	
				{
					blocks.push_back(j);
				}
			}
		}
		

		void setFileId(FileID* fid)
		{
			if(fid->name != NULL)
				memcpy(header.name,fid->name,256);
			if(fid->owner != NULL)
				memcpy(header.owner,fid->owner,6);
			header.size.longNum = fid->size;
			header.time.ttime = fid->time;
		}

		void setReceiver(Host h)
		{
			sender = &(h.addr);
			addres_length = &(h.addr_length);
		}
};

#endif
