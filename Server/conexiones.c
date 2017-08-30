/*
 * conexiones.c
 *
 *  Created on: 7/8/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "serializacion.h"
#include "conexiones.h"

fd_set master;   // conjunto maestro de descriptores de fichero

void comprobarConexion(int numbytes, int socket){
	 if (numbytes <= 0) {
		// error o conexión cerrada por el cliente
		if (numbytes == 0) {
			// conexión cerrada
			printf("selectserver: socket %d hung up\n", socket);
		} else {
			perror("recv");
		}
		close(socket); // bye!
		FD_CLR(socket, &master); // eliminar del conjunto maestro

		pthread_exit(NULL);
	 }
}


void* manejarCliente(void* socket){
	int newfd;
	newfd = (int) socket;

	int numbytes;
	int buf;

	//bucleo hasta que se desconecte
	while(1){
		numbytes = recv(newfd, &buf, 1, 0); //leo el primer byte. Me dirá el tipo de paquete. (es un int)

		comprobarConexion(numbytes, newfd); //Me fijo si lo que recibí esta todo ok.

		manejarDatos(buf, newfd); //Si llegamos hasta acá manejamos los datos que recibimos.
	}
}

void* iniciarServer()
{

	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	struct sockaddr_in myaddr;     // dirección del servidor
	struct sockaddr_in remoteaddr; // dirección del cliente
	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int newfd;        // descriptor de socket de nueva conexión aceptada
	int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
	int addrlen;
	int i;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
														sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	// enlazar
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(PORT);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	// escuchar
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste

    int contador = 0;

	// bucle principal

	for(;;) {

		read_fds = master; // cópialo
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == listener) {
					// gestionar nuevas conexiones
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,
															 &addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // añadir al conjunto maestro
						if (newfd > fdmax) {    // actualizar el máximo
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
							"socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);

						int rc;
						pthread_t tid[MAXCLIENTES];
						rc = pthread_create(&tid[contador], NULL, manejarCliente, newfd);
								if(rc) printf("no pudo crear el hilo");
						contador++;
						//verificar contadorde hilos (para cuando cerramos hilos)
					}
				}
			}
		}
	}
}

void inicializarServer(){
	int rc;
	pthread_t tid;
	rc = pthread_create(&tid, NULL, iniciarServer, NULL);
		if(rc) printf("no pudo crear el hilo");
}
