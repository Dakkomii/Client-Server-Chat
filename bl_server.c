#include "blather.h"
#include <stdbool.h>

server_t actual_server;
server_t *server = &actual_server;
int signaled = 0;
int disconnected = 0;

void handle_shutdown(int sign){
	if(sign == SIGTERM || sign == SIGINT){ 
	char *msg = "Signaled to shutdown.\n";
	write(STDERR_FILENO, msg, strlen(msg));
	server_shutdown(server);
	signaled = 1; 
	exit(0);                     
	}
}




int main(int argc, char *argv[]){

	struct sigaction shutdown = {};
 	shutdown.sa_handler = handle_shutdown;
  	sigemptyset(&shutdown.sa_mask);
  	shutdown.sa_flags = SA_RESTART;
  	sigaction(SIGTERM, &shutdown, NULL);
 	sigaction(SIGINT,  &shutdown, NULL);
 	 umask(~DEFAULT_PERMS);

	server_start(server, argv[1], DEFAULT_PERMS);

	int i;
	while(!signaled){
		// printf("llooping\n");
		server_check_sources(server);
		int k = server_join_ready(server);
		if(k){
			server_handle_join(server);
		}
		// printf("no of clients %d \n",server->n_clients);
		for( i=0; i < server->n_clients; i++){
			if(server_client_ready(server, i)){
				//printf("clientsready\n" );
				server_handle_client(server, i);		
			}	
		}
	}
	server_shutdown(server);
	exit(0);
}
