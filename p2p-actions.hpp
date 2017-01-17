/*
 * Plik zawierający funkcje realizujące sieciową logike aplikacji.
 * Miejsce dodawania bibliotek.
 * Anna Skupińska
 * Artur M. Brodzki
 * Adam Małkowski
 * Piotr Włodkowski
 */
#ifndef P2P_ACTIONS_HPP
#define P2P_ACTIONS_HPP

#include "data.hpp"


using namespace std;

/*************************
		DEKLARACJE
 *************************/


struct CommandData {
	string filePath;
	string fileName;
	uint64_t fileSize;
	FileIDs ids;
	bool find_one;
	bool find_all;
	bool find_first;
	unsigned int find_first_count;
};

struct CommandData cmdata;



void  upload_action();
void* upload_thread(void*);
void  delete_action();
void* delete_thread(void*);
void  find_action();
void* find_thread(void*);
void* find_msg_sender_thread(void *par);
void* find_msg_sender_thread(void *par);
void* download_thread(void* par);
void* download_file(void* par);
void  download_action();
void check_files_validations();
void* receiver_thread_UDP(void*);
void* receiver_thread_TCP(void*);
void* up_msg_reaction(void*);
void* has_msg_reaction(void*);
void* del_msg_reaction(void* par);
void* force_del_msg_reaction(void* par);
void* find_msg_reaction(void* par);
void* own_msg_reaction(void* par);
void* response_down_tcp(void* par);
void send_broadcast(ResourceHeader *header);
void send_response(NetMsg *netMsg, int port = PORT);
bool recive_fragment(deque<Host>::iterator it, FileDownload file_downloading);
void close_hosts_sockets(deque<Host> *collection);
void write_progress_of_download(list<Resource>::iterator* file);
int get_next_fragments(Interval* result, vector<long>* input, int* intervals_count, int portion, int from);
struct FileIDStruct fileid_from_header(ResourceHeader* header);
ResourceHeader header_from_command_data(CommandData* data);
void run_downloadings();


//Mutex chroniący dostęp do stdout
pthread_mutex_t termtx;
//Mutex chroniący dostęp do cmdata
pthread_mutex_t cmtx;

//Komunikaty
const char* welcome = 
	"Welcome in Pompous AAP File Transfer Protocol v1.0!\nRemember: your computer is your friend.\nPlease type a command now.\n";
const char* prompt = ">> ";
const char* errMsg = "command error"; 
const char* threadFailed = "Creating command thread failed!\n>> ";
const char* receiverFailed = "Creating receiver thread failed!\n>> ";
const char* tcpThreadFailed = "Creating data-changing thread failed!\n>> ";
const char* downloadThreadFailed = "Creating download thread failed!\n>> ";

//zmienne

uint64_t id;




/***************************
	    PROCEDURY
 ***************************/

//Metoda wypisująca identyfikator.
void printFileID(FileID f) {
	if(f.name)
		cout<<"Name: "<<f.name<<", ";
	if(f.size)
		cout<<"Size: "<<f.size;
	cout<<". "<<endl;
}

//Bezpieczne wypisywanie na stdout z relizacją wzajemnego wykluczania.
void safe_cout(string str) {
	pthread_mutex_lock(&termtx);
	cout<<str;
	fflush(stdout);
	pthread_mutex_unlock(&termtx);
}
/*
	Procedura wołana bezpośrednio po uruchomieniu programu.
	Inicjuje mutexy, pobiera ID obecnego klienta, deserializuje metadane
	tworzy wątki nasłuchując, sprawdza poprawność danych 
	oraz uruchamia przerwane procesy pobierania.
*/
void main_init() {


	pthread_mutex_init(&termtx, NULL);
	pthread_mutex_init(&cmtx, NULL);
	initiateMutexes();
	id = get_id();
	//Tu jest deserializacja
	std::cout << "Loading metadata:" << deserializeMetaData() << std::endl;

	pthread_t receiverThreadUDP;
	if(pthread_create(&receiverThreadUDP, NULL, receiver_thread_UDP, NULL)) 
	{
		safe_cout(receiverFailed);
	}
	
	pthread_t receiverThreadTCP;
	if(pthread_create(&receiverThreadTCP, NULL, receiver_thread_TCP, NULL)) 
	{
		safe_cout(receiverFailed);
	}
	
	run_downloadings();
	check_files_validations();
}

//Destrukcja muteksów i innych aktywów
void main_destroy() {
	pthread_mutex_destroy(&termtx);
	pthread_mutex_destroy(&cmtx);
	destroyMutexes();
	std::cout << "Save metadata:" << serializeMetaData() << std::endl;
}


void upload_action() {
	pthread_t uploadThread;
	if(pthread_create(&uploadThread, NULL, upload_thread, (void*)&cmdata))
	{ 
		safe_cout(threadFailed);
	}
}

/*
	Cialo watku obslugujacego operacje dodania zasobu.
	W przypadku udanego dodania rozsyla o tym fakcie informace wraz z danymi zasobu.
*/
void* upload_thread(void* par) {
	struct CommandData parT = *((struct CommandData*)par);
	safe_cout("Uploading file " + parT.fileName + "...\n>> ");
	list<Resource>::iterator it;
	it = addFile(parT.filePath, parT.fileName);
	if(!isValidResource(it))
	{
		safe_cout("Uploading unsuccessfull! Invalid path\n>> ");
	}
	else
	{
		if(strcmp(it->id->name,parT.fileName.c_str()))
		{
			safe_cout("Uploading unsuccessfull! In storage exists resource, which shares that file.\n>> ");
		}
		else if(it->filePathName != parT.filePath)
		{
			safe_cout("Uploading unsuccessfull! In storage exists resource, which has the same name and size \n>> ");
		}
		else
		{
			safe_cout("File " + parT.fileName + " uploaded!\n>> ");
			NetMsg netMsg;
			netMsg.setFileId(it->id);
			netMsg.header.type = UP_MSG;
			send_broadcast(&(netMsg.header));
		}
	}
}

void delete_action() {
	pthread_t deleteThread;
	if(pthread_create(&deleteThread, NULL, delete_thread, (void*)&cmdata))
	{
		safe_cout(threadFailed);
	}
}

/*
	Cialo watku obslugujacego operacje usuniecia zasobu.
	W przypadku udanego usuniecia zasobu, ktorego bylismy wlascicielami,
	rozsyla o tym fakcie informace wraz z danymi usunietego zasobu.
*/
void* delete_thread(void* par) {
	struct CommandData parT = *((struct CommandData*)par);
	safe_cout("Deleting file " + parT.fileName + "...\n>> ");
	FileID fid;

	fid = deleteFile(parT.fileName);
	if(fid.name == NULL)
	{
		safe_cout("Deleting unsuccessfull!\n>> ");
	}
	else
	{
		safe_cout("Deleted file " + string(fid.name) + "...\n>> ");
		if(bytesToLong(fid.owner) == id)
		{
			NetMsg netMsg;
			netMsg.setFileId(&fid);
			netMsg.header.type = DEL_MSG;
			send_broadcast(&(netMsg.header));
		}
		free((void*)fid.name);
		free((void*)fid.owner);
	}
}
/*
	Wątek nasłuchujący na próby połączenia TCP w celu pobrania od nas danych.
	W przypadku nawiązania uruchamia wątek obsługujący dane połączenie.
*/
void* receiver_thread_TCP(void* par) 
{
	int sock, length;
	struct sockaddr_in server;
	struct sockaddr_in client;
	length = sizeof(client);
	int msgsock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		perror("DOWN loop: Opening stream socket\n>> ");
		exit(1);
	}

	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *) &optval , sizeof(optval));

	server.sin_family= AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	if (bind(sock, (struct sockaddr *) &server, sizeof server) == -1)
	{
		perror("DOWN loop: Binding stream socket\n>> ");
		exit(1);
	}

	if(listen(sock, TCP_ONE_TIME_CONNECTION)!= 0)
		perror ("DOWN loop: Listen call error\n>> ");
	do {
		msgsock = accept(sock,(struct sockaddr *) 0,(socklen_t*) 0);
		if (msgsock == -1 )
			perror("DOWN loop: Accept call error\n>> ");
		else 
		{
			pthread_t responseTCP;
			if(pthread_create(&responseTCP, NULL, response_down_tcp, &msgsock)) 
			{
				safe_cout(tcpThreadFailed);
			}
		}
	} while(true);

	exit(0);
}


/*
	Ciało wątku obsługującego zapytanie DOWN.
	Po odebraniu wiadomości sprawdza o jaki fragmenty jesteśmy pytani i 
	odsyła je kolejno.
	W przypadku wystąpienia problemów i zakończenia transmisji wysyła
	fragment z numerem 0 co oznacza zakończenie transmisji.

*/
void* response_down_tcp(void* par)
{
	int msgsock = *((int*)par);
	int rec_bytes;
	DownMsg down_msg;
	int number_of_block;	

	socklen_t length;
	struct sockaddr_in clientaddr;
	length = sizeof(clientaddr);
cout<<"przed rec_bytes"<<endl;
	rec_bytes = recvfrom(msgsock, &down_msg, sizeof(down_msg), 0, (struct sockaddr *) &clientaddr,  &length);
	if (rec_bytes < sizeof(down_msg))
	{
		close(msgsock);
		return NULL;
	}

	NetMsg net_msg(&down_msg, &clientaddr, &length);

	
	FileID* file = isFileInStorage(&net_msg.header);

	
	bool send_cant;
	if(file == NULL)
	{	
		send_cant = true;
	}
	else
		send_cant = false;


	if(!send_cant)
	{
		for(int i = 0; i < net_msg.blocks.size(); ++i)
		{
			if(isFileInStorage(&net_msg.header) == NULL)
			{
				close(msgsock);
				return NULL;	
			}
			FileID id = fileid_from_header(&net_msg.header);
			FileFragment* file_fragment = getFileFragment(&id, net_msg.blocks[i]); 
			delete id.name;
			delete id.owner;
			if(file_fragment == NULL)
			{
				send_cant = true;
				break;
			}
			else
			{
				if (sendto(msgsock, file_fragment, sizeof(FileFragment), MSG_NOSIGNAL, 
					(sockaddr *)&clientaddr, sizeof(&clientaddr)) < 0)
				{
					delete file_fragment;
					close(msgsock);cout<<"cokolwiek 1\n";
					return NULL;		
				}
				delete file_fragment;
			}
		}
	}
	if(send_cant)
	{
		FileFragment file;
		file.number = 0;
		if (sendto(msgsock, &file, sizeof(file), 0, 
			(sockaddr *)&clientaddr, sizeof(&clientaddr)) < 0)
		{
			close(msgsock);
			return NULL;
		}
	}
	close(msgsock);
}

/*
	Funkcja obliczająca o jakie fragmenty pobieranego pliku spytać.
	result		- parametr wynikowy, lista przedziałów o które mamy spytać.
	intervals_count - parametr wynikowy, liczba przedziałów.
	input		- lista brakujących bloków.
	portion		- liczba fragmentów o które zapytamy.
	from		- ostatni fragment o który zapytaliśmy.
	Zwraca numer ostatniego fragmentu o który zapytamy.
*/
int get_next_fragments(Interval* result, vector<long>* input, int* intervals_count, int portion, int from)
{
	bzero(result, 64 * sizeof(Interval));

	int i, j , choosen_fragments;
	choosen_fragments = 0;
	for(i = 0; i < input->size(); ++i)
	{
		if((*input)[i] > from)
			break;
	}
	if(i == input->size())
		i = 0;

	for(j = 0; j < 64 && choosen_fragments < portion && choosen_fragments < input->size(); ++j)
	{
		Interval tmp;
		tmp.start = (*input)[i];
		tmp.end = (*input)[i];
		if(i < input->size())
			++i;
		else
			i = 0;
		++choosen_fragments;
		for(int k = 0; k < portion && choosen_fragments < portion && i < input->size(); ++k)
		{
			if(tmp.start + k == (*input)[i] - 1)
			{
				tmp.end = (*input)[i];
				++i;

				++choosen_fragments;
			}
			else
				break;
		}
		result[j] = tmp;
		if(i == input->size())
			i = 0;
	}



	*intervals_count = j;
	if(i == 0)
		return (*input)[input->size()-1];
	else
		return (*input)[i-1];
}

//Funkcja wysyłająca wiadomość DOWN.
bool send_down_message(DownMsg* down_msg, int socket_tcp)
{
	if (send(socket_tcp, down_msg, sizeof(DownMsg), 0) < 0)
	{
		close(socket_tcp);
		return false;
	}
	return true;
}

/*
	Funkcja zajmująca się odbiorem pojedyńczego fragmentu zasobu.
	W przypadku wystąpienia problemów z odbiorem, usuwa nadawce
	z listy maszyn od których pobieramy.
	W przypadku gdy był to ostatni pakiet na który czekaliśmy
	od danego nadawcy
		a) jeżeli był najwolniejszym dostawcą, usuwa go
			z listy maszyn od których pobieramy
		b) w. p. p. prosi go o następne fragmenty
		
	Zwraca true gdy udało się odebrać fragment.
*/

bool recive_fragment(deque<Host>::iterator *host, FileDownload *file_downloading, int* fragment_counter)
{	

	struct sockaddr_in *clientaddr;
	int length;
	FileFragment file_fragment;
	int rec_bytes, tmp_rec_bytes;
	rec_bytes = 0;

        
	while(rec_bytes != sizeof(FileFragment))
	{
		tmp_rec_bytes = recvfrom((*host)->sock, &file_fragment + rec_bytes, sizeof(FileFragment) - rec_bytes, 0, (struct sockaddr *) clientaddr,  (socklen_t*) &length);

		if (tmp_rec_bytes <= 8)
		{
			close((*host)->sock);	
			(*host) = file_downloading->using_hosts.erase((*host));
			return false;
		}
		rec_bytes += tmp_rec_bytes;
	}
    
	if(file_fragment.number == 0)
	{
		(*host) = file_downloading->using_hosts.erase((*host));
		return false;
	}


	if(!saveFileFragment((*file_downloading->file)->id, &file_fragment))
	{
		return false;
	}


	list<Resource>::iterator* res = file_downloading->file;
	if((*host)->is_all_fragments() && (*res)->missingBlocks.size()!= 0) 
	{
		if(file_downloading->speed_in_hosts((*host)->get_speed()) == HOST_NUMER && HOST_NUMER != 1)
		{
			close((*host)->sock);
			file_downloading->avaiable_hosts.push_back(*(*host));
			(*host) = file_downloading->using_hosts.erase((*host));
		}
		else
		{
			if(file_downloading->speed_in_hosts((*host)->get_speed() < TOP_HOSTS ))
			{
				(*host)->grow_portion();
			}

			DownMsg msg;

			bzero(msg.header.name, sizeof(msg.header.name));
			memcpy(msg.header.name, (*file_downloading->file)->id->name, sizeof(msg.header.name));	

			msg.header.size.longNum = (*file_downloading->file)->id->size;	
			msg.header.time.ttime = (*file_downloading->file)->id->time;	
			memcpy(msg.header.owner, (*file_downloading->file)->id->owner, sizeof(msg.header.owner));
		
			msg.header.type = DOWN_MSG;	
			int internval_count;
			*fragment_counter = get_next_fragments(msg.fragments, &(*file_downloading->file)->missingBlocks, &internval_count, (*host)->portion, *fragment_counter);
			close((*host)->sock);
			if((*host)->connect_host_socket())
			{
				if(!send_down_message(&msg, (*host)->sock))
					(*host) = file_downloading->using_hosts.erase((*host));
			}
			else
			{
				(*host) = file_downloading->using_hosts.erase((*host));
			}
		}
	}
	return true;
}
/*
	Wyświetla komunikat o stanie pobierania.
	break_downloading - gdy true, wypisuje informacje o przerwaniu pobierania pliku.
*/
void write_progress_of_download(list<Resource>::iterator* file, bool break_downloading)	
{
	int fragments = (*file)->id->size/1024 ;
	if(fragments == 0 && (*file)->id->size != 0)
		fragments ++;

	string text;


	double progress = ((double)(fragments - (*file)->missingBlocks.size())/((double)fragments));
	progress *= 100;

	ostringstream tmp;
	if(break_downloading)
		tmp << "Downloading file: " << string((*file)->id->name) << " is break\n>> ";
	else
		tmp << "File: " << string((*file)->id->name) << " is downloading. Progress: " << progress << "%\n>> ";
	text = tmp.str();
	safe_cout(text);
}

void download_action() {
	pthread_t downloadThread;
	if(pthread_create(&downloadThread, NULL, download_thread, (void*)&cmdata)) 
	{ 
		safe_cout(downloadThreadFailed);
	}
}

//Funkcja pomocnicza tworząca strukturę ResourceHeader z CommandData.
ResourceHeader header_from_command_data(CommandData* data)
{
	ResourceHeader result;

	bzero(result.name, sizeof(result.name));
	int i;

	const char *cstr = data->fileName.c_str();

	memcpy((void*) result.name, cstr, data->fileName.size());
	result.size.longNum = data->fileSize;

	result.time.ttime = 0;
	bzero(result.owner, sizeof(result.owner));
	return result;
}

//Funkcja pomocnicza tworząca strukturę ResourceHeader z FileID.
ResourceHeader header_from_id(FileID* file, MsgType type)
{
	ResourceHeader header;
	bzero(header.name, sizeof(header.name));
	header.type = type;
	int name_length;
	for(name_length = 0; file->name[name_length] != 0 && name_length < 256; ++name_length);
	memcpy((void*) header.name, file->name, name_length);

	header.size.longNum = file->size;
	header.time.ttime = file->time;
	memcpy((void*) header.owner, file->owner, 6);
	return header;
}

//Funkcja pomocnicza tworząca strukturę FileID z ResourceHeader.
struct FileIDStruct fileid_from_header(ResourceHeader* header)
{
	struct FileIDStruct id;
	
	int i;

	char* buf_name = strdup(header->name);
	char* buf_owner = new char[6];

	memcpy(buf_owner, header->owner, 6);
	id.name = buf_name;
	id.owner = buf_owner;
	id.size = header->size.longNum;
	id.time = header->time.ttime;
	return id;
}

/*
	Ciało wątku pobierającego.
	Próbuje znaleść zasób w sieci, jeżeli mu to się uda rozpoczyna pobieranie zasobu.
*/
void* download_thread(void* par) 
{
	safe_cout("Dowloading...\n>> ");
	
	ResourceHeader header = header_from_command_data((struct CommandData*)par);

	list<Resource>::iterator file = getResource(&header);
	bool is_valid = isValidResource(file);
	if(!is_valid)
	{
		CommandData command_data;
		command_data.ids.size = 1;
		command_data.ids.ids = new FileID;
		command_data.ids.ids[0] = fileid_from_header(&header);
		find_thread((void*)&command_data);
	}
	else if(fileIsDownloading(file))
	{
		write_progress_of_download(&file, false);
		return NULL;
	}
	else if(file->missingBlocks.size() == 0)
	{
		safe_cout("File is already downloaded!\n>> ");
		return NULL;
	}

	file = getResource(&header);
	if(!isValidResource(file))
	{
		safe_cout("Can't download this file!\n>> ");
		return NULL;
	}

	if(file->filePathName == "")
	{
		file->filePathName = returnFilePath(configFilePath, file->id->name);
		if(file->filePathName == "")
		{
			return NULL;
		}
	}
	list<Resource>::iterator* param = new list<Resource>::iterator;
	*param = file;
	download_file((void*) param);
	return NULL;
}

/*
	Funkcja odpowiadająca za pobieranie zasobu.
	Rozsyła zapytania o zasoby do pierwszych HOST_NUMER dostępnych maszyn,
	następnie w pętli czego na odbiór fragmentu po czym
	go zapisuje.
	Po upłynięciu time-outu w czekaniu na fragment 
	czyści liste dostępnych maszyn i wyszukuj je od nowa.
	W przypadku gdy nie ma żadnych dostępnych maszyn, w pętli czeka 
	i rozsyła zapytanie FIND.
	Gdy są dostępne maszyny od których nie pobieramy, rozpoczyna pobieranie.
	
	Co około 5% pobierania wypisuje o tym informacje.

*/

void* download_file(void* par)
{
	list<Resource>::iterator file = *((list<Resource>::iterator*) par);
	delete (list<Resource>::iterator*) par;
	int info_message = 0;
	int fragment_counter = 0;
	ResourceHeader header;

	FileDownload file_downloading(&file);
	file_downloading.avaiable_hosts = file->peers; 

	setFileIsDownloading(file);

	DownMsg down_msg; 
	header = header_from_id(file->id, DOWN_MSG);
	down_msg.header = header;

	deque<Host>::iterator it = file_downloading.avaiable_hosts.begin();
	for(int i = 0; i < HOST_NUMER && i < file_downloading.avaiable_hosts.size(); ++i)
	{
		int intervals_count;
		Host* host = &(*it);
		fragment_counter = get_next_fragments(down_msg.fragments, &file->missingBlocks, &intervals_count, START_FRAGMENT_COUNT, fragment_counter);
		host->asked_fragment += START_FRAGMENT_COUNT; 
		if(!host->connect_host_socket())
		{
			--i;
			it = file_downloading.avaiable_hosts.erase(it);
		}
		else if(!send_down_message(&down_msg, host->sock))
		{
			--i;
			it = file_downloading.avaiable_hosts.erase(it);
		}
		else
		{	
			file_downloading.using_hosts.push_back(*host);
			it = file_downloading.avaiable_hosts.erase(it);
		}
	}

	fd_set descriptors;
	struct timeval timeout;
	timeout.tv_sec = TIMEOUT_SEC;
	timeout.tv_usec = 0;
	bool recive_any_fragment = false;
	while(true)
	{
		if(file_downloading.using_hosts.size() < HOST_NUMER)
		{
			CommandData command_data;
			command_data.ids.size = 1;
			command_data.ids.ids = new FileID[1];
			command_data.ids.ids[0] = fileid_from_header(&header);
			while(file_downloading.using_hosts.size() == 0 && file_downloading.avaiable_hosts.size() == 0)
			{
				find_thread((void*)&command_data);

				file_downloading.avaiable_hosts = file->peers;
				if(file_downloading.avaiable_hosts.size() == 0)
                                    sleep(TIMEOUT_SEC_FIND_AGAIN);
			}
			while(file_downloading.using_hosts.size() < HOST_NUMER && file_downloading.avaiable_hosts.size() > 0)
			{
				deque<Host>::iterator iterator = file_downloading.avaiable_hosts.begin();
                                Host* tmp = &(*iterator);


				
				if(tmp->connect_host_socket())
				{
                                    
					int intervals_count;
					fragment_counter = get_next_fragments(down_msg.fragments, &file->missingBlocks, &intervals_count, START_FRAGMENT_COUNT, fragment_counter);
					if(send_down_message(&down_msg, tmp->sock))
                                        {
                                            file_downloading.using_hosts.push_back(*iterator);
                                            file_downloading.avaiable_hosts.erase(iterator);

                                            
                                        }
                                            
				}
				else
                                    file_downloading.avaiable_hosts.erase(iterator);

			} 
		} 
		FD_ZERO(&descriptors);
		it = file_downloading.using_hosts.begin();
		while(it != file_downloading.using_hosts.end())
		{
			FD_SET(it->sock, &descriptors);
            ++it;
		}

		if(!isValidResource(getResource(&header)))
		{
			write_progress_of_download(&file, true);
			return NULL;
		} 


		if(select(65536, &descriptors, (fd_set *) NULL, (fd_set *) NULL, &timeout) < 0 )
		{
			close_hosts_sockets(&file_downloading.using_hosts);
			file_downloading.using_hosts.clear();
			file_downloading.avaiable_hosts.clear();
                        continue;
                        //return NULL;
		}

		recive_any_fragment = false;
		it = file_downloading.using_hosts.begin();
		while(it != file_downloading.using_hosts.end())
		{
			if (FD_ISSET(it->sock, &descriptors))
			{
				if(recive_fragment(&it, &file_downloading, &fragment_counter))
                {
					recive_any_fragment = true;
                }

			}
			if(it != file_downloading.using_hosts.end())
				++it;
		}

		double percent = ((double)(file_downloading.fragments - file->missingBlocks.size())/((double)file_downloading.fragments));
		percent *= 100;


		if(info_message + 1 <= (int)percent/5)
		{
			info_message = (int)percent/5;
			write_progress_of_download(&file, false);
		}

		
		if(file->missingBlocks.size() == 0)
		{
			safe_cout("Downloading ends!...\n>> ");
			close_hosts_sockets(&file_downloading.using_hosts);
			return NULL;
		}

		if(!recive_any_fragment)
		{
			close_hosts_sockets(&file_downloading.using_hosts);
			file_downloading.using_hosts.clear();
			file_downloading.avaiable_hosts.clear();
		}
	}

	return NULL;
}

//Funkcja zamykająca połączenia z maszynami z zadanego kontenera.
void close_hosts_sockets(deque<Host> *collection)
{
	deque<Host>::iterator it = collection->begin();
	while(it != collection->end())
	{
		close(it->sock);
		++it;
	}
}

/* procedura wątku nasłuchującego komunikatów
	od innych węzłów sieci
*/
void* receiver_thread_UDP(void* par) 
{
	int sock;
	socklen_t length;
	struct sockaddr_in server;
	struct sockaddr_in clientaddr;
	int msgsock;
	ResourceHeader header;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (sock == -1) 
	{
		perror("Opening stream socket");
		exit(1);
	}
	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	server.sin_family= AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) == -1)
	{
		perror("Binding stream socket");
		exit(1);
	}
	int n;
	length = sizeof(clientaddr);
	while(true)
	{
		n = recvfrom(sock, &header, MIN_MSG_SIZE, 0, (struct sockaddr *) &clientaddr,  &length);
		if (n < 0)
		{
			perror("ERROR in recvfrom");	
			break;
		}

		NetMsg *netMsg = new NetMsg(&header, &clientaddr, &length);
		switch(header.type)
		{
			pthread_t newThread;
			case UP_MSG:
				if(pthread_create(&newThread, NULL, up_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;
			case HAS_MSG:
				if(pthread_create(&newThread, NULL, has_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;
			case DEL_MSG:
				if(pthread_create(&newThread, NULL, del_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;
			case FORCE_DEL_MSG:
			case DONT_MSG:
				if(pthread_create(&newThread, NULL, force_del_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;	
			case FIND_MSG:
				if(pthread_create(&newThread, NULL, find_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;	
			case OWN_MSG:
				if(pthread_create(&newThread, NULL, own_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;	
			default:
				break;
		}

	}
	close(sock);
}

/*
	wyslanie broadcastem wskazanej struktury ResourceHeader
*/
void send_broadcast(ResourceHeader *header)
{
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (sock == -1) {
		perror("opening stream socket");
		exit(0);
	}
	int flag = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));

	struct sockaddr_in Target_addr;
	memset(&Target_addr, 0, sizeof(Target_addr));   
	Target_addr.sin_family = AF_INET;
	Target_addr.sin_port = htons(PORT);
	Target_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

	struct sockaddr_in Local_addr;
	memset(&Local_addr, 0, sizeof(Local_addr));   
	Local_addr.sin_family = AF_INET;
	Local_addr.sin_port = htons(0);
	Local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (sockaddr*)&Local_addr, sizeof(Local_addr)) < 0)
	{
		perror("Bind call error");
		exit(0);
	}
	
	if (sendto(sock, header, MIN_MSG_SIZE, 0, 
		(sockaddr *)&Target_addr, sizeof(Target_addr)) < 0)	
	{
		perror("Send call error");
		close(sock);
		return;
	}

	close(sock);
}

/*
	wyslanie wiadomosci, ktorej tresc i adres odbiorcy zapisane sa w obiekcie NetMsg 
	znajdujacym sie pod adresem, ktory podajemy w argumencie wywolania funkcji
*/
void send_response(NetMsg *data, int port)
{
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (sock == -1) {
		perror("opening stream socket");
		exit(0);
	}

	struct sockaddr_in* Target_addr; 
	Target_addr = data->sender;
	Target_addr->sin_port = htons(port);

	struct sockaddr_in Local_addr;
	memset(&Local_addr, 0, sizeof(Local_addr));   
	Local_addr.sin_family = AF_INET;
	Local_addr.sin_port = htons(0);
	Local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (sockaddr*)&Local_addr, sizeof(Local_addr)) < 0)
	{
		perror("Bind call error");
		exit(0);
	}

	if (sendto(sock, &(data->header), MIN_MSG_SIZE, 0, 
		(sockaddr *)Target_addr, *(data->addres_length)) < 0)
	{
		perror("Send call error");
		close(sock);
		return;
	}

	close(sock);
}

//procedura wołana przez komendę find
void find_action() {
	pthread_t findThread;
	if(
		pthread_create(&findThread, NULL, find_thread, (void*)&cmdata)
	) { // błąd przy tworzeniu wątku
		safe_cout(threadFailed);
	}
}

/*
	Procedura wątku obsługi komendy find. Funkcja przez okreslony czas
	zbiera informacje HAS_MSG na temat zasobow, (lub wyszukuje okreslona liczba zasobow),
	tworzy struktury, ktore umozliwia pobranie tych zasobow i przypisuje do nich liste peerow, 
	od ktorych zasob mozna sciagnac. Rozwiazuje tez konflikty wlascicieli, jesli je stwierdzi.
*/
void* find_thread(void* par) {
	struct CommandData parT = *((struct CommandData*)par);
	int sock;
	int n;

	struct sockaddr_in server;
	struct sockaddr_in clientaddr;
	socklen_t length;
	length = sizeof(clientaddr);
	ResourceHeader* header = new ResourceHeader();
	list<Resource>::iterator resourceIt;
	set<FileID*> found;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (sock == -1) 
	{
		perror("Opening stream socket");
		safe_cout("Unsuccessful find operation. Network problem\n>> ");
		return NULL;
	}

	struct timeval tv;
	tv.tv_sec = 1;  /* 1-sekundowy timeout*/
	tv.tv_usec = 0; 

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEPORT,&optval,sizeof(optval));		
	
	server.sin_family= AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(FIND_PORT);
	
	if (bind(sock, (struct sockaddr *) &server, sizeof server) == -1)
	{
		perror("Binding stream socket\n>> ");
		safe_cout("Unsuccessful find operation. Network problem\n>> ");
		return NULL;
	}

	safe_cout("Searching...\n>> ");

	pthread_t find_msg_sender;
	if(pthread_create(&find_msg_sender, NULL, find_msg_sender_thread, &parT)) { // błąd przy tworzeniu wątku
		safe_cout(threadFailed);
		return NULL;
	}
	time_t now = time(0);


	while(time(0) - now < TIMEOUT_FIND)
	{
		n = recvfrom(sock, header, MIN_MSG_SIZE, 0, (struct sockaddr *) &clientaddr,  &length);
cout << "ODEBRALEM size " << n << endl;
		if(errno == EAGAIN || errno == EWOULDBLOCK)	//timeout
		{
			errno = 0;
			continue;			
		}
		if (n != MIN_MSG_SIZE)
		{
			perror("ERROR in recvfrom");	
			return NULL;
		}
		if(header->type != HAS_MSG)	//zly komunikat
		{
			continue;
		}

		//ktos przyslal informacje o zasobie, ktorego bylismy wlascicielami (usunelismy zasob)
		if(bytesToLong(header->owner) == id && isFileInStorage(header)==NULL)
		{
			NetMsg netMsg(header,&clientaddr, &length);
			netMsg.header.type = FORCE_DEL_MSG;
			send_response(&netMsg);
			continue;
		}
		resourceIt = openResource(header);	//stworzenie lub zwrocenie struktury zasobu
		bool isUnique = true;
		//sprawdzenie, czy nie mamy juz zapisanego adresu wezla, ktory przyslal nam informacje o zasobie
		for(int i=0; i<(*resourceIt).peers.size(); i++)	
		{
			if((*resourceIt).peers[i].addr.sin_addr.s_addr == clientaddr.sin_addr.s_addr)
			{
			  	isUnique = false;
				break;
			}
		}
		if(isUnique)		//zapisanie adresu wezla do listy peerow danego zasobu
		{
			(*resourceIt).peers.push_back(Host(&clientaddr,&length));
		}
		uint64_t headerOwner = bytesToLong(header->owner);
		uint64_t resourceOwner = bytesToLong((*resourceIt).id->owner);
		found.insert((*resourceIt).id);
		if(headerOwner != resourceOwner)	//roziwazywanie konfliktow wlascicieli
		{
			NetMsg netMsg(header, &clientaddr, &length);
			if((header->time.ttime) > ((*resourceIt).id->time))
			{
				netMsg.header.type = HAS_MSG;
				netMsg.setFileId((*resourceIt).id);
				send_response(&netMsg);
			
			}
			else if((header->time.ttime) < ((*resourceIt).id->time))
			{
				changeOwner((*resourceIt).id,header->owner,header->time.ttime);
				netMsg.header.type = HAS_MSG;
				for(Host h : (*resourceIt).peers)
				{
					netMsg.setReceiver(h);
					send_response(&netMsg);					
				}
			}
			else
			{
				netMsg.header.type = FORCE_DEL_MSG;
				for(Host h : (*resourceIt).peers)
				{
					netMsg.setReceiver(h);
					send_response(&netMsg);					
				}
				deleteFile((*resourceIt).id);
			}
		}
		if(parT.find_one || (parT.find_first && found.size() == parT.find_first_count))	//warunki konca - znalezlismy wymagana ilosc zasobow
		{
			break;
		}
	}
	close(sock);
	if(found.empty())
	{
		safe_cout("No results!\n>> ");
	}
	else									//wypisanie wynikow wyszukiwania
	{
		safe_cout("Results:\n>> ");
		for(FileID *fid : found) {
			printFileID(*fid);
		}
		safe_cout("\n>> ");

	}
}

/*
	Cialo watku, uruchamianego, by rozeslac komunikaty FIND_MSG
	z informacjami o zasobach, ktorych szukamy.
*/
void* find_msg_sender_thread(void *par)
{
	struct CommandData parT;
	parT.ids = ((struct CommandData*)par)->ids;
	cout<<"FileIDs: "<<parT.ids.size<<endl;
	for(unsigned i = 0; i < parT.ids.size; ++i) {
		NetMsg netMsg;
		netMsg.setFileId(&parT.ids.ids[i]);
		netMsg.header.type = FIND_MSG;
		send_broadcast(&(netMsg.header));
	}
}


/*
	Cialo watku, uruchamianego pod wplywem otrzymania komunikatu UP_MSG.
	Jesli mamy zasob, o ktorym dostalismy inforamacje - odsylamy komunikat
	HAS_MSG czym powodujemy wywlaszczenie.
*/
void* up_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)// MAMY TEN ZASOB
	{
		printHeader(&netMsg->header);
		netMsg->setFileId(myFid);
		netMsg->header.type = HAS_MSG;                                                                                                           
		send_response(netMsg);
	}
	delete netMsg;
}

/*
	Cialo watku, uruchamianego pod wplywem otrzymania komunikatu HAS_MSG.
	Jesli z informacji o zasobie wynika, ze mamy do niego przypisanego
	wlasciciela pasozytniczego - nastepuje zmiana wlasciciela.
	W razie konfliktow wlascicieli i rownosci czasow dodania - rozsylane jest
	zadanie natychmiastowego usuniecia zasobu - FORCE_DEL
*/
void* has_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)	// MAMY TEN ZASOB
	{
		uint64_t myOwner = bytesToLong(myFid->owner);
		uint64_t recOwner = bytesToLong(netMsg->header.owner);
		if(myOwner != recOwner)
		{
			if(netMsg->header.time.ttime < myFid->time)
			{
				changeOwner(myFid,netMsg->header.owner, netMsg->header.time.ttime);
				if(myOwner == get_id())		//to byl nasz zasob - wywlaszczenie
				{
					safe_cout("Your resource (" + string(myFid->name) + " " + to_string(myFid->size)  + "b)confiscated! You were parasitic owner!\n>> ");
				}
			}
			else if(netMsg->header.time.ttime == myFid->time)
			{
				safe_cout("A conflict of adding the same resource by different users\n>> ");
				netMsg->header.type = FORCE_DEL_MSG;
				send_response(netMsg);
				deleteFile(myFid);
			}	
		}
	}
	delete netMsg;
}

/*
	Cialo watku, uruchamianego pod wplywem otrzymania komunikatu DEL_MSG.
	Funkcja natychmiast usuwa zasob, o ktorym otrzymala informacje, jesli go posiada 
	i spelniony jest ktorys z warunkow:
	a)informacje o wlascicielach sa zgodne 
	b)czas dodania zasobu przez wlasciciela uzyskany z wiadomosci jest wczesniejszy niz
	  zapisany w lokalnym wezle
*/
void* del_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)	// MAMY TEN ZASOB
	{
		uint64_t myOwner = bytesToLong(myFid->owner);
		uint64_t recOwner = bytesToLong(netMsg->header.owner);
		if(myOwner == recOwner)
		{
			deleteFile(myFid);
		}
		else if(netMsg->header.time.ttime < myFid->time)
		{
			deleteFile(myFid);
		}
	}
	delete netMsg;
}

/*
	Cialo watku, uruchamianego pod wplywem otrzymania komunikatu FORCE_DEL_MSG lub DONT_MSG.
	Funkcja natychmiast usuwa zasob, o ktorym otrzymala informacje, jesli go posiada.
*/
void* force_del_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)	// mamy ten zasob
	{
		deleteFile(myFid);
	}
	delete netMsg;
}

/*
	Cialo watku, uruchamianego pod wplywem otrzymania komunikatu FIND_MSG.
	Fukcja wyszukuje zasoby, ktore pasuja do przyslanych kryteriow wyszukiwania, a ktore
	mamy sciagniete w calosci. Wysyla informacje o tych zasobach wraz z wiadomoscia HAS_MSG
	do wezla, od ktorego otrzymalismy komunikat FIND_MSG na port, na ktorym nasłuchuje wątek
	wyszukiwania zasobow.
*/
void* find_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	vector<FileID*> myFids;
	myFids = myFindInStorage(&(netMsg->header));
	for(FileID* fid : myFids)
	{
		netMsg->setFileId(fid);
		netMsg->header.type = HAS_MSG;
		send_response(netMsg, FIND_PORT);
	}
	delete netMsg;
}

/*
	Cialo watku, uruchamianego pod wplywem otrzymania komunikatu OWN_MSG.
	Fukcja sprawdza, czy mamy zasob, o ktorym informacje otrzymalismy.
	Jesli nie, a bylismy jego wlascicielem, odsylamy do wezla, ktory przyslal 
	nam wiadomosc OWN_MSG komunikat DONT_MSG.
*/
void* own_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	uint64_t owner = bytesToLong(netMsg->header.owner);
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid==NULL && owner==id)	// NIE MAMY ZASOBU, A BYLISMY WLASCICIELAMI
	{	
		netMsg->header.type = DONT_MSG;
		send_response(netMsg);
	}
	delete netMsg;
}

/*
	Funkcja dla każdego zasobu, ktorego nie jest wlascicielem sprawdza, czy
	wlasciceil nie usunal tego zasobu (wysyla broadcast z informacja o zasobie i
	komunikatem OWN_MSG)
	Usuwa takze zasoby, ktorych powiazane pliki sa uszkodzone lub usuniete.
	Jesli wezel byl wlascicielem tych zasobow, rozsyla wiadomosc o ich usunieciu.
*/
void check_files_validations()
{
	list<FileID*> fileList = getFileIds();
	NetMsg netMsg;
	netMsg.header.type = OWN_MSG;
	for(FileID* fid : fileList)
	{
		if(bytesToLong(fid->owner) != id)
		{
			netMsg.setFileId(fid);
			send_broadcast(&(netMsg.header));
		}
	}

	fileList = getUnlinked();
	netMsg.header.type = DEL_MSG;
	for(FileID* fid : fileList)
	{
		if(bytesToLong(fid->owner) == id)
		{
			netMsg.setFileId(fid);
			send_broadcast(&(netMsg.header));
		}
		free((void*)fid->name);
		free((void*)fid->owner);
		delete fid;		
	} 
}

//Funkcja uruchamiająca procesy pobierania niedokończone podczas ostatniego działania aplikacji.
void run_downloadings()
{
	lockMetaDataMutex();
	for(list<Resource>::iterator it = metaData.begin(); it != metaData.end();it++){
		if(it->missingBlocks.size() != 0)
		{
			list<Resource>::iterator* param = new list<Resource>::iterator;
			*param = it;
			pthread_t receiverThreadTCP;
			if(pthread_create(&receiverThreadTCP, NULL, download_file, (void*) param)) 
			{
				safe_cout(downloadThreadFailed);
			}
		}

	}
	unlockMetaDataMutex();
}

#endif
