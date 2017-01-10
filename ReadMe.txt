Interpreter z TIN

Program jest przeznaczony na system zgodny ze standardem POSIX (np. Linux). Aby uruchomiæ program na Windowsie, nale¿y mieæ zainstalowanego emulator Cygwin. 

Budowanie projektu z u¿yciem makefile. Aby skompilowaæ program, nale¿y mieæ zainstalowane programy: generator lekserów flex oraz generator parserów bison.

Pliki TIN.l, TIN.y to pliki wejœciowe tych programów i nie nale¿y ich edytowaæ. Pliki Ÿród³owe TIN.yy.c, TIN.tab.c oraz TIN.tab.h s¹ generowane przez te narzêdzia i te¿ nie nale¿y ich edytowaæ. 

Dzia³anie interpretera okreœlane jest przez procedury zdefiniowane w pliku p2p-actions.hpp. Znajduj¹ce siê tam procedury nale¿y wype³niæ treœci¹. Modyfikujemy tylko ten jeden plik! Ewentualnie jeszcze nag³ówek fileid.h. 

W pliku PrzykladyKomend.txt znajduj¹ siê przyk³adowe komendy. 

W plikach TIN dokumentacja.odf oraz TIN Gramatyka.odf s¹ stworzone przez nas dokumentacje protoko³u AAP oraz gramatyki poleceñ (któr¹ uproœci³em w ten sposób ¿e w komendzie UPLOAD zamiast SET NAME jest po prostu AS. Nie mo¿na te¿ ju¿ u¿ywaæ wyra¿eñ logicznych po komendzie UPLOAD, bo by³oby to bezsensowne semantycznie. Same wyra¿enia logiczne nie obs³uguj¹ operatorów >, <, >=, >=, bo by³oby to bezsensowne. Obs³ugiwany jest domyœlnie operator == dla liczb i !==! dla stringów.). 

Dalsze informacje w komentarzach w pliku p2p-actions.hpp. 