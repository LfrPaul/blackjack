all: carte joueur client serveur

carte : source/carte.c 
	gcc -c source/carte.c -o build/carte.o

joueur : source/joueur.c 
	gcc -c source/joueur.c -o build/joueur.o

client : source/client.c build/joueur.o build/carte.o
	gcc source/client.c build/carte.o build/joueur.o -o client 

serveur: source/serveur.c build/joueur.o build/carte.o
	gcc source/serveur.c build/carte.o build/joueur.o -o serveur

clean:
	rm build/*.o client serveur

	