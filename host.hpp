<<<<<<< Updated upstream
/*
 * Klasa pomocnicza reprezentująca maszynę od której pobieramy dane.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */
=======
/**
  * \file host.hpp
  * Klasa reprezentująca hosta od którego można pobierać dane. 
  */
>>>>>>> Stashed changes

#ifndef HOST_H
#define HOST_H

#include "structs.h"

///Klasa reprezentująca hosta od którego można pobierać dane. 
class Host
{
	public:
		int sock, asked_fragment, get_fragment, portion;
		time_t time_of_start;
		struct sockaddr_in addr;	
		socklen_t addr_length;	

		Host(){}

		~Host(){}

<<<<<<< Updated upstream
		//Konstruktor przygotowujący podstawowe informacje o maszynie i połączeniu z nią - w tym jej adres.
=======
		///Konstruktor przygotowujący podstawowe informacje o maszynie i połączeniu z nią - w tym jej adres.
>>>>>>> Stashed changes
		Host(struct sockaddr_in* host_addr, socklen_t* host_addr_length)
		{
			addr = *host_addr;
			addr.sin_port = htons(PORT);
			addr_length = *host_addr_length;
			portion = START_FRAGMENT_COUNT;
			asked_fragment = 0;
			get_fragment = 0;
		}

<<<<<<< Updated upstream
		//Metoda zestawiająca połączenie.
=======
		///Metoda zestawiająca połączenie.
>>>>>>> Stashed changes
		bool connect_host_socket()
		{
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sock == -1) {
				perror("Opening stream socket");
				return false;
			}

			if (connect(sock, (sockaddr*)(&addr), addr_length) < 0)
			{
				return false;
			}

			time ( &time_of_start );
			asked_fragment = portion;
			get_fragment = 0;
			return true;
		}

<<<<<<< Updated upstream
		//Metoda inkrementująca liczbe pobranych pakietów w ramacj połączenia oraz zwracająca czy odebraliśmy wszystki fragmenty o które prosiliśmy.
=======
		///Metoda inkrementująca liczbe pobranych pakietów w ramacj połączenia oraz zwracająca czy odebraliśmy wszystkie fragmenty o które prosiliśmy.
>>>>>>> Stashed changes
		bool is_all_fragments()
		{
			++get_fragment;
			return asked_fragment <= get_fragment;
		}

<<<<<<< Updated upstream
		//Metoda zwracająca prędkość pobierania z danej maszyny.
=======
		///Metoda zwracająca prędkość pobierania z danej maszyny.
>>>>>>> Stashed changes
		double get_speed()
		{
			return get_fragment / time_of_start;
		}

<<<<<<< Updated upstream
		//Metoda zwiększająca porcje fragmentów o które będziemy prosić w zapytaniu DOWN.
=======
		///Metoda zwiększająca porcje fragmentów o które będziemy prosić w zapytaniu DOWN.
>>>>>>> Stashed changes
		void grow_portion()
		{
			portion = (int) ((double)portion * GROW_PORTION);
		}
};

#endif
