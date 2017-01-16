/*
 * Klasa pomocnicza zbierająca podstawowe informacje nt. procesu pobierania pliku
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */

#ifndef FILEDOWNLOAD_H
#define FILEDOWNLOAD_H

#include "resource.h"

class FileDownload
{
	public:
		vector<Resource>::iterator* file;
		deque<Host> avaiable_hosts;
		deque<Host> using_hosts;
		int fragments;

		//Konstruktor - liczy liczbe fragmentów pliku.
		FileDownload(vector<Resource>::iterator* param)
		{
			file = param;
			fragments = (*file)->id->size/1024;
			if((*file)->id->size%1024 > 0)
				++fragments;
		}

		//Metoda podająca które miejsce na liście elementów od których pobieramy zajmuje elemenet o zadanej prędkości.
		int speed_in_hosts(double speed)
		{
			int place = 1;
			for(int i = 0; i < using_hosts.size(); ++i)
				if(using_hosts[i].get_speed() > speed)
					++place;
			return place;
		}
};


#endif
