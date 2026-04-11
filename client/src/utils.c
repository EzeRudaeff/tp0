#include "utils.h"


void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; esto es para server

	error=getaddrinfo(ip, puerto, &hints, &server_info);
	if (error!=0){
		printf("getaddrinfo: %s\n", gai_strerror(error));
		abort();
	}

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,server_info->ai_socktype,server_info->ai_protocol);
	if(socket_cliente==-1){
		printf("Error al generar socket cliente");
		abort();
	}

	// Ahora que tenemos el socket, vamos a conectarlo
	error=connect(socket_cliente,server_info->ai_addr,server_info->ai_addrlen);
	if(error!=0){
		perror("conexión: ");
		abort();
	}
	printf("Conexión exitosa!");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete)); //Inicializa puntero a t_paquete

	paquete->codigo_operacion = MENSAJE; //asigna codigo MENSAJE del enum
	paquete->buffer = malloc(sizeof(t_buffer)); //inicializa puntero
	paquete->buffer->size = strlen(mensaje) + 1; //guarda el largo del mensaje + \0
	paquete->buffer->stream = malloc(paquete->buffer->size); //inicializa puntero
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size); //guarda los bytes

	int bytes = paquete->buffer->size + 2*sizeof(int); //el largo del mensaje + 2 enteros

	void* a_enviar = serializar_paquete(paquete, bytes); 

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
