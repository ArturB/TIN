#ifndef DATA_H
#define DATA_H


#include <sys/stat.h>	
#include <fstream>		
#include <ctime>		
#include <mutex>		
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include "resource.h"
#include "netmsg.hpp"
#include "filedownload.hpp"


//sciezka do pliku konfiguracyjnego okreslajacego katalog, do ktorego zapisywane beda sciagniete pliki
const char* configFilePath = "./p2p.config";

/*
	struktura opakujaca std::mutex. Pozwala na przechowywanie go w deque.
	Przechowuje nazwe i size pliku, ktoremu odpowiada mutex.
*/
struct mutex_wrapper : std::mutex
{
	mutex_wrapper() = default;
	mutex_wrapper(string Name, uint64_t Size) : name(Name),size(Size)
	{
	};
	mutex_wrapper(mutex_wrapper const&other) noexcept : std::mutex()
	{
		name = other.name; 
		size = other.size;
	}	
	bool operator==(mutex_wrapper const&other) noexcept
	{
		return this==&other;
	}
public:
	string name;
	uint64_t size;
};



bool deleteFile(FileID* id);
vector<Resource>::iterator openResource(ResourceHeader *header);
vector<FileID*> myFindInStorage(ResourceHeader *header);
vector<Resource>::iterator openResourceByIterator(ResourceHeader * header,bool *isNotEmpty);
std::vector<Resource> getMetaData();
void printBits(size_t const size, void const * const ptr);
void copyBits(void const * const ptrSource,void const * const ptrDest,size_t const size);
void printHost(Host *h);
void printFileID(FileID *id);
FileFragment * getFileFragment(FileID *id, long number);
bool saveFileFragment(FileID *id, FileFragment *fragment);
bool initiateMutexes();
bool destroyMutexes();
void lockMetaDataMutex();
void unlockMetaDataMutex();
uint64_t get_id();
void printHeader(ResourceHeader* header);
uint64_t bytesToLong(const char* bytes);
std::vector<Resource> metaData;
pthread_mutex_t metaDataMutex;
pthread_mutex_t fileMutexDequeMutex;
string returnFilePath(const char * cfg_path, const char *filename);
vector<string> putToParts(string s, char dividerLeft, char dividerRight);
string buildFromParts(vector<string> parts,char dividerLeft, char dividerRight);
Resource *des2Resource(string build, char dividerLeft, char dividerRight);
string ser2Resource(Resource *r, char dividerLeft, char dividerRight);
FileID *des2FileID(string build, char dividerLeft, char dividerRight);
string ser2FileID(FileID *id, char dividerLeft, char dividerRight);
Host *des2Host(string build, char dividerLeft, char dividerRight);
string ser2Host(Host *host, char dividerLeft, char dividerRight);
sockaddr_in des2SockAddr_in(string build, char dividerLeft, char dividerRight);
string ser2SockAddr_in(sockaddr_in sa, char dividerLeft, char dividerRight);
deque<Host> *des2dequeHost(string build, char dividerLeft, char dividerRight);
string ser2dequeHost(deque<Host> *d, char dividerLeft, char dividerRight);
vector<long> des2vectorLong(string build, char dividerLeft, char dividerRight);
string ser2vectorLong(vector<long> vl, char dividerLeft, char dividerRight);
bool serializeMetaData();
bool deserializeMetaData();
list<FileID*> getUnlinked();
bool fileIsDownloading(vector<Resource>::iterator it);
bool setFileIsDownloading(vector<Resource>::iterator it);

bool fileIsDownloading(vector<Resource>::iterator it)
{
	lockMetaDataMutex();
	bool result = it->is_downloading;
	unlockMetaDataMutex();
	return result;
}

bool setFileIsDownloading(vector<Resource>::iterator it)
{
	lockMetaDataMutex();
	it->is_downloading = true;
	unlockMetaDataMutex();
}



/*	Dodanie pliku do wezla.
	pathName - sciezka do pliku na dysku
	filename - nazwa, pod jaka plik bedzie udostepniony jako zasob
	Funkcja tworzy nowy zasob, umieszcza go w vector<Resource> i zwraca do niego iterator.
	Jesli istnieje już zasob o tej samej nazwie i rozmiarze lub udostepniajacy plik spod tej samej lokalizacji,
	operacja dodania nie udaje sie i zwracany jest iterator do tego zasobu.
	Jesli sciezka do pliku jest nieprawidlowa - zwracany jest iterator::end()
*/
vector<Resource>::iterator addFile(string pathName, string fileName)
{
	string path = pathName;
	vector<Resource>::iterator it;

	lockMetaDataMutex();
	for(it = metaData.begin(); it<metaData.end(); it++){
		if(it->filePathName == path){
			unlockMetaDataMutex();
			return it;
		}
	}	
	unlockMetaDataMutex();
	
	ifstream file(path.c_str());
	if(!file.good())
		return metaData.end();

	struct stat buf;
	stat(path.c_str(),&buf);			
	uint64_t uSize = buf.st_size;			

	lockMetaDataMutex();

	//sprawdzenie, czy nie ma resourcu o takej samej nazwie i size
	for(it = metaData.begin(); it<metaData.end(); it++){
		if(!strcmp(it->id->name,fileName.c_str()) && it->id->size == uSize){
			unlockMetaDataMutex();
			return it;
		}
	}	
	unlockMetaDataMutex();

	uint64_t ownerId = get_id();
	char * ownerName =(char*)malloc(6);
	memcpy(ownerName,&ownerId,sizeof(char)*6);
	FileID* toReturn = new FileID();
	toReturn->name = strdup(fileName.c_str());
	toReturn->owner = ownerName;
	toReturn->size = uSize;
	
	Resource res;
	res.id = toReturn;
	res.filePathName = pathName;
	res.is_downloading = false;
	res.peers.clear();
	res.missingBlocks.clear();
	toReturn->time = time(0);
	
	lockMetaDataMutex();
	metaData.push_back(res);
	it = metaData.end();
	it--;
	unlockMetaDataMutex();
	return it;
}


/*
	Funkcja zwraca liste wszystkich wskazan do struktur FileID zwiazanych
	z zasobami Resource, wsytepujacymi w bazie wezla (zarowno tych, ktorych jestesmy wlascicielami jak i nie)
*/
list<FileID*> getFileIds()
{
	list<FileID*> toReturn;
	lockMetaDataMutex();
	for (Resource res : metaData){
		toReturn.push_back(res.id);
	}
	unlockMetaDataMutex();
	return toReturn;
}


/*
	Funkcja na podstawie przekazanego wskazania na strukture FileID usuwa zwiazany z nia zasob.
	Usuwa takze plik na dysku, zwiazany z danym zasobem. Zwraca true w przypadku zakonczenia
	operacji usuniecie zasobu.
*/
bool deleteFile(FileID * id)
{
	struct stat info;
	lockMetaDataMutex();
	for(vector<Resource>::iterator it = metaData.begin(); it<metaData.end();it++){
		if(strcmp(it->id->name,id->name)==0 && id->size == it->id->size)
		{
			if (!stat((it->filePathName).c_str(), &info))
			{
				cout << "Resource deleted! name: " +  string(it->id->name)  +  "; size: "  +  to_string(it->id->size) << endl;
				cout << flush;
				if( remove((it->filePathName).c_str()) != 0 )
	    				perror( "Error while deleting physical file" );
			}
			free((void*)it->id->name);
			free((void*)it->id->owner);	
			delete it->id;
			metaData.erase(it);
			unlockMetaDataMutex();
			return true;
		}
	}	
	unlockMetaDataMutex();
	return false;
}

/*
	Na podstawie przekazanego adresu struktury ResourceHeader funkcja zwraca
	adres struktury FileID, ktora ma zgodne pola name i size z tym, co znajduje sie 
	w strukturze ResourceHeader. Jesli w bazie zasobow nie ma odpowiadajacego obiektu,
	funkjca zwraca null.
*/
FileID * isFileInStorage(ResourceHeader * header)
{
	FileID *toReturn=__null;
	lockMetaDataMutex();
	for(Resource res : metaData){
		if(strcmp(res.id->name,header->name)==0 && res.id->size==header->size.longNum)
			{
				toReturn = res.id;
				unlockMetaDataMutex();
				return toReturn;
			}
	}
	unlockMetaDataMutex();
	return __null;
}

std::vector<Resource> getMetaData(){
	lockMetaDataMutex();
	vector<Resource> toReturn = metaData;
	unlockMetaDataMutex();
	return toReturn;
}

/*
	zmiana wlasciciela zasobu i czasu dodania zasobu, zwiazanego ze struktura FileID pod adresem id
	
*/
bool changeOwner(FileID * id, const char * newOwner, time_t addTime)
{
	char * ownerName =(char*)malloc(6);
	memcpy(ownerName,newOwner,sizeof(char)*6);
	id->owner = ownerName;
	id->time = addTime;
}


/*
	Funkcja zwraca iterator, ktory wskazuje na obiekt Reosurce, ktory mozna
	zidentyfikowac po strukturze ResourceHeader spod adresu przekazanego jako argument funkcji
	(zasob o takiej samej nazwie i rozmiarze, jak podaje obiket struktury).
	Jesli taki zasob nie istnieje w bazie, jest on tworzony.
*/
vector<Resource>::iterator openResource(ResourceHeader * header)
{
	pthread_mutex_lock(&metaDataMutex);
	
	for(vector<Resource>::iterator it = metaData.begin(); it!=metaData.end();it++){
		if(strcmp(it->id->name,header->name)==0 && it->id->size == header->size.longNum){
			pthread_mutex_unlock(&metaDataMutex);
			return it;
		}
	}
	pthread_mutex_unlock(&metaDataMutex);

	FileID* newID = new FileID();
	newID->size = header->size.longNum;
	newID->time = header->time.ttime;
	char *headerName = strdup(header->name);

	char * ownerName =(char*)malloc(6);
	memcpy(ownerName,&header->owner,sizeof(char)*6);
	
	Resource resource;
	resource.peers.resize(0);
	resource.id = newID;
	resource.id->name = headerName;
	resource.id->owner = ownerName;
	resource.is_downloading = false;
	for (long i = 1; i <= ((long)(resource.id->size / 1024)); i++)
		resource.missingBlocks.push_back(i);

	if(resource.id->size % 1024){
		resource.missingBlocks.push_back((long)((resource.id->size / 1024) + 1));
	}

	resource.filePathName = "";
	pthread_mutex_lock(&metaDataMutex);
	metaData.push_back(resource);
	vector<Resource>::iterator resourceIt = metaData.end();
	resourceIt--;
	pthread_mutex_unlock(&metaDataMutex);
	return resourceIt;
}

vector<Resource>::iterator getResource(ResourceHeader * header)
{
	pthread_mutex_lock(&metaDataMutex);

	for(vector<Resource>::iterator it = metaData.begin(); it != metaData.end();it++){
		if(strcmp(it->id->name,header->name)==0 && it->id->size == header->size.longNum){
			pthread_mutex_unlock(&metaDataMutex);
			return it;
		}
	}
	pthread_mutex_unlock(&metaDataMutex);
	return metaData.end();
}

bool isValidResource(vector<Resource>::iterator resource)
{
	pthread_mutex_lock(&metaDataMutex);

	bool result;
	result = (resource != metaData.end());
	pthread_mutex_unlock(&metaDataMutex);
	return result;
}


/*
	Usuniecie zasobu zwiazanego z plikiem spod lokalizacji podanej w argumencie wywolania.
	Jesli nie jestesmy wlascicielami zasobu - plik zostaje usuniety z dysku, w przeciwnym
	wypadku nie jest. Funkcja zwraca strukture FileID zawierajaca informacje o usunietym
	zasobie. Jesli w bazie nie ma zasobu zwiazanego ze wskazywanym plikiem - pole name
	w strukturze FileID rowne jest null.
*/
FileID deleteFile(string pathName)
{							
	FileID toReturn;
	
	lockMetaDataMutex();
	
	for(vector<Resource>::iterator it = metaData.begin(); it<metaData.end();it++){
		if(it->filePathName == pathName)
		{	
			toReturn.name = it->id->name;
			toReturn.owner = it->id->owner;
			toReturn.size = it->id->size;
			toReturn.time = it->id->time;
			delete it->id;
			if(bytesToLong(it->id->owner) != get_id())
			{
				struct stat info;
				if (!stat((it->filePathName).c_str(), &info))
				{
					cout << "Resource deleted! name: " << it->id->name << "; size: " << to_string(it->id->size) << endl;
					if( remove((it->filePathName).c_str()) != 0 )
	    					perror( "Error while deleting physical file" );
				}
			}
			metaData.erase(it);
			unlockMetaDataMutex();
			return toReturn;
		}
	}	
	unlockMetaDataMutex();
	
	toReturn.name = __null;
	return toReturn;
}

/*
	Funkcja zwraca liste adresow obiektow struktur FileID, zwiazanych
	z zasobami, ktorych lokalizacja na dysku jest bledna (plik przeniesiony
	lub usuniety). Usuwa tez zwiazane obiekty Resource.
*/
list<FileID*> getUnlinked()
{
	list<FileID*> result;
	lockMetaDataMutex();
	struct stat info;
	for(vector<Resource>::iterator it = metaData.begin(); it<metaData.end();it++){
		printFileID(it->id);
		if( it->missingBlocks.empty() && (stat(it->filePathName.c_str(), &info) != 0))
		{	
			result.push_front(it->id);
			metaData.erase(it);
			it--;
		}
	}	
	unlockMetaDataMutex();
	return result;
}

/*
	Funkcja zwraca wektor adresów struktur FileID zwiazanych z zasobami, 
	ktore mamy w calosci (sciagniety caly plik lub nasz plik),
	a ktore pasuja do struktury ResourceHeader pod adresem przekazanym do funkcji
	(pasujace pola name i size).
*/
vector<FileID*> myFindInStorage(ResourceHeader *header){
	vector<FileID*> vector;

	bool checkName = false;
	bool checkSize = false;
	
	uint64_t headerSize=header->size.longNum;
	
	if(header->name!=NULL)
		checkName = true;
	if(headerSize!=0)
		checkSize = true;
	
	lockMetaDataMutex();
	
	for(Resource res : metaData){
		if(checkName && checkSize){			
			if(strcmp(header->name,res.id->name)==0 && headerSize==res.id->size && res.missingBlocks.empty()){
				vector.push_back(res.id);
			}
		}else if(checkName && !checkSize && res.missingBlocks.empty()){
			if(strcmp(header->name,res.id->name)==0){
				vector.push_back(res.id);
			}
		}else if(!checkName && checkSize && res.missingBlocks.empty()){
			if(headerSize==res.id->size){
				vector.push_back(res.id);
			}
		}
	}
	unlockMetaDataMutex();
	return vector;
}

vector<Resource>::iterator openResourceByIterator(ResourceHeader * header,bool *isNotEmpty){
	vector<Resource>::iterator toReturn;
	char *headerName = strdup(header->name);

	lockMetaDataMutex();
	for(vector<Resource>::iterator it = metaData.begin(); it<metaData.end();it++){
		if(strcmp(it->id->name,headerName)==0 && it->id->size == header->size.longNum){

			unlockMetaDataMutex();
			*isNotEmpty = true;
			return it;
		}
	}
	unlockMetaDataMutex();
	*isNotEmpty = false;
	return toReturn;
}


void printHost(Host *h){
	if(h!=__null){
		cout<<h->sock<<" "<<h->asked_fragment<<" "<<h->get_fragment<<" "<<h->portion<<" ";
		cout<<h->time_of_start<<" "<<h->addr.sin_family<<" "<<h->addr.sin_port<<" ";
		cout<<h->addr.sin_addr.s_addr<<" "<<h->addr.sin_zero<<endl;
	}
}
void printBits(size_t const size, void const * const ptr){
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
		printf("|");
    }
    puts("");
}
void copyBits(void const * const ptrSource,void const * const ptrDest,size_t const size){
	unsigned char *bD = (unsigned char*) ptrDest;
	unsigned char *bS = (unsigned char*) ptrSource;
	unsigned char byte;
	unsigned char byteIn;
	for(int i=size-1;i>=0;i--){
		byteIn=0;		
		for (int j=7;j>=0;j--)
        {
            byte = (bS[i] >> j) & 1;
			
			if(byte == 1)
				{
				byteIn=byteIn<<1;
				byteIn=byteIn+1;
				}
			else
				byteIn=byteIn<<1;
			
        }
		
		bD[i] = byteIn;
	}
}

void printFileID(FileID *id){
	if(id!=__null){
		cout<<"name: "<<id->name<<" owner:"<< bytesToLong(id->owner)<<" size:"<< id->size;
	
		tm *ptm = localtime(&(id->time));
		char buffer[33];
		strftime(buffer,32,"%a, %d.%m.%Y %H:%M:%S", ptm);
		buffer[32] = '\0';
		cout<<" time:"<<buffer<<endl;
	}
}

/*
	Zwraca szukany FileFragment.
	FileID okresla, z ktorego pliku pochodzi fragment,
	a number ktory fragment to jest.
	
	Jezeli pozycja pliku znajduje sie w metaDanych, ale nie istnieje jako plik w pamieci,
	to jego pozycja jest kasowana w metaDanych.
	Jezeli nie udalo sie odnalezc pliku badz fragmentu w nim, zwraca null.
	
*/
FileFragment * getFileFragment(FileID *id, long number){
	bool isFind = true;
	string path;
	FileFragment *ff = new FileFragment();
	memset(ff->byteArray,0,sizeof(uint8_t)*1024);
	ff->number = number;
	
	lockMetaDataMutex();
	vector<Resource>::iterator res;
	for(vector<Resource>::iterator it = metaData.begin(); it<metaData.end();++it){
		if(it->id->size == id->size && strcmp(it->id->name,id->name)==0)
		{	res = it;
			for(long l : it->missingBlocks){
				if (l==number)
				{
					isFind = false;
				}		
			}
			path = it->filePathName;
		}
	}
	unlockMetaDataMutex();
	
	if(!isFind)
	{
		delete ff;
		return __null;
	}
	ifstream file;
	file.open(path);
	if(!file.good()){
	  lockMetaDataMutex();
	  metaData.erase(res);
	  unlockMetaDataMutex();
	  return __null;  
	}
	file.seekg(1024*(number-1),ios::beg);
	if(file.good()){
		isFind = true;
		file.read((char*)ff->byteArray,sizeof(uint8_t)*1024);
	}
	file.close();
	return ff;
}
/*
	Zapisuje fragment FileFragment fragment pliku okreslonego przez FileID id do pamieci.
	Wymagane jest, aby plik istnial w metadanych, oraz jako plik w pamieci - inaczej zwracane jest false.
	
	Jezeli pozycja pliku znajduje sie w metaDanych, ale nie istnieje jako plik w pamieci,
	to jego pozycja jest kasowana w metaDanych. 
	
	Zwraca false jezeli zapis sie nie powiodl, plik nie istnial w pamieci badz w metaDanych.
*/
bool saveFileFragment(FileID *id, FileFragment *fragment){
	vector<Resource>::iterator res;
	vector<long>::iterator blockToDelete;
	bool isMissing = false;
	char* fileName = strdup(id->name);
	uint64_t fileSize = id->size;
	
	lockMetaDataMutex();
	
	for(vector<Resource>::iterator it = metaData.begin(); it<metaData.end();++it){
		if(strcmp(it->id->name,fileName)==0 && it->id->size==fileSize){
			res = it;
			for(vector<long>::iterator lit = res->missingBlocks.begin(); lit<res->missingBlocks.end();lit++)
			{	
				if(*lit == fragment->number)
				{
					isMissing = true;
					blockToDelete = lit;
				}
			}
			if(isMissing)
			{
				break;
			}
		}
	}
	if(!isMissing){
		unlockMetaDataMutex();
		return true;	
	}
	
	//res->missingBlocks.erase(blockToDelete);
	string resFilePathName = res->filePathName;
	uint64_t resSize = res->id->size;
	unlockMetaDataMutex();
	
	fstream file;
	file.open(resFilePathName, ios::in|ios::out|ios::ate);
	if(!file.good()){
	  lockMetaDataMutex();
	  metaData.erase(res);
	  unlockMetaDataMutex();
	  return false;  
	}
	
	file.seekp((fragment->number-1)*1024,ios_base::beg);
	long amountToWrite = 1024;
	if ((fragment->number-1)*1024+amountToWrite>resSize)
		amountToWrite = (resSize-(fragment->number-1)*1024)%1025;
	file.write((char*)fragment->byteArray,amountToWrite);
	file.close();
        
        lockMetaDataMutex();
        
        res->missingBlocks.erase(blockToDelete);
        
        unlockMetaDataMutex();
	
	return true;
};

/*
	Inicjuje dwa mutexy uzywane przy dostepie do metaDanych oraz listyMutexow dla plikow.
*/
bool initiateMutexes(){
	if(pthread_mutex_init(&metaDataMutex,__null)!=0){
		cerr<<"metaDataMutex initialisation failed\n";
		return false;
	}
	if(pthread_mutex_init(&fileMutexDequeMutex,__null)!=0){
		cerr<<"fileMutexDequeMutex initialisation failed\n";
		return false;
	}
	
}
/*
	Zwalnia dwa mutexy uzywane przy dostepie do metaDanych oraz listyMutexow dla plikow.
*/
bool destroyMutexes(){
	pthread_mutex_destroy(&metaDataMutex);
	pthread_mutex_destroy(&fileMutexDequeMutex);
	return true;
}
/*
	Lock na mutexie dostepu do metaDanych
*/
void lockMetaDataMutex(){
	pthread_mutex_lock(&metaDataMutex);	
}

/*
	Unlock na mutexie dostepu do metaDanych
*/
void unlockMetaDataMutex(){
	pthread_mutex_unlock(&metaDataMutex);	
}

/*
	Funkcja zwraca id wezla na podstawie adresu MAC karty sieciowej.
	Preferowana jest karta ethernetowa, jesli takiej nie ma - pobierany
	jest adres MAC pierwszej karty sieciowej z listy.
*/
uint64_t get_id()
{
    uint64_t id = 0;
    ifstream stream("/sys/class/net/eth0/address",ifstream::in);
    if (stream.is_open())
    {
        string macAddres, tmp;
        std::getline(stream, macAddres);
        for(int i = 0; i < macAddres.size(); ++i)
            if(macAddres[i] != ':')
                tmp = tmp + macAddres[i];

        id = std::stoul(tmp, nullptr, 16);
        stream.close();
    }
    else
    {
		DIR           *d;
	  	struct dirent *dir;
	  	d = opendir("/sys/class/net");
	  	if (d)
		{
	   		while ((dir = readdir(d)) != NULL)
			{
				string path = "/sys/class/net/" + string(dir->d_name) + "/address";
				stream.open(path,ifstream::in);
			 		if(stream.is_open())
					{
						string macAddres, tmp;
						std::getline(stream, macAddres);
						for(int i = 0; i < macAddres.size(); ++i)
				    			if(macAddres[i] != ':')
				        			tmp = tmp + macAddres[i];

						id = std::stoul(tmp, nullptr, 16);
						stream.close();
						if(id!=0)
							break;
					}
			}
			closedir(d);
		}
    }
	if(id==0)
	{
		perror("Cannot get mac address. Network interfaces isn't configurated\n");
	}
	return id;
}
/*
	Template. Konertuje proste typy zmiennych jak liczby do stringa.
*/
template <typename T> inline std::string tToString(const T& t){
	std::stringstream ss;
	ss << t;
	return ss.str();
}

/*
	Template. Konertuje stringi na proste typy zmiennych jak liczby.
*/
template <typename T> inline T stringToT(std::string s){
	std::istringstream stream(s);
	T t;
	stream >> t;
	return t;
}
/*
	Funkcja przyjmujaca zserializowanego stringa. 
	Rozpakowuje go na czesci i zwraca w postaci vectora.
	Części moga skladac sie z większej ilosci czesci.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
vector<string> putToParts(string s, char dividerLeft, char dividerRight){

	vector<string> parts;
	const char *charArray = s.c_str();	
	string part="";
	int beginning =0;
	int counter =0;
	char c;
	for(int i=0;i<s.length();i++){
		c = charArray[i];
		if(c==dividerLeft){
			if(counter==0)
				beginning = i;
			counter++;
		}else if(c==dividerRight){
			counter--;
			if(counter==0){
				part = s.substr(beginning+1,i-beginning-1);
				parts.push_back(part);
			}
		}
	}
	
	return parts;
}
/*
	Funkcja przyjmujaca czesci z bedace stringai z vectora stringow.
	opakowuje je w jeden zserializowany string.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string buildFromParts(vector<string> parts,char dividerLeft, char dividerRight){
	string toReturn="";
	for(string part:parts){
		toReturn+=dividerLeft;
		toReturn+=part+dividerRight;
	}
	return toReturn;
}
/*
	Funkcja ta zamienia zserializowany string na obiekt typu Resource,
	do ktorego zwraca wskaznik.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
Resource *des2Resource(string build, char dividerLeft, char dividerRight){
	vector<string> parts = putToParts(build,dividerLeft,dividerRight);
	Resource *toReturn = new Resource();
	toReturn->id = des2FileID(parts.at(0),dividerLeft,dividerRight);
	toReturn->filePathName =parts.at(1);
	toReturn->peers = *des2dequeHost(parts.at(2),dividerLeft,dividerRight);
	toReturn->missingBlocks = des2vectorLong(parts.at(3),dividerLeft,dividerRight);
	toReturn->is_downloading = stringToT<bool>(parts.at(4));

	return toReturn;
}
/*
	Serializuje obiekt typu Resource do postaci stringa.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string ser2Resource(Resource *r, char dividerLeft, char dividerRight){
	string toReturn="";

	vector<string> parts;
	parts.push_back(ser2FileID(r->id,dividerLeft,dividerRight));
	parts.push_back(r->filePathName);
	parts.push_back(ser2dequeHost(&(r->peers),dividerLeft,dividerRight));
	parts.push_back(ser2vectorLong((r->missingBlocks),dividerLeft,dividerRight));
	parts.push_back(tToString<bool>(false));
	
	toReturn = buildFromParts(parts,dividerLeft,dividerRight);
	return toReturn;
}
/*
	Funkcja ta zamienia zserializowany string na obiekt typu FileID,
	do ktorego zwraca wskaznik.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
FileID *des2FileID(string build, char dividerLeft, char dividerRight){
	vector<string> parts = putToParts(build,dividerLeft,dividerRight);
	FileID *toReturn = new FileID();
	toReturn->name = strdup(parts.at(0).c_str());
	uint64_t owner = stringToT<uint64_t>(parts.at(1));
	char * ownerId =(char*)malloc(6);
	memcpy(ownerId,&owner,sizeof(char)*6);
	toReturn->owner = ownerId;
	toReturn->size = stringToT<uint64_t>(parts.at(2));
	toReturn->time = stringToT<time_t>(parts.at(3));
	return toReturn;
}
/*
	Serializuje obiekt typu FileID do postaci stringa.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string ser2FileID(FileID *id, char dividerLeft, char dividerRight){
	string toReturn="";
	char *name = strdup(id->name);
	uint64_t size = id->size;
	time_t time = id->time;
	vector<string> parts;
	parts.push_back(string(name));
	uint64_t owner = bytesToLong(id->owner);
	parts.push_back(tToString<uint64_t>(owner));
	parts.push_back(tToString<uint64_t>(size));
	parts.push_back(tToString<time_t>(time));
	toReturn = buildFromParts(parts,dividerLeft,dividerRight);
	return toReturn;
}
/*
	Funkcja ta zamienia zserializowany string na obiekt typu FileID,
	do ktorego zwraca wskaznik.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
Host *des2Host(string build, char dividerLeft, char dividerRight){
	vector<string> parts = putToParts(build,dividerLeft,dividerRight);
	Host *toReturn = new Host();
	toReturn->sock = stringToT<int>(parts.at(0));
	toReturn->asked_fragment = stringToT<int>(parts.at(1));
	toReturn->get_fragment = stringToT<int>(parts.at(2));
	toReturn->portion = stringToT<int>(parts.at(3));
	toReturn->time_of_start = stringToT<time_t>(parts.at(4));
	toReturn->addr = des2SockAddr_in(parts.at(5),dividerLeft,dividerRight);
	toReturn->addr_length = stringToT<socklen_t>(parts.at(6));
	return toReturn;
}
/*
	Serializuje obiekt typu Host do postaci stringa.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string ser2Host(Host *host, char dividerLeft, char dividerRight){
	string toReturn="";
	int sock = host->sock;
	int asked_fragment = host->asked_fragment;
	int get_fragment = host->get_fragment;
	int portion = host->portion;
	time_t time_of_start = host->time_of_start;
	struct sockaddr_in addr = host->addr;
	socklen_t addr_length = host->addr_length;
	vector<string> parts;
	parts.push_back(tToString(sock));
	parts.push_back(tToString(asked_fragment));
	parts.push_back(tToString(get_fragment));
	parts.push_back(tToString(portion));
	parts.push_back(tToString(time_of_start));
	parts.push_back(ser2SockAddr_in(addr,dividerLeft,dividerRight));
	parts.push_back(tToString(addr_length));
	toReturn = buildFromParts(parts,dividerLeft,dividerRight);
	return toReturn;

}

/*
	Funkcja ta zamienia zserializowany string na obiekt typu sockaddr_in.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
sockaddr_in des2SockAddr_in(string build, char dividerLeft, char dividerRight){
	vector<string> parts = putToParts(build,dividerLeft,dividerRight);
	sockaddr_in toReturn;
	toReturn.sin_family = stringToT<short>(parts.at(0));
	toReturn.sin_port = stringToT<unsigned short>(parts.at(1));
	toReturn.sin_addr.s_addr = stringToT<unsigned long>(parts.at(2));
	uint64_t zero = stringToT<uint64_t>(parts.at(3));
	memcpy(toReturn.sin_zero,&zero,sizeof(toReturn.sin_zero));
	return toReturn;
}
/*
	Serializuje obiekt typu sockaddr_in do postaci stringa.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string ser2SockAddr_in(sockaddr_in sa, char dividerLeft, char dividerRight){
	string toReturn="";
	
	uint64_t zero;
	memcpy(&zero,sa.sin_zero,sizeof(sa.sin_zero));
	vector<string> parts;
	parts.push_back(tToString(sa.sin_family));
	parts.push_back(tToString(sa.sin_port));
	parts.push_back(tToString(sa.sin_addr.s_addr));
	parts.push_back(tToString(zero));
	
	toReturn = buildFromParts(parts,dividerLeft,dividerRight);
	return toReturn;
}
/*
	Funkcja ta zamienia zserializowany string na obiekt typu deque<Host>.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
deque<Host> *des2dequeHost(string build, char dividerLeft, char dividerRight){
	vector<string> parts = putToParts(build,dividerLeft,dividerRight);
	deque<Host> *toReturn = new deque<Host>();
	Host *h;
	for(string part : parts){
		h = des2Host(part,dividerLeft,dividerRight);
		toReturn->push_back(*h);
	}
	return toReturn;
}
/*
	Serializuje obiekt typu deque<Host> do postaci stringa.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string ser2dequeHost(deque<Host> *d, char dividerLeft, char dividerRight){
	string toReturn="";
	
	vector<string> parts;
	for(Host h: *d)
	{
		parts.push_back(ser2Host(&h,dividerLeft,dividerRight));
	}
	toReturn = buildFromParts(parts,dividerLeft,dividerRight);
	return toReturn;
}
/*
	Funkcja ta zamienia zserializowany string na obiekt typu vector<long>.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
vector<long> des2vectorLong(string build, char dividerLeft, char dividerRight){
	vector<string> parts = putToParts(build,dividerLeft,dividerRight);
	vector<long> toReturn;
	long l;
	for(string part : parts){
		l = stringToT<long>(part);
		toReturn.push_back(l);
	}
	return toReturn;
}
/*
	Serializuje obiekt typu vector<long> do postaci stringa.
	dividerLeft i dividerRight odpowiadaja za oddzielanie czesci.
*/
string ser2vectorLong(vector<long> vl, char dividerLeft, char dividerRight){
	string toReturn="";
	
	vector<string> parts;
	for(long l : vl)
	{
		parts.push_back(tToString<long>(l));
	}
	toReturn = buildFromParts(parts,dividerLeft,dividerRight);
	return toReturn;
}
/*
	Zapisuje cala zserializowana metadate do pliku "_metaData.txt" w sciezce programu.
	Jako dividerLeft i dividerRight uzywa odpowiednio '<' i '>'
	Zwraca true jezeli utworzenie pliku sie powiodlo, false w przeciwnym przypadku.
*/
bool serializeMetaData(){
	lockMetaDataMutex();
	vector<Resource> container = metaData;
	unlockMetaDataMutex();
	vector<string> lines;
	for(Resource r : container)
		lines.push_back(ser2Resource(&r, '<','>'));
	ofstream file;
	file.open("_metaData.txt");
	if(!file.good())
		return false;
	for(string line : lines)
		file<<line<<endl;
	file.close();
	return true;
}

/*
	Otwiera plik z metaData _metaData.txt i deserializuje go w vector<Resource>
	bedacy metadanymi. 
	Zwraca true jezeli udalo jej sie odnalezc odpowiedni plik _metaData.txt
*/
bool deserializeMetaData(){
	vector<Resource> container;
	ifstream file("_metaData.txt");
	if(!file.good())
		return false;
	string line;
	while(getline(file,line)){
		container.push_back(*des2Resource(line,'<','>'));
	}

	lockMetaDataMutex();
	metaData=container;
	unlockMetaDataMutex();
	return true;
}

/*
	Pomocnicza funkcja, konwertujaca liczbe zakodowana w 6 bajtach na uint64_t
*/
uint64_t bytesToLong(const char* bytes)
{
	union ByteLong{
		char bytes[6];
		long number;
	}byteLong;
	memcpy(&byteLong, bytes, 6);
	return byteLong.number;
}




void printHeader(ResourceHeader* header)
{
	std::cout << "Header" << std::endl;
	std::cout << "Name: " << header->name << std::endl;
	std::cout << "Size: " << header->size.longNum << std::endl;
	std::cout << "Id: " << bytesToLong(header->owner) << std::endl;
	std::cout << "Time: " << header->time.ttime << std::endl;
	std::cout << "MsgType: " << header->type << std::endl;
}


/*
	Funkcja zwraca w postaci string'a wygenerowana sciezke do pliku o nazwie filename i tworzy tam pusty plik.
	cfg_path to sciezka do pliku konfiguracyjnego, w ktorym znajdziemy lokalizacje katalogu, w ktorym znalezc ma sie plik.
	Funkcja zwraca pusty string, jesli nie mozna otworzyc pliku konfiguracyjnego, podana tam sciezka jest nieprawidlowa lub
	nie mozna utworzyc nowego, pustego pliku.
*/
string returnFilePath(const char * cfg_path, const char *filename)
{
	struct stat info;
	char directorypath[400];
	ifstream configFile(cfg_path);
	if (!configFile.is_open()){
		perror("Cannot open config file.\n");
		return "";
	}
	configFile.getline(directorypath, 400);
	configFile.close();
	string fName(filename);
	string filePath = string(directorypath) + "/" +  fName;
	if (!stat(filePath.c_str(), &info))
	{
		int counter = 0;
		int index = fName.find_last_of('.');
		bool extension = true;
		string temp;
		if (index == string::npos)
		{
			extension = false;
		}
		do{
			counter++;
			temp = fName;
			filePath = extension ? string(directorypath) + "/" + temp.insert(index, to_string(counter)) : string(directorypath) + "/" + temp + to_string(counter);
		} while (stat(filePath.c_str(), &info) == 0);
	}

	fstream fs;
	fs.open(filePath, ios::out);
	if (fs.is_open())
		fs.close();
	else{
		perror("Cannot create file to downloading.\n");
		return "";
	}
	return filePath;

}

#endif

