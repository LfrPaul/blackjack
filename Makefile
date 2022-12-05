all: clean bal3_s bal3_c serveur

bal3_c: bal3_c.c
	gcc bal3_c.c -o bal3_c

bal3_s: bal3_s.c
	gcc bal3_s.c -o bal3_s

serveur: serveur.c
	gcc serveur.c -o serveur
	gcc client.c -o client

clean:
	rm -f *.o

	