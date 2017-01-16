/*
 * Klasa pomocnicza reprezentująca maszynę od której pobieramy dane.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */

#ifndef HOST_H
#define HOST_H

#include "structs.h"

class Host
{
	public:
		int sock, asked_fragment, get_fragment, portion;
		time_t time_of_start;
		struct sockaddr_in addr;	
		socklen_t addr_length;	

		Host(){}

		~Host(){}

		//Konstruktor przygotowujący podstawowe informacje o maszynie i połączeniu z nią - w tym jej adres.
		Host(struct sockaddr_in* host_addr, socklen_t* host_addr_length)
		{
			addr = *host_addr;
			addr.sin_port = htons(PORT);
			addr_length = *host_addr_length;
			portion = START_FRAGMENT_COUNT;
			asked_fragment = 0;
			get_fragment = 0;
		}

		//Metoda zestawiająca połączenie.
		bool connect_host_socket()
		{
			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sock == -1) {
				perror("Opening stream socket");
				return false;
			}

			if (connect(sock, (sockaddr*)(&addr), addr_length) < 0)
			{
				perror("Connect error");
				return false;
			}

			time ( &time_of_start );
			asked_fragment = portion;
			get_fragment = 0;
			return true;
		}

		//Metoda inkrementująca liczbe pobranych pakietów w ramacj połączenia oraz zwracająca czy odebraliśmy wszystki fragmenty o które prosiliśmy.
		bool is_all_fragments()
		{
			++get_fragment;
			return asked_fragment <= get_fragment;
		}

		//Metoda zwracająca prędkość pobierania z danej maszyny.
		double get_speed()
		{
			return get_fragment / time_of_start;
		}

		//Metoda zwiększająca porcje fragmentów o które będziemy prosić w zapytaniu DOWN.
		void grow_portion()
		{
			portion = (int) ((double)portion * GROW_PORTION);
		}
};

#endif
