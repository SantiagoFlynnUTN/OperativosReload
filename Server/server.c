/*
    ** selectserver.c -- servidor de chat multiusuario
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

	#define PORT 9034   // puerto en el que escuchamos


void* holaMundo(void* arg){

	int numbytes;
	char buf[1000];
	int newfd;
	newfd = (int) arg;

	if (send(newfd, "asd\n", 4, 0) == -1)
			perror("send");
	if ((numbytes=recv(newfd, buf,1024, 0)) == -1) {
					perror("recv");
					exit(1);
	}
	buf[numbytes] = '\0';

	printf("Received: %s\n",buf);

	sleep(2);
	if (send(newfd, "hola\n", 5, 0) == -1)
			perror("send");
	if ((numbytes=recv(newfd, buf,1024, 0)) == -1) {
					perror("recv");
					exit(1);
	}
	buf[numbytes] = '\0';

	printf("Received: %s\n",buf);
	sleep(2);
	if (send(newfd, "bienvenido\n", 11, 0) == -1)
			perror("send");
	if ((numbytes=recv(newfd, buf,1024, 0)) == -1) {
					perror("recv");
					exit(1);
	}
	buf[numbytes] = '\0';

	printf("Received: %s\n",buf);
	sleep(2);
	if (send(newfd, "chau\n", 5, 0) == -1)
			perror("send");
	if ((numbytes=recv(newfd, buf,1024, 0)) == -1) {
					perror("recv");
					exit(1);
	}
	buf[numbytes] = '\0';

	printf("Received: %s\n",buf);
	sleep(2);

	pthread_exit(NULL);
}

int main(void)
{
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	struct sockaddr_in myaddr;     // dirección del servidor
	struct sockaddr_in remoteaddr; // dirección del cliente
	int fdmax;        // número máximo de descriptores de fichero
	int listener;     // descriptor de socket a la escucha
	int newfd;        // descriptor de socket de nueva conexión aceptada
	char buf[256];    // buffer para datos del cliente
	int nbytes;
	int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
	int addrlen;
	int i, j;
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

    int maxclientes, contador;
    maxclientes = 100;
    contador = 0;
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
						maxclientes = 100;
						pthread_t tid[maxclientes];
						rc = pthread_create(&tid[contador], NULL, holaMundo, newfd);
								if(rc) printf("no pudo crear el hilo");
						contador++;
						//verificar contadorde hilos (para cuando cerramos hilos)
					}
				}
			}
		}
	}



	return 0;
}
