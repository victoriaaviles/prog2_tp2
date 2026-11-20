CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11
LDLIBS  = -lm
OBJETOS = main.o dataset.o normaliza.o knn.o
BIN     = ../bin

tudo: knn
	mkdir -p $(BIN)
	cp knn $(BIN)/knn

knn: $(OBJETOS)
	$(CC) $(OBJETOS) -o knn $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c dataset.h knn.h normaliza.h
dataset.o: dataset.c dataset.h
normaliza.o: normaliza.c normaliza.h dataset.h
knn.o: knn.c knn.h dataset.h

limpa:
	rm -f *.o

faxina: limpa
	rm -f knn $(BIN)/knn

.PHONY: tudo limpa faxina