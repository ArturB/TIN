<<<<<<< Updated upstream
/*
 * Klasa związana z komunikatem.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */
=======
/**
  * \file netmsg.hpp
  * Klasa reprezentująca komunikat protokołu AAP. 
  */
>>>>>>> Stashed changes

#ifndef NETMSG_H
#define NETMSG_H

#include "structs.h"

///Klasa reprezentująca komunikat protokołu AAP. 
class NetMsg {
	public:
		ResourceHeader header;
		vector<long> blocks;
		struct sockaddr_in* sender;
		socklen_t* addres_length;
		
		///Konstruktor domyślny
		NetMsg(){}
		
		///Destruktor
		~NetMsg(){}

<<<<<<< Updated upstream
		NetMsg(){}

		~NetMsg(){}

		//Konstruktor przygotwujący wiadomość na podstawie nagłówka oraz adresu odbiorcy.
=======
		///Konstruktor przygotwujący wiadomość na podstawie nagłówka oraz adresu odbiorcy.
>>>>>>> Stashed changes
		NetMsg(ResourceHeader* h, struct sockaddr_in* sender_addr, socklen_t* addr_length)
		{
			memcpy(&header, h, sizeof(header));	
			sender = sender_addr;
			addres_length = addr_length;
		}

<<<<<<< Updated upstream
		//Konstruktor obiektu reprezentującego odebrana wiadomość DOWN.
=======
		///Konstruktor obiektu reprezentującego odebrana wiadomość DOWN.
>>>>>>> Stashed changes
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
		
<<<<<<< Updated upstream
		//Metoda tworząca nagłówek na podstawie FileID.
=======
		///Metoda tworząca nagłówek na podstawie FileID.
>>>>>>> Stashed changes
		void setFileId(FileID* fid)
		{
			if(fid->name != NULL)
				memcpy(header.name,fid->name,256);
			if(fid->owner != NULL)
				memcpy(header.owner,fid->owner,6);
			header.size.longNum = fid->size;
			header.time.ttime = fid->time;
		}

<<<<<<< Updated upstream
		//Metoda ustawiająca adresata na podstawie obiektu Host.
=======
		///Metoda ustawiająca adresata na podstawie obiektu Host.
>>>>>>> Stashed changes
		void setReceiver(Host h)
		{
			sender = &(h.addr);
			addres_length = &(h.addr_length);
		}
};

#endif
