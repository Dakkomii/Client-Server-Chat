#include "blather.h"
#include <errno.h>




client_t *server_get_client(server_t *server, int idx){
// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.
	client_t *client = &(server->client[idx]);

		return client;
}

void server_start(server_t *server, char *server_name, int perms){
// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd._
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.

	strcpy(server->server_name, server_name);
	char name[MAXNAME];
	//char log_name[MAXNAME];
	snprintf(name, MAXPATH, "%s.fifo", server_name);
	remove(name);
    mkfifo(name, perms);                          // create the fifo if needed
    server->join_fd = open(name, O_RDWR);
	server->join_ready = 0;
  	server->n_clients=0;
}


void server_shutdown(server_t *server){
// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
//
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.

	mesg_t msg2 ={ .kind = BL_SHUTDOWN};
	server_broadcast(server, &msg2);
	int i;
	for(i = server->n_clients; i>= 0; i--){
		server_remove_client(server, i);
	}
	char sem_name[MAXNAME];
	strncpy(sem_name, server->server_name, MAXNAME);
	int c_join = close(server->join_fd);
	int rem_server = remove(strcat(server->server_name,".fifo"));
	//int c_log = close(server->log_fd);
	int c_sem = sem_close(server->log_sem);
	int unl_sem = sem_unlink(strcat(sem_name,".sem"));
	
	int list[4]={c_join, rem_server, c_sem, unl_sem, /*c_log,*/};
	for(int i=0; i < 2; i++){
		if(list[i]== -1){
			perror("Error in server shutdown");
			abort();
			}
		}
	
return;

}




int server_add_client(server_t *server, join_t *join){
// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).

	if(server->n_clients == MAXCLIENTS){
    return 1;
	  } 
	else {
		server->n_clients++; 
		client_t *client =server_get_client(server,server->n_clients-1);
		// printf("%s %s %s\n",join->name,join->to_client_fname,join->to_server_fname);					
		client->to_client_fd = open(join->to_client_fname, O_RDWR);
		client->to_server_fd = open(join->to_server_fname, O_RDONLY);
		strncpy(client->to_client_fname, join->to_client_fname, MAXPATH);		
		strncpy(client->to_server_fname, join->to_server_fname, MAXPATH);

		strncpy(client->name, join->name, MAXNAME);		

		printf("%s %s %s\n",client->name, client->to_client_fname,client->to_server_fname);					

		client->data_ready = 0;
		client->last_contact_time = server->time_sec;

      	return 0;
	  }
}





int server_remove_client(server_t *server, int idx){
// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients.


   client_t *cnt = server_get_client(server, idx);

   //close(cnt->to_client_fd);
   //close(cnt->to_server_fd);
    unlink(cnt->to_server_fname);
    unlink(cnt->to_client_fname);

   int i;
	if(idx > server->n_clients){
		server->n_clients--;
	}
	else{
    	for(i = idx; i< server->n_clients-1; i++){
        	server->client[i] = server->client[i+1];
   		}
	}
    server->n_clients--;

    
    



    /*if(idx == server->n_clients){
        server->n_clients--;
        //memset(cnt, 0, sizeof(client_t));  // my attempts to fix stuff
        //server->client[idx] = 0;
        }
    else if(idx < server->n_clients){
        for(int i = idx; i < server->n_clients; i++){
            server->client[i] = server->client[i+1];
        }
       client_t *lst = server_get_client(server, server->n_clients);
//        memset(lst, NULL, sizeof(client_t));    // my attempts to fix stuff
        server->n_clients--;
    }
    else if(idx > server->n_clients){
        printf("Can't remove client %d; outside of bounds.\n", idx);
    } */
//    memset(server->client[idx], NULL, sizeof(client_t));



	return 0;

}


int server_broadcast(server_t *server, mesg_t *mesg){
// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.

	int i;
		if(mesg->kind == BL_PING){
		for(i = 0; i< server->n_clients; i++){  
			write(server->client[i].to_client_fd, mesg, sizeof(mesg_t));
		}
	}
	else{
		printf("Broadcasting message %s \n", mesg->body);
		for(i = 0; i< server->n_clients; i++){  
			write(server->client[i].to_client_fd, mesg, sizeof(mesg_t));
		}
	}
	return 0;
}


void server_check_sources(server_t *server){
// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine
// which sources are ready.
	


	int n = server->n_clients+1;
	struct pollfd pfds[n];

	for (int i = 0; i<server->n_clients; i++) {
		client_t *c = server_get_client(server, i);
		c->data_ready= 0;
		pfds[i].fd = c->to_server_fd;
		pfds[i].events = POLLIN;
	}

	server->join_ready = 0;
	pfds[n-1].fd = server->join_fd;
	pfds[n-1].events = POLLIN;
	//int debug = 
	poll(pfds, n, -1);

	server->join_ready = pfds[n-1].revents & POLLIN; //Added by TA Chase for debugging

	for(int j = 0; j<server->n_clients; j++) {
		client_t *i_client = server_get_client(server, j);
		i_client->data_ready = pfds[j].revents & POLLIN;
	}
	if( server_get_client(server, 0)->data_ready)
	printf("client data ready: %d \n", server->client[0].data_ready);

/*
	int i;
    struct pollfd pfds[(server->n_clients) + 1];
    pfds[0].fd       =server->join_fd;
    pfds[0].events   = POLLIN;
    for( i = 1; i < server->n_clients; i++){
        pfds[i].fd       =server->client[i].to_server_fd;
        pfds[i].events   = POLLIN;
    }


    int ret = poll(pfds, (server->n_clients)+1 , -1);
    if (ret < 0){
        perror("poll() failed");
    }
    if(pfds[0].revents & POLLIN){
            server->join_ready = 1;
            return;
        }
    for(int i = 1; i < server->n_clients; i++){
        if(pfds[i].revents & POLLIN){
            server->client[i].data_ready = 1;
        }

	}
*/	


}
int server_join_ready(server_t *server){
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.
	return server->join_ready;
}

int server_handle_join(server_t *server){
// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.


	int server_join = server_join_ready(server);
	if(server_join){
		join_t join;
		read(server->join_fd, &join, sizeof(join_t));

		// strncpy(server->client[server->n_clients].name, join.name, MAXNAME);
		// server->client[server->n_clients].to_client_fd = open(join.to_client_fname, O_RDWR);
		// server->client[server->n_clients].to_server_fd = open(join.to_server_fname, O_RDWR);

		// server->client[server->n_clients].data_ready = 0;

		//server->client[server->n_clients].last_contact_time = server->time_sec;
		server_add_client(server,&join);
		mesg_t mesg = {
    	.kind = BL_JOINED,
    	.name = "",
   		 .body = "",
 		 };
 		 strncpy(mesg.name, join.name, MAXNAME);
 		 server_broadcast(server,&mesg);
		// mesg_t msg;
		// mesg_t *join_message = &msg;
		// memset(join_message, 0, sizeof(mesg_t));
		// strncpy(join_message->name, join.name, MAXNAME);
		// join_message->kind = BL_JOINED;
		// // server->n_clients++;
		// server_broadcast(server, join_message);
 		server->join_ready = 0;
		return 0;
	}
	else
	{
		abort();
	}

}




int server_client_ready(server_t *server, int idx){
// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.

	//int ind = server_get_client(server, idx);
	//client_t client = server->client[ind];
	client_t client = server->client[idx];
	int data = client.data_ready;
	return data;

}



int server_handle_client(server_t *server, int idx){
// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. Clear the client's
// data_ready flag so it has value 0.
//
// ADVANCED: Update the last_contact_time of the client to the current
// server time_sec.
  
    	//printf("server_handle_client  \n");

	mesg_t msg;
	mesg_t *msg2 = &msg;

    	read(server->client[idx].to_server_fd, msg2, sizeof(mesg_t));
    	printf("server_handle_client message %s\n", msg2->body);

    	strncpy(msg2->name, server->client[idx].name, MAXNAME);
    	server->client[idx].data_ready = 0;

		if(msg2->kind == BL_DEPARTED){
			server_remove_client(server, idx);
			//server_broadcast(server, msg2);
		}
		else if(msg2->kind == BL_PING){
    			server->client[idx].last_contact_time = server->time_sec;
			server->client[idx].data_ready = 0;
		}
		
			//strncpy(msg2->name, server->client[idx].name, MAXNAME);
			//server_broadcast(server, msg2);
			//server->client[idx].data_ready = 0;
		
		
		server_broadcast(server, msg2);
  		
	
	return 0;
}





void server_tick(server_t *server){
// ADVANCED: Increment the time for the server
	server->time_sec++;
}


/*
void server_ping_clients(server_t *server){
// ADVANCED: Ping all clients in the server by broadcasting a ping.

	int i;
	//printf("ping 0\n");
	for( i = 0; i < server->n_clients; i++){
		printf("%s: ping 1\n", server->client[i].name);
		mesg_t msg;
		mesg_t *msg2 = &msg;
		memset(msg2, 0, sizeof(mesg_t));
		msg2->kind = BL_PING;
		strncpy(msg2->name, server->client[i].name, MAXNAME);
		printf("%s: ping 2\n", server->client[i].name);
		//if(!write(server->client[i].to_client_fd, msg2, sizeof(mesg_t))){
		//	perror("Error");
		//	abort();
		//}
		printf("%s: ping 3\n", server->client[i].name);
	}
	
	return;
}
*/
//void server_remove_disconnected(server_t *server, int disconnect_secs){
// ADVANCED: Check all clients to see if they have contacted the
// server recently. Any client with a last_contact_time field equal to
// or greater than the parameter disconnect_secs should be
// removed. Broadcast that the client was disconnected to remaining
// clients.  Process clients from lowest to highest and take care of
// loop indexing as clients may be removed during the loop
// necessitating index adjustments.
/*

	
	int i;
	for(i = 0; i < server->n_clients; i++){
		int time_elapsed = server->time_sec - server->client[i].last_contact_time;
  		if(time_elapsed >= disconnect_secs){
			printf("Removing client %s for inactivity.\n", server->client[i].name);
			mesg_t discon_msg;
			mesg_t *disconnect = &discon_msg;
			strcpy(disconnect->name, server->client[i].name);
			disconnect->kind = BL_DISCONNECTED;
			server_broadcast(server, disconnect);
			server_remove_client(server, i);
  			}
		}

	printf("Number of clients now: %d\n", server->n_clients);
}
*/

/*
void server_write_who(server_t *server){
// ADVANCED: Write the current set of clients logged into the server
// to the BEGINNING the log_fd. Ensure that the write is protected by
// locking the semaphore associated with the log file. Since it may
// take some time to complete this operation (acquire semaphore then
// write) it should likely be done in its own thread to preven the
// main server operations from stalling.  For threaded I/O, consider
// using the pwrite() function to write to a specific location in an
// open file descriptor which will not alter the position of log_fd so
// that appends continue to write to the end of the file.

	
}

void server_log_message(server_t *server, mesg_t *mesg){
// ADVANCED: Write the given message to the end of log file associated
// with the server.



}
*/
