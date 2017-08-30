/*
 * serializacion.c
 *
 *  Created on: 8/8/2017
 *      Author: utnso
 */

#include "serializacion.h"
#include <stdio.h>

void manejarDatos(int buf, int socket){
	switch(buf){
	case OK:
		printf("socket %i dice OK\n", socket);
		break;
	case ESTRUCTURA:
		printf("crear funcion para deserializar struct\n");
		break;
	case ARCHIVO:
		printf("crear funcion para deserializar archivo\n");
		break;
	}
}
