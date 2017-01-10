/*
 * Akcje interpretera klienta z TIN
 * Definicje procedur wykoywanych w reakcji na komendy
 * Artur M. Brodzki, Kalisz 2016
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

void printFileID(FileID f) {
	if(f.name)
		cout<<"Name: "<<f.name<<", ";
	//if(f.owner)
	//	cout<<"Owner: "<<f.owner<<", ";
	if(f.size)
		cout<<"Size: "<<f.size<<", ";
	//if(f.time)
	//	cout<<"Time: "<<f.time;
	cout<<". "<<endl;
}


//deklaracje procedur
void  upload_action();
void* upload_thread(void*);
void  delete_action();
void* delete_thread(void*);
void  find_action();
void* find_thread(void*);
void* find_msg_sender_thread(void *par);
void* find_msg_sender_thread(void *par);
void* download_thread(void* par);
void  download_action();
void check_files_validations();
//uint64_t get_id();
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
void write_progress_of_download(vector<Resource>::iterator* file);
int get_next_fragments(Interval* result, vector<long>* input, int* intervals_count, int portion, int from);
struct FileIDStruct fileid_from_header(ResourceHeader* header);
ResourceHeader header_from_command_data(CommandData* data);

//mutex chroniący dostęp do stdout
pthread_mutex_t termtx;
//mutex chroniący dostęp do cmdata
pthread_mutex_t cmtx;

//komunikaty
const char* welcome = 
	"Welcome in Pompous AAP File Transfer Protocol v1.0!\nRemember: your computer is your friend.\nPlease type a command now.\n";
const char* prompt = ">> ";
const char* errMsg = "command error"; 
const char* threadFailed = "Creating command thread failed!\n>> ";
const char* receiverFailed = "Creating receiver thread failed!\n>> ";
const char* tcpThreadFailed = "Creating data-changing thread failed!\n>> ";

//zmienne

uint64_t id;




/***************************
	    PROCEDURY
 ***************************/

//bezpieczne wypisywanie na stdout z relizacją wzajemnego wykluczania
void safe_cout(string str) {
	pthread_mutex_lock(&termtx);
	cout<<str;
	fflush(stdout);
	pthread_mutex_unlock(&termtx);
}

//procedura wołana bezpośrednio po uruchomieniu programu
void main_init() {


	pthread_mutex_init(&termtx, NULL);
	pthread_mutex_init(&cmtx, NULL);
	initiateMutexes();
	id = get_id();
	//std::cout << "POBIERAM DANE:" << deserializeMetaData() << std::endl;

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
	

	check_files_validations();
}

//destrukcja muteksów i innych aktywów
void main_destroy() {
	pthread_mutex_destroy(&termtx);
	pthread_mutex_destroy(&cmtx);
	destroyMutexes();
	//std::cout << "ZRZUCAM DANE:" << serializeMetaData() << std::endl;
}


void upload_action() {
	pthread_t uploadThread;
	if(pthread_create(&uploadThread, NULL, upload_thread, (void*)&cmdata))
	{ 
		safe_cout(threadFailed);
	}
}

void* upload_thread(void* par) {
	struct CommandData parT = *((struct CommandData*)par);
	safe_cout("Uploading file " + parT.fileName + "...\n>> ");
	FileID* fid;
	fid = addFile(parT.filePath, parT.fileName);
	if(fid == NULL)
	{
		safe_cout("Uploading unsuccessfull!\n>> ");
	}
	else
	{
		safe_cout("File " + parT.fileName + " uploaded!\n>> ");
		NetMsg netMsg;
		netMsg.setFileId(fid);
		netMsg.header.type = UP_MSG;
		send_broadcast(&(netMsg.header));
	}
}

void delete_action() {
	pthread_t deleteThread;
	if(pthread_create(&deleteThread, NULL, delete_thread, (void*)&cmdata))
	{
		safe_cout(threadFailed);
	}
}


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
			free((void*)fid.name);
			free((void*)fid.owner);
		}
	}
}

void* receiver_thread_TCP(void* par) 
{
	//safe_cout("receiver_thread_TCP\n>> ");
	int sock, length;
	struct sockaddr_in server;
	struct sockaddr_in client;
	length = sizeof(client);
	int msgsock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		perror("DOWN loop: Opening stream socket");
		exit(1);
	}

	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char *) &optval , sizeof(optval));

	server.sin_family= AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	if (bind(sock, (struct sockaddr *) &server, sizeof server) == -1)
	{
		perror("DOWN loop:Binding stream socket");
		exit(1);
	}

	if(listen(sock, 5)!= 0)
		perror ("DOWN loop: Listen call error");

	do {

		msgsock = accept(sock,(struct sockaddr *) 0,(socklen_t*) 0);

		if (msgsock == -1 )
			perror("DOWN loop: Accept call error");
		else 
		{
			pthread_t responseTCP;
			if(pthread_create(&responseTCP, NULL, response_down_tcp, &msgsock)) 
			{
				safe_cout(tcpThreadFailed);
			}
		}
		//close(msgsock);
	} while(true);

	exit(0);
}


struct FileIDStruct fileid_from_header(ResourceHeader* header)
{
	struct FileIDStruct id;
	
	int i;
	for(i = 0; header->name[i] != 0 && i < 256; ++i);

	id.name = new char[i];
	id.owner = new char[6];

	memcpy((void*) id.name, header->name, i);
	memcpy((void*) id.owner, header->owner, 6);
	id.size = header->size.longNum;
	id.time = header->time.ttime;

	return id;
}

void* response_down_tcp(void* par)
{
	//safe_cout("response_down_tcp\n>> ");
	int msgsock = *((int*)par);
	int rec_bytes;
	DownMsg down_msg;
	int number_of_block;	

	socklen_t length;
	struct sockaddr_in clientaddr;
	length = sizeof(clientaddr);

	rec_bytes = recvfrom(msgsock, &down_msg, sizeof(down_msg), 0, (struct sockaddr *) &clientaddr,  &length);

	if (rec_bytes < 0)
	{
		perror("ERROR in recvfrom");
		close(msgsock);	
		return NULL;
	}

	if (rec_bytes < MIN_MSG_SIZE)
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
				send_cant = true;
			else
			{
				if (sendto(msgsock, file_fragment, sizeof(FileFragment), 0, 
					(sockaddr *)&clientaddr, sizeof(&clientaddr)) < 0)
				{
					delete file_fragment;
					close(msgsock);
					return NULL;		
				}
				delete file_fragment;
			}
		}
	}
	if(send_cant)
	{
		net_msg.header.type = CANT_MSG;
		if (sendto(msgsock, &net_msg.header, MIN_MSG_SIZE, 0, 
			(sockaddr *)&clientaddr, sizeof(&clientaddr)) < 0)
		{
			close(msgsock);
			return NULL;
		}
	}
	close(msgsock);
}


int get_next_fragments(Interval* result, vector<long>* input, int* intervals_count, int portion, int from)
{
	bzero(result, 64*sizeof(Interval));

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


bool send_down_message(DownMsg* down_msg, int socket_tcp)
{

	if (send(socket_tcp, down_msg, sizeof(DownMsg), 0) < 0)
	{
		perror("SEND");
		close(socket_tcp);
		return false;
	}
	return true;
}


bool recive_fragment(deque<Host>::iterator *host, FileDownload *file_downloading, int* fragment_counter)
{	

	struct sockaddr_in *clientaddr;
	int length;
	char buffor[MAX_MSG_SIZE];
	int rec_bytes = recvfrom((*host)->sock, &buffor, sizeof(FileFragment), 0, (struct sockaddr *) clientaddr,  (socklen_t*) &length);

	if (rec_bytes < 0)
	{
		perror("Recvfrom");
		close((*host)->sock);	
		(*host) = file_downloading->using_hosts.erase((*host));
		return false;
	}
	else if(rec_bytes == 0)
	{
		safe_cout("Close conection\n>> ");
		close((*host)->sock);
		return true;
	}
	else if(rec_bytes > MIN_MSG_SIZE)
	{
		if(rec_bytes == sizeof(FileFragment))
		{
			FileFragment file_fragment;
			memcpy(&file_fragment, buffor, sizeof(FileFragment));
			if(!saveFileFragment((*file_downloading->file)->id, &file_fragment))
			{
				return false;
			}


			vector<Resource>::iterator* res = file_downloading->file;
			if((*host)->is_all_fragments() && (*res)->missingBlocks.size()!= 0) 
			{
				if(file_downloading->speed_in_hosts((*host)->get_speed()) == HOST_NUMER)
				{
					close((*host)->sock);
					file_downloading->avaiable_hosts.push_back(*(*host));
					(*host) = file_downloading->using_hosts.erase((*host));
				}
				else
				{
					if(file_downloading->speed_in_hosts((*host)->get_speed() < 3 ))
					{
						(*host)->grow_portion();
					}

					DownMsg msg;

					bzero(&msg.header.name, sizeof(msg.header.name));
					memcpy(&msg.header.name, (*file_downloading->file)->id->name, sizeof(msg.header.name));		

					msg.header.size.longNum = (*file_downloading->file)->id->size;	
					msg.header.time.ttime = (*file_downloading->file)->id->time;	
					memcpy(&msg.header.owner, (*file_downloading->file)->id->owner, sizeof(msg.header.owner));	
				
					msg.header.type = DOWN_MSG;	
				
					int internval_count;
					*fragment_counter = get_next_fragments(msg.fragments, &(*file_downloading->file)->missingBlocks, &internval_count, (*host)->portion, *fragment_counter);

					close((*host)->sock);
					if((*host)->connect_host_socket())
					{
						send_down_message(&msg, (*host)->sock);
					}
					else
					{
						(*host) = file_downloading->using_hosts.erase((*host));
					}
				}
			}
		}
		else if(rec_bytes == sizeof(ResourceHeader))
		{
			ResourceHeader header;
			memcpy(&header, &buffor, sizeof(header));
			if(header.type == CANT_MSG)
				(*host) = file_downloading->using_hosts.erase((*host));
			return false;
		}
		else
			return true;
	}
	return true;
}

void write_progress_of_download(vector<Resource>::iterator* file, bool break_downloading)	
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
	if(
		pthread_create(&downloadThread, NULL, download_thread, (void*)&cmdata) 
	) { // błąd przy tworzeniu wątku
		safe_cout(threadFailed);
	}
}


ResourceHeader header_from_command_data(CommandData* data)
{
	ResourceHeader result;

	bzero(result.name, sizeof(result.name));
	int i;


	const char *cstr = data->fileName.c_str();

	memcpy((void*) result.name, cstr, data->fileName.size());	// Z USUWANIEM ""
	result.size.longNum = data->fileSize;

	result.time.ttime = 0;
	bzero(result.owner, sizeof(result.owner));
	return result;
}


void* download_thread(void* par) {
	int info_message = 0;
	int fragment_counter = 0;

	safe_cout("Dowloading...\n>> ");
	
	ResourceHeader header = header_from_command_data((struct CommandData*)par);
	vector<Resource>::iterator file = getResource(&header);
	
	bool is_valid =  isValidResource(file);
	if(!is_valid)
	{
		CommandData command_data;
		command_data.ids.size = 1;
		FileID id = fileid_from_header(&header);
		command_data.ids.ids = new FileID;
		command_data.ids.ids[0] = fileid_from_header(&header);
		find_thread((void*)&command_data);
	}
	else if(file->is_downloading)
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

	file->filePathName = returnFilePath(configFilePath, file->id->name);
	DownMsg down_msg; 
	FileDownload file_downloading(&file);
	

	file_downloading.avaiable_hosts = file->peers; 

	down_msg.header.type = DOWN_MSG;
	bzero(&down_msg.header.name, sizeof(down_msg.header.name));

	int name_length;

	for(name_length = 0; file->id->name[name_length] != 0 && name_length < 256; ++name_length);
	memcpy((void*) &down_msg.header.name, file->id->name, name_length);
	down_msg.header.size.longNum = file->id->size;
	down_msg.header.time.ttime = file->id->time;
	memcpy((void*) &down_msg.header.owner, file->id->owner, 6);

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
			FileID id = fileid_from_header(&header);
			command_data.ids.ids = new FileID[1];
			command_data.ids.ids[0] = fileid_from_header(&header);

			while(file_downloading.using_hosts.size() == 0 && file_downloading.avaiable_hosts.size() == 0)
			{
				sleep(TIMEOUT_SEC_FIND_AGAIN);
				find_thread((void*)&command_data);

				file_downloading.avaiable_hosts = file->peers;
			}
			
			while(file_downloading.using_hosts.size() < HOST_NUMER && file_downloading.avaiable_hosts.size() > 0)
			{
				deque<Host>::iterator iterator = file_downloading.avaiable_hosts.begin();
				file_downloading.using_hosts.push_back(*iterator);
				file_downloading.avaiable_hosts.erase(iterator);

				Host* tmp = &file_downloading.using_hosts.back();

				if(tmp->connect_host_socket())
				{
					int intervals_count;
					fragment_counter = get_next_fragments(down_msg.fragments, &file->missingBlocks, &intervals_count, START_FRAGMENT_COUNT, fragment_counter);
					send_down_message(&down_msg, tmp->sock);
				}
				else
				{
					file_downloading.using_hosts.pop_back();
				}
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
			return NULL;
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

		if(!recive_any_fragment)
		{
			close_hosts_sockets(&file_downloading.using_hosts);
			file_downloading.using_hosts.clear();
			file_downloading.avaiable_hosts.clear();

		}

		double percent = ((double)(file_downloading.fragments - file->missingBlocks.size())/((double)file_downloading.fragments));
		percent *= 100;

		//if((int)percent % 10 < 5) 
		//{
			if(info_message + 1 <= (int)percent/10)
			{
				info_message = (int)percent/10;
				write_progress_of_download(&file, false);
			}
		//}
		
		if(file->missingBlocks.size() == 0)
		{
			safe_cout("Downloading ends!...\n>> ");
			close_hosts_sockets(&file_downloading.using_hosts);
			return NULL;
		}

	}

	return NULL;
}

void close_hosts_sockets(deque<Host> *collection)
{
	deque<Host>::iterator it = collection->begin();
	while(it != collection->end())
	{
		close(it->sock);
		++it;
	}
}

//procedura wątku nasłuchującego komunikatów
//od innych węzłów sieci
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
		cout<<"Odebralem cos na sockecie\n";
		if (n < 0)
		{
			perror("ERROR in recvfrom");	
			break;
		}
		if(clientaddr.sin_addr.s_addr == server.sin_addr.s_addr)	//Odebralismy wiadomosc, ktora sami wyslalismy //POPRAWKI TO NIE BEDZIE DZIALAC NIESTETY!
		{
			cout << "Odebralismy wiadomosc, ktora sami wyslalismy" << endl;
			continue;
		}

		NetMsg *netMsg = new NetMsg(&header, &clientaddr, &length); //POPRAWKI		

		switch(header.type)
		{
			pthread_t newThread;
			case UP_MSG:
				if(pthread_create(&newThread, NULL, up_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				// W NOWYM WATKU: SPRAWDZENIE CZY MAMY TAKI PLIK, JEZELI TAK TO ODSYLAMY OWN
				break;
			case HAS_MSG:
				//cout << "HAS" << endl;
				if(pthread_create(&newThread, NULL, has_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				// NOWY WATEK, SPRAWDZAMY CZY POSIADAMY PLIK I CZY JEST NASZ, JEZELI NIE WYWLASZCZAMY, DODAJEMY INFO NT. PLIKU
				break;
			case DEL_MSG:
				//cout << "USUWANIE" << endl;
				if(pthread_create(&newThread, NULL, del_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				//NOWY WATEK,JEZELI TO NASZ ZASOB, ODSYLAMY HAS, JEZELI MAMY GO TO USUWAMY I PRZERYWAMY TRANSAKCJE (JAK?)
				break;
			case FORCE_DEL_MSG:
			case DONT_MSG:
				//NOWY WATEK,USUWAMY ZASOB I PRZERYWAMY TRANSAKCJE
				if(pthread_create(&newThread, NULL, force_del_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;	
			case FIND_MSG:
				if(pthread_create(&newThread, NULL, find_msg_reaction, netMsg)) 		//DO POPRAWKI - ZMIENNA AUTOMATYCZNA A NOWY WATEK, IDZIE W PIZDU
				{
					safe_cout(receiverFailed);
				}
				//NOWY WATEK, JEZELI MAMY ODSYLAMY HAS
				break;	
			case OWN_MSG:
				if(pthread_create(&newThread, NULL, own_msg_reaction, netMsg)) 
				{
					safe_cout(receiverFailed);
				}
				break;	
			default:
				//error!
				break;
		}

	}
	close(sock);
}

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
		(sockaddr *)&Target_addr, sizeof(Target_addr)) < 0)	//POPRAWKI &header i &Target_addr
	{
		perror("Send call error");
		close(sock);
		return;
	}

	close(sock);
}

void send_response(NetMsg *data, int port)
{
	int sock;
	char msg[MIN_MSG_SIZE];

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
		(sockaddr *)Target_addr, sizeof(*Target_addr)) < 0)	//POPRAWKI
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

//procedura wątku obsługi komendy find
void* find_thread(void* par) {
	struct CommandData parT = *((struct CommandData*)par);
	int sock;
	int n;

	struct sockaddr_in server;
	struct sockaddr_in clientaddr;		//POPRAWKI
	socklen_t length;
	length = sizeof(clientaddr);	//POPRAWKI
	ResourceHeader* header = new ResourceHeader(); //POPRAWKI
	vector<Resource>::iterator resourceIt;
	set<FileID*> found;

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (sock == -1) 
	{
		perror("Opening stream socket");
		safe_cout("Unsuccessful find operation. Network problem\n>> ");
		return NULL;
	}

	struct timeval tv;
	tv.tv_sec = 1;  /* 1 Secs Timeout */
	tv.tv_usec = 0; 

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

	server.sin_family= AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(FIND_PORT);			
	

	if (bind(sock, (struct sockaddr *) &server, sizeof server) == -1)
	{
		perror("Binding stream socket");
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

ResourceHeader miodek;

	while(time(0) - now < 5)
	{
		n = recvfrom(sock, header, MIN_MSG_SIZE, 0, (struct sockaddr *) &clientaddr,  &length);	//CZEMU HEADER NIE JEST ZAINICJALIZOWANE?!

		if(errno == EAGAIN || errno == EWOULDBLOCK)
		{
			//safe_cout("TimeOut!!!...\n>> ");
			errno = 0;
			continue;			
		}
		if (n != MIN_MSG_SIZE)
		{
			perror("ERROR in recvfrom");	
			return NULL;
		}
		if(header->type != HAS_MSG)
		{
			continue;
		}
		resourceIt = openResource(header);
		bool isUnique = true;
		for(int i=0; i<(*resourceIt).peers.size(); i++)
		{
			if((*resourceIt).peers[i].addr.sin_addr.s_addr == clientaddr.sin_addr.s_addr)
			{
			  	isUnique = false;
				break;
			}
		}
		if(isUnique)
		{
			(*resourceIt).peers.push_back(Host(&clientaddr,&length));
		}

		uint64_t headerOwner = bytesToLong(header->owner);
		uint64_t resourceOwner = bytesToLong((*resourceIt).id->owner);
		found.insert((*resourceIt).id);



		if(headerOwner != resourceOwner)
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

		if(parT.find_one || (parT.find_first && found.size() == parT.find_first_count))
		{
			break;
		}
	}
	close(sock);
	if(found.empty())
	{
		safe_cout("No results!\n>> ");
	}
	else
	{
		safe_cout("Results:\n>> ");
		for(FileID *fid : found) {
			printFileID(*fid);
		}
		safe_cout("\n>> ");

	}
}

//PORCEDURA WATKU, KTORY WYSYLA KOMUNIKATY FIND, NIE WIEM, CZY NOWY WĄTEK JEST KOCIECZNIE POTRZEBNY
void* find_msg_sender_thread(void *par)
{
	struct CommandData parT;
	parT.ids = ((struct CommandData*)par)->ids;
	//cout<<"FileIDs: "<<parT.ids.size<<endl;
	for(unsigned i = 0; i < parT.ids.size; ++i) {
		NetMsg netMsg;
		netMsg.setFileId(&parT.ids.ids[i]);
		netMsg.header.type = FIND_MSG;
		send_broadcast(&(netMsg.header));
	}
}


//SPRAWDZENIE CZY MAMY TAKI PLIK, JEZELI TAK TO ODSYLAMY HAS
void* up_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)// MAMY TEN PLIK!
	{
		//std::cout<< "WYWŁASZCZAMYYYY!" << std::endl;
		printHeader(&netMsg->header);
		netMsg->setFileId(myFid);
		netMsg->header.type = HAS_MSG;                                                                                                           
		send_response(netMsg);
	}
	delete netMsg;
}

//SPRAWDZAMY CZY POSIADAMY PLIK I CZY JEST NASZ, JEZELI NIE WYWLASZCZAMY
void* has_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)	// MAMY TEN PLIK!
	{
		uint64_t myOwner = bytesToLong(myFid->owner);
		uint64_t recOwner = bytesToLong(netMsg->header.owner);
		if(myOwner != recOwner)
		{
			if(netMsg->header.time.ttime < myFid->time)
			{
				changeOwner(myFid,netMsg->header.owner, netMsg->header.time.ttime);
			}
			else if(netMsg->header.time.ttime == myFid->time)
			{
				netMsg->header.type = FORCE_DEL_MSG;
				send_response(netMsg);
				deleteFile(myFid);
			}	
		}
	}
	delete netMsg;
}

//NOWY WATEK,JEZELI TO NASZ ZASOB, ODSYLAMY HAS, JEZELI MAMY GO TO USUWAMY I PRZERYWAMY TRANSAKCJE
void* del_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)	// MAMY TEN PLIK!
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

//USUN ZASOB BEZ ZBEDNYCH PYTAN
void* force_del_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid!=NULL)	// MAMY TEN PLIK!
	{
		deleteFile(myFid);
	}
	delete netMsg;
}

//NOWY WATEK, JEZELI MAMY ODSYLAMY HAS
void* find_msg_reaction(void* par)
{
//cout << "find_msg_reaction" << endl;
	NetMsg *netMsg = (NetMsg*) par;
	vector<FileID*> myFids;
//cout << netMsg->header.name << endl;
	myFids = myFindInStorage(&(netMsg->header));
	for(FileID* fid : myFids)
	{
		netMsg->setFileId(fid);
		netMsg->header.type = HAS_MSG;
		send_response(netMsg, FIND_PORT);
	}
	delete netMsg;
}

//NOWY WATEK, JEZELI NIE MAMY ZASOBU, DONT
void* own_msg_reaction(void* par)
{
	NetMsg *netMsg = (NetMsg*) par;
	FileID *myFid;
	uint64_t owner = bytesToLong(netMsg->header.owner);
	myFid = isFileInStorage(&(netMsg->header));
	if(myFid==NULL && owner==id)	// NIE MAMY PLIKU, A BYLISMY WLASCICIELAMI
	{	
		netMsg->header.type = DONT_MSG;
		send_response(netMsg);
	}
	delete netMsg;
}

void check_files_validations()
{
	//SPRAWDZENIE WSZYSTKICH PLIKOW, TYLKO PYTANIE JAK NAPISAC DO ICH WLASCICIELI SKORO ZNAMY MAC ALE NIE ZNAMY IP? HMM -- BROADCAST
	list<FileID*> fileList; //potrzebna taka funkcja, ktora zwraca wszystkie FileID
	NetMsg netMsg;
	netMsg.header.type = OWN_MSG;
	for(FileID* fid : fileList)
	{
		netMsg.setFileId(fid);
		send_broadcast(&(netMsg.header));
	} 	
}

#endif
