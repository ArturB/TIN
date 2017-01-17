<<<<<<< Updated upstream
/*
 * Klasa pomocnicza zbierająca podstawowe informacje nt. procesu pobierania pliku
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */
=======
/**
  * \file filedownload.hpp
  * Klasa reprezentująca aktualnie pobierany w systemie plik. 
  */
>>>>>>> Stashed changes

#ifndef FILEDOWNLOAD_H
#define FILEDOWNLOAD_H

#include "resource.h"

///Klasa reprezentująca aktualnie pobierany w systemie plik. 
class FileDownload
{
	public:
		list<Resource>::iterator* file;
		deque<Host> avaiable_hosts;
		deque<Host> using_hosts;
		int fragments;

<<<<<<< Updated upstream
		//Konstruktor - liczy liczbe fragmentów pliku.
=======
		///Konstruktor
		/**
		  * \param param iterator po fragmentach pliku do pobrania
		  */
>>>>>>> Stashed changes
		FileDownload(list<Resource>::iterator* param)
		{
			file = param;
			fragments = (*file)->id->size/1024;
			if((*file)->id->size%1024 > 0)
				++fragments;
		}

<<<<<<< Updated upstream
		//Metoda podająca które miejsce na liście elementów od których pobieramy zajmuje elemenet o zadanej prędkości.
=======
		///Zwraca ranking hosta wg prędkości pobierania. Plik pobieramy od 5 najszybszych hostów. 
>>>>>>> Stashed changes
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
