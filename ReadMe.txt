Interpreter z TIN

Program jest przeznaczony na system zgodny ze standardem POSIX (np. Linux). Aby uruchomi� program na Windowsie, nale�y mie� zainstalowanego emulator Cygwin. 

Budowanie projektu z u�yciem makefile. Aby skompilowa� program, nale�y mie� zainstalowane programy: generator lekser�w flex oraz generator parser�w bison.

Pliki TIN.l, TIN.y to pliki wej�ciowe tych program�w i nie nale�y ich edytowa�. Pliki �r�d�owe TIN.yy.c, TIN.tab.c oraz TIN.tab.h s� generowane przez te narz�dzia i te� nie nale�y ich edytowa�. 

Dzia�anie interpretera okre�lane jest przez procedury zdefiniowane w pliku p2p-actions.hpp. Znajduj�ce si� tam procedury nale�y wype�ni� tre�ci�. Modyfikujemy tylko ten jeden plik! Ewentualnie jeszcze nag��wek fileid.h. 

W pliku PrzykladyKomend.txt znajduj� si� przyk�adowe komendy. 

W plikach TIN dokumentacja.odf oraz TIN Gramatyka.odf s� stworzone przez nas dokumentacje protoko�u AAP oraz gramatyki polece� (kt�r� upro�ci�em w ten spos�b �e w komendzie UPLOAD zamiast SET NAME jest po prostu AS. Nie mo�na te� ju� u�ywa� wyra�e� logicznych po komendzie UPLOAD, bo by�oby to bezsensowne semantycznie. Same wyra�enia logiczne nie obs�uguj� operator�w >, <, >=, >=, bo by�oby to bezsensowne. Obs�ugiwany jest domy�lnie operator == dla liczb i !==! dla string�w.). 

Dalsze informacje w komentarzach w pliku p2p-actions.hpp. 