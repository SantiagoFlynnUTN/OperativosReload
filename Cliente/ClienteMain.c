 /*
    ** client.c -- Ejemplo de cliente de sockets de flujo
    */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9034 // puerto al que vamos a conectar

#define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez

#define OK 1
#define ESTRUCTURA 2
#define ARCHIVO 3

int sockfd;

//https://es.wikibooks.org/wiki/Programaci%C3%B3n_en_C/Estructuras_y_Uniones
typedef struct {
	int tipoMensaje;
	char mensaje[100];
} __attribute__((packed))
mensajeCorto;


int funcionTest(){
	int salir = 1;
	int numbytes;
	char buf[MAXDATASIZE];

	while(salir){
		if ((numbytes=recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}
		buf[numbytes] = '\0';

		printf("Received: %s",buf);

		if (!strncmp(buf, "chau", 4)){

			if (send(sockfd, "Me llego chau\n", 13, 0) == -1)
					perror("send");

			close(sockfd);
			salir = 0;

		}
		else{
			if (send(sockfd, "Me llego inv\n", 12, 0) == -1)
								perror("send");
		}
	}
}


void enviarMensajeCorto(){
	mensajeCorto msj;
	msj.tipoMensaje = 2;
	printf("Escribe algo: ");
	memset(&msj.mensaje, 0, 100);
	gets(msj.mensaje);

	void* mensajeAEnviar = (void*) malloc(sizeof(mensajeCorto));
	memset(mensajeAEnviar, 0, sizeof(mensajeCorto));
	memcpy(mensajeAEnviar, &msj, sizeof(mensajeCorto));

	if (send(sockfd, mensajeAEnviar, sizeof(mensajeCorto), 0) ==-1)
		printf("No puedo enviar\n");
	free (mensajeAEnviar);
}

int main(int argc, char *argv[])
{
	int numbytes;
	char buf[MAXDATASIZE];
	struct sockaddr_in their_addr; // información de la dirección de destino


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(PORT);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(&(their_addr.sin_zero), 0, 8);  // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *)&their_addr,
										  sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	enviarMensajeCorto();

	//funcionTest();

	close(sockfd);

	return 0;
}
