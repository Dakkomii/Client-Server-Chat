#include "blather.h"
#include <time.h>
#include <dirent.h>

simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;
client_t client_actual;
client_t *client = &client_actual;
server_t server_actual;
server_t *server = &server_actual;
pthread_t user_thread;
pthread_t server_thread;


void *server_worker(){
	int s = 0;
	while(s == 0){
		mesg_t msg;
		mesg_t *msg2 = &msg;
		read(client->to_client_fd, msg2, sizeof(mesg_t));
/*
		if(msg2->kind == BL_JOINED){
			printf("%s", msg2->name);
				iprintf(simpio, "-- %s JOINED --\n", msg2->name);
		}
		else if(msg2->kind == BL_DEPARTED){
			iprintf(simpio, "-- %s DEPARTED --\n", msg2->name);
		}
		else if(msg2->kind == BL_PING){
			write(client->to_server_fd, msg2, sizeof(mesg_t));
				iprintf(simpio,"%s pinged back\n", msg2->name);
		}
		else if(msg2->kind == BL_MESG){
			iprintf(simpio, "[%s] : %s\n", msg2->name, msg2->body);
		}
		else if(msg2->kind == BL_SHUTDOWN){
			s = 1;
		}
*/			
		switch(msg2->kind){
			case BL_JOINED:
				//printf("%s", msg2->name);
				iprintf(simpio, "-- %s JOINED --\n", msg2->name);
				break;
			case BL_DEPARTED:
				iprintf(simpio, "-- %s DEPARTED --\n", msg2->name);
				break;
			case BL_MESG:
				iprintf(simpio, "[%s] : %s\n", msg2->name, msg2->body);
				break;
			case BL_PING:
				write(client->to_server_fd, msg2, sizeof(mesg_t));
				iprintf(simpio,"%s pinged back\n", msg2->name);
				break;
			case BL_SHUTDOWN:
				s = 1;
			default:
				break;
				}
	}
	iprintf(simpio, "!!! server is shutting down !!!\n");
	pthread_cancel(user_thread);
	return NULL;
}


void *user_worker(){
	while(!simpio->end_of_input){
		simpio_reset(simpio);
		iprintf(simpio,"");
		while(!simpio->line_ready && !simpio->end_of_input){
			simpio_get_char(simpio);
		}
		if(simpio->end_of_input){
			mesg_t msg;
			mesg_t *msg2 = &msg;
			memset(msg2, 0, sizeof(mesg_t));
			msg2->kind = BL_DEPARTED;
			strncpy(msg2->name, client->name, MAXNAME);
			write(client->to_server_fd, msg2, sizeof(mesg_t));
		}
		if(simpio->line_ready){
			mesg_t message;
			mesg_t *msg2 = &message;
			memset(msg2, 0, sizeof(mesg_t));
			msg2->kind = BL_MESG;
			strncpy(msg2->name, client->name, MAXNAME);
			strncpy(msg2->body, simpio->buf, MAXLINE);
			//iprintf(simpio, "%s message body \n",msg2->body);
			write(client->to_server_fd, msg2, sizeof(mesg_t));
		}
	}
	pthread_cancel(server_thread);
	return NULL;
}

int main(int argc, char **argv){
	

	char servr[MAXNAME];
	char user[MAXNAME];

	strncpy(server->server_name, argv[1], MAXNAME);
	strncpy(servr, server->server_name, MAXNAME);
	strcat(servr,".fifo");
	
	strncpy(user, argv[2], MAXNAME);
	strncpy(client->name, user, MAXNAME);
	

	join_t join;
	memset(&join,0,sizeof(join_t));
	
	snprintf(join.name, MAXNAME, "%s", argv[2]);

	char client_fname[MAXPATH];
	char server_fname[MAXPATH];

	snprintf(client_fname,sizeof(long),"%ld",(long)getpid());
	snprintf(server_fname,sizeof(long),"%ld",(long)getpid());

	strncpy(join.to_client_fname, strcat(client_fname,".client.fifo"),MAXPATH);
	strncpy(join.to_server_fname, strcat(server_fname,".server.fifo"),MAXPATH);

	//snprintf(join.to_client_fname, MAXPATH, "%d.client.fifo", getpid());	//added
	//snprintf(join.to_client_fname, MAXPATH, "%d.server.fifo", getpid());	//added


	mkfifo(join.to_client_fname, S_IWUSR | S_IRUSR);
	mkfifo(join.to_server_fname, S_IWUSR | S_IRUSR);

	client->to_client_fd = open(join.to_client_fname, O_RDWR);
	client->to_server_fd = open(join.to_server_fname, O_RDWR);
	server->join_fd = open(servr, O_RDWR);
	write(server->join_fd, &join, sizeof(join_t));

	
	char prompt[MAXNAME];
	snprintf(prompt, MAXNAME, "%s>>", user); 
	simpio_set_prompt(simpio, prompt);
	simpio_reset(simpio);
	simpio_noncanonical_terminal_mode();

	pthread_create(&user_thread, NULL, user_worker, NULL); 
	pthread_create(&server_thread, NULL, server_worker, NULL); 
	pthread_join(user_thread, NULL);
	pthread_join(server_thread, NULL);

	simpio_reset_terminal_mode();
	printf("\n");
	return 0;
}
