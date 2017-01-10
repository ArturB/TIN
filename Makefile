#Makefile do projektu z TIN
#Artur M. Brodzki, Kalisz 2016

CC=g++
OS=$(shell uname)

all: TIN.y TIN.l p2p-actions.hpp
	bison -d -o TIN.tab.c TIN.y
	flex -o TIN.yy.c TIN.l
	$(CC) -o tin TIN.yy.c TIN.tab.c -std=gnu++11 -lpthread