
1 Introduction: A Chat Service

Demo Videos: From the Wed 4/21 Lecture, and a demo from a previous semester.

Systems programming is often associated with communication as this invariably requires coordinating multiple entities that are related only based on their desire to share information. This project focuses on developing a simple chat server and client called blather. The basic usage is in two parts.

Server
    Some user starts bl_server which manages the chat "room". The server is non-interactive and will likely only print debugging output as it runs.
Client
    Any user who wishes to chat runs bl_client which takes input typed on the keyboard and sends it to the server. The server broadcasts the input to all other clients who can respond by typing their own input.

Chat servers are an old idea dating to the late 1970's and if you have never used one previously, get online a bit more or read about them on Wikipedia. Even standard Unix terminals have a built-in ability to communicate with other users through commands like write, wall, and talk.

Unlike standard internet chat services, blather will be restricted to a single Unix machine and to users with permission to read related files. However, extending the programs to run via the internet will be the subject of some discussion.

Like most interesting projects, blather utilizes a combination of many different system tools. Look for the following items to arise.

    Multiple communicating processes: clients to servers
    Communication through FIFOs
    Signal handling for graceful server shutdown
    Alarm signals for periodic behavior
    Input multiplexing with poll()
    Multiple threads in the client to handle typed input versus info from the server

Finally, this project includes some advanced features which may be implemented for Makeup Credit: this credit will be added to your overall Project total to make up for mistakes in this and previous projects.
1.1 Basic Features

The specification below is mainly concerned with basic features of the blather server and client. Implementing these can garner full credit for the assignment.
1.2 ADVANCED Features : Makeup Credit

Some portions of the specification and code are marked ADVANCED to indicate optional features that can be implemented for Makeup Credit. Some of these have dependencies so take care to read carefully.

ADVANCED features may be evaluated using tests and manual inspection criteria that are made available after the standard tests are distributed.
2 Download Code and Setup

As in labs, download the code pack linked at the top of the page. Unzip this which will create a folder and create your files in that folder.
File 	State 	Notes
GROUP_MEMBERS.txt 	Edit 	Fill in names of group members to indicate partnerships
Makefile 	Create 	Build project, run tests
server_funcs.c 	Create 	Service routines for the server
bl_server.c 	Create 	Main function for bl_server executable
bl_client.c 	Create 	Main function for bl_client executable
blather.h 	Provided 	Header file which contains required structs, defines, and prototypes
util.c 	Provided 	Utility methods debug messages and checking system call returns
simpio.c 	Provided 	Simplified terminal I/O to get nice interactive sessions
simpio_demo.c 	Provided 	Demonstrates simpio features, model for bl_client.c
Testing 	TBD 	Will be released separately.
3 Demo / ScreenShot

Demonstrating an interactive, dynamic chat session between multiple users is difficult but the screenshot below attempts to do so. It shows the server and several clients mid-chat.

    The server bl_server is started in the large terminal on the left and shows output about what is going on. The server is required to show some specific output through the use of the log_printf() function as it performs certain operations and can print additional debug information using the dbg_printf() function.
    There are 4 bl_client instances run by various users who log into the server and specify their name. The lower-right user Barbara joins later while the upper left user Bruce logs out near the end of the session.

NOTE: A few of the message formats might differ in the image from the expectations described in the rest of the project specification.

blather-demo.png

Figure 1: Sample blather server and client runs.
4 Overall Architecture
4.1 What Needs to be Implemented

blather has two parts: the server and client. Both need to be written along with service routines in server.c
server_funcs.c data structure manipulations

Implement the functions in this file to manipulate the server_t and client_t data that will ultimately be used by the server to fulfill its role.
bl_server.c main function and signal handlers

Implement the server which manages the interactions between clients in this file making use of the service functions in server_funcs.c to get the job done.
bl_client.c main function and thread workers

Implement the client which allows a single user to communicate with the server in this file. The client must have multiple threads so you will need to implement some worker functions as thread entry points here.
4.2 What's Already Done

Examine the provided files closely as they give some insight into what work is already done.

    blather.h can be included in most files to make programs aware of the data structures and required functions. It contains documentation of the central data structures.
    util.c contains a few functions for logging server messages, debugging, and checking system calls.
    simpio.c and simpio_demo.c provide a small library and data structure to do terminal I/O nicely. The demo program shows how it works.

4.3 Provide a Makefile

Provide a Makefile which has at least the following targets.

    bl_server : builds the server executable, be included in the default targets.
    bl_client : builds the client executable, be included in the default targets.

Correction: An earlier version of the spec stated that testing targets are required. This has been changed as the included test_Makefile provides these.
4.4 No malloc() / free() required

It may come as a surprise, but this set of programs is specifically oriented so that no dynamic memory allocation is required. All of the data structures are fixed size, strings have a maximum length, and communication is done through FIFOs. Keep this in mind as you code and avoid malloc() unless you see no other way around it as it will likely make things harder than they need to be.
5 Basic Protocol and Architecture

Clients communicate with the server along the following lines.

    bl_server creates a FIFO at startup time. The FIFO is created with a name specified according to naming convention at startup. Examples:

      $> bl_server server1               # creates the server1.fifo
      $> bl_server batcave               # creates the batcave.fifo
      $> bl_server fortress-of-solitude  # creates the fortress-of-solitude.fifo

    This FIFO is only for join requests to the server. The name of this file must be known for the clients to connect to server.
    The server then waits for clients to write join requests to join FIFO.
    bl_client creates two FIFOs on startup:
        A to-client FIFO for the server to write data intended for the client. The client reads this FIFO.
        A to-server FIFO to which the client writes and from which the server reads.
        The names of these files are inconsequential so long as they are fairly unique. Basing names for the FIFOs on the PID of the client is an easy way to do this. This avoids multiple clients in the same directory mistakenly using each other's FIFOs.
    A client writes a join request to the server which includes the names of its to-client and to-server FIFOs. All subsequent communication between client and server is done through the to-client and to-server FIFOs.
    A client reads typed input from users. When input is ready, it is sent as a mesgt to the server.
    As a server receives messages from clients, it will broadcast the message to all clients including the sender.
    The server sends notices of other kinds to the clients such as when a new client joins. Clients may send departure notices which are re-broadcast by the server.

The sections below give graphical depictions of some of the ideas of the protocols.
5.1 Server and Clients

The following diagram illustrates the single server server1 with 2 clients connected. Each client has its own FIFOs for communication with the server and the server knows about these FIFOs.

schematic.png

Figure 2: Schematic of server data which has been joined by 2 clients.
5.2 Joining

This diagram illustrates how a new client can join the server. It creates FIFOs for communication and then writes a join_t request on the server's FIFO. The server then adds the client information to its array and broadcasts the join to all clients.

join-request.png

Figure 3: New client joining the server which has two existing clients.
5.3 Normal Messages

A client wanting to communicate with others sends a normal mesg_t to the server with its name and body filled in. The client's to-server FIFO is used for this. The server then broadcasts the same message to all clients including the sender.

broadcast.png

Figure 4: Client sending a message which is broadcast to all other clients.
5.4 Departures and Removing Clients

A client can indicate it is leaving by sending an appropriate message. The server will remove its FIFOs and shift all elements in its clients array. The departure is broadcast to all clients.

depart.png

Figure 5: Client sending a message which is broadcast to all other clients.
5.5 Protocol Data Types

The header file blather.h describes several data types associated with basic communication operations. One of these is the mesg_t struct which is used to convey information between clients and server.

// mesg_t: struct for messages between server/client
typedef struct {
  mesg_kind_t kind;               // kind of message
  char name[MAXNAME];             // name of sending client or subject of event
  char body[MAXLINE];             // body text, possibly empty depending on kind
} mesg_t;

Each message has a kind which helps determine what to do with the messages. The kinds are defined in the mesg_kind_t enumeration which sets up specific integers associated with each kind.

// mesg_kind_t: Kinds of messages between server/client
typedef enum {
  BL_MESG         = 10,         // normal messasge from client with name/body
  BL_JOINED       = 20,         // client joined the server, name only
  BL_DEPARTED     = 30,         // client leaving/left server normally, name only
  BL_SHUTDOWN     = 40,         // server to client : server is shutting down, no name/body
  BL_DISCONNECTED = 50,         // ADVANCED: client disconnected abnormally, name only
  BL_PING         = 60,         // ADVANCED: ping to ask or show liveness
} mesg_kind_t;

Messages are sent both from client to server and from server to client through the FIFOs that connect them. The specification below will describe what actions need be taken on receiving messages.

Finally, when the a client wishes to initially join the server, it fills in and sends a join_t.

// join_t: structure for requests to join the chat room
typedef struct {
  char name[MAXPATH];            // name of the client joining the server
  char to_client_fname[MAXPATH]; // name of file server writes to to send to client
  char to_server_fname[MAXPATH]; // name of file client writes to to send to server
} join_t;

6 The Server

The bl_server.c program is responsible for allowing clients to connect, become aware of one another, and communicate. It's main functionality is to broadcast messages from one client to all others and to broadcast status changes such as new clients that join or depart.
6.1 Server and Client Data

To that end, the server process should maintain a server_t data structure which is defined in blather.h as below

// server_t: data pertaining to server operations
typedef struct {
  char server_name[MAXPATH];    // name of server which dictates file names for joining and logging
  int join_fd;                  // file descriptor of join file/FIFO
  int join_ready;               // flag indicating if a join is available
  int n_clients;                // number of clients communicating with server
  client_t client[MAXCLIENTS];  // array of clients populated up to n_clients
  int time_sec;                 // ADVANCED: time in seconds since server started
  int log_fd;                   // ADVANCED: file descriptor for log
  sem_t *log_sem;               // ADVANCED: posix semaphore to control who_t section of log file
} server_t;

The most prominent features of this structure are as follows.

    The server_name which determines the name of the FIFO filename for joining and a few other things.
    A join_fd file descriptor which should be attached to a FIFO to read requests from clients to join the server.
    The client[][ array of client_t structs that track clients connected to the server. The field n_clients determines how full this array is.

Associated closely with the server_t is the client_t struct which contains data on each client.

// client_t: data on a client connected to the server
typedef struct {
  char name[MAXPATH];             // name of the client
  int to_client_fd;               // file descriptor to write to to send to client
  int to_server_fd;               // file descriptor to read from to receive from client
  char to_client_fname[MAXPATH];  // name of file (FIFO) to write into send to client
  char to_server_fname[MAXPATH];  // name of file (FIFO) to read from receive from client
  int data_ready;                 // flag indicating a mesg_t can be read from to_server_fd
  int last_contact_time;          // ADVANCED: server time at which last contact was made with client
} client_t;

While the client program bl_client may make use of this struct as well its main purpose is to help track data for the server. Prominent features are as follows.

    The user name of the client which is provided when it joins.
    Names and file descriptors of the to-client and to-server FIFOs. The file descriptors are opened by the server based on the file names the provided by the client and used for communication between them.
    A data_ready flag which is set and cleared by the server as messages are available.

6.2 Logging and Debugging Messages in the Server

The bl_server program is somewhat complex and to ease the task of understanding what it is doing at any given moment, it is required that it print certain information its current state as it works. This output will be checked in the automated tests.

Make use of the provided log_printf() function which is in util.c. This function works just like printf() in that code like

log_printf("This is a message with a '%s' and a number '%d'\n","string",42);

will print a message to the screen with format specifiers. However, it is special in that the can be silenced by setting the BL_NOLOG environment variable as demonstrated below:

> ./bl_server serv1             # log messages printed by default
LOG: BEGIN: server_start()
LOG: END: server_start()
LOG: BEGIN: server_check_sources()
LOG: poll()'ing to check 1 input sources
  C-c 
LOG: poll() completed with return value -1
LOG: poll() interrupted by a signal
LOG: END: server_check_sources()
LOG: BEGIN: server_shutdown()
LOG: END: server_shutdown()

> export BL_NOLOG=1             # disable logging
> ./bl_server serv1
  C-c
> 

No special actions need to be taken to get this effect. Keep in mind that logging the messages described is REQUIRED.

Conversely, while debugging it is often useful to print more detailed information. The should e done with the provided dbg_printf() which will only print if the BL_DEBUG environment variable is set. This variable will not be enabled during grading but is useful for debugging as shown below.

> unset BL_NOLOG                # ensure logging printed
> export BL_DEBUG=1             # also print debug messages
> ./bl_server serv1
LOG: BEGIN: server_start()
LOG: END: server_start()
DEBUG: At the top of main loop
LOG: BEGIN: server_check_sources()
LOG: poll()'ing to check 1 input sources
  C-c 
LOG: poll() completed with return value -1
LOG: poll() interrupted by a signal
LOG: END: server_check_sources()
DEBUG: Finished checking sources
DEBUG: Checking 0 clients
LOG: BEGIN: server_shutdown()
DEBUG: server_broadcast(): 40 from  - 
LOG: END: server_shutdown()

6.3 Server Operations: server_funcs.c

To facilitate operations of bl_server main program, complete the server_funcs.c file which provides service routines that mainly manipulate server_t structures. Each of these has a purpose to serve in the ultimate goal of the server.

Note that some functions have ADVANCED features listed that pertain to the Makeup credit for the project.

Also note that a number of functions have a "LOG Messages" section indicating which messages should be printed by the server and under what circumstances.

client_t *server_get_client(server_t *server, int idx);
// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.

void server_start(server_t *server, char *server_name, int perms);
// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd.
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.
// 
// LOG Messages:
// log_printf("BEGIN: server_start()\n");              // at beginning of function
// log_printf("END: server_start()\n");                // at end of function

void server_shutdown(server_t *server);
// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
// 
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.
//
// LOG Messages:
// log_printf("BEGIN: server_shutdown()\n");           // at beginning of function
// log_printf("END: server_shutdown()\n");             // at end of function

int server_add_client(server_t *server, join_t *join);
// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).
//
// LOG Messages:
// log_printf("BEGIN: server_add_client()\n");         // at beginning of function
// log_printf("END: server_add_client()\n");           // at end of function

int server_remove_client(server_t *server, int idx);
// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients. Returns 0
// on success, 1 on failure.

void server_broadcast(server_t *server, mesg_t *mesg);
// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.

void server_check_sources(server_t *server);
// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine which
// sources are ready.
// 
// NOTE: the poll() system call will return -1 if it is interrupted by
// the process receiving a signal. This is expected to initiate server
// shutdown and is handled by returning immediagely from this function.
// 
// LOG Messages:
// log_printf("BEGIN: server_check_sources()\n");             // at beginning of function
// log_printf("poll()'ing to check %d input sources\n",...);  // prior to poll() call
// log_printf("poll() completed with return value %d\n",...); // after poll() call
// log_printf("poll() interrupted by a signal\n");            // if poll interrupted by a signal
// log_printf("join_ready = %d\n",...);                       // whether join queue has data
// log_printf("client %d '%s' data_ready = %d\n",...)         // whether client has data ready
// log_printf("END: server_check_sources()\n");               // at end of function

int server_join_ready(server_t *server);
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.

void server_handle_join(server_t *server);
// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
//
// LOG Messages:
// log_printf("BEGIN: server_handle_join()\n");               // at beginnning of function
// log_printf("join request for new client '%s'\n",...);      // reports name of new client
// log_printf("END: server_handle_join()\n");                 // at end of function

int server_client_ready(server_t *server, int idx);
// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.

void server_handle_client(server_t *server, int idx);
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
//
// LOG Messages:
// log_printf("BEGIN: server_handle_client()\n");           // at beginning of function
// log_printf("client %d '%s' DEPARTED\n",                  // indicates client departed
// log_printf("client %d '%s' MESSAGE '%s'\n",              // indicates client message
// log_printf("END: server_handle_client()\n");             // at end of function 

void server_tick(server_t *server);
// ADVANCED: Increment the time for the server

void server_ping_clients(server_t *server);
// ADVANCED: Ping all clients in the server by broadcasting a ping.

void server_remove_disconnected(server_t *server, int disconnect_secs);
// ADVANCED: Check all clients to see if they have contacted the
// server recently. Any client with a last_contact_time field equal to
// or greater than the parameter disconnect_secs should be
// removed. Broadcast that the client was disconnected to remaining
// clients.  Process clients from lowest to highest and take care of
// loop indexing as clients may be removed during the loop
// necessitating index adjustments.

void server_write_who(server_t *server);
// ADVANCED: Write the current set of clients logged into the server
// to the BEGINNING the log_fd. Ensure that the write is protected by
// locking the semaphore associated with the log file. Since it may
// take some time to complete this operation (acquire semaphore then
// write) it should likely be done in its own thread to preven the
// main server operations from stalling.  For threaded I/O, consider
// using the pwrite() function to write to a specific location in an
// open file descriptor which will not alter the position of log_fd so
// that appends continue to write to the end of the file.

void server_log_message(server_t *server, mesg_t *mesg);
// ADVANCED: Write the given message to the end of log file associated
// with the server.

6.4 Messages Handled by the Server

The function server_handle_client() is a workhorse that will read a message from a client and respond take appropriate action. Here are the messages that the server should accept and respond to.
kind 	Name 	Body? 	Action
BL_MESG 	Yes 	Yes 	Broadcast message to all clients.
BL_DEPARTED 	Yes 	No 	Broadcast message to all clients.
6.5 Other Messages Sent by the Server

The server will broadcast several other kinds of messages to clients under the following circumstances.
kind 	Name 	Body? 	Circumstance
BL_JOINED 	Yes 	No 	Broadcast when a new client joins the server
BL_SHUTDOWN 	No 	No 	Broadcast when the server starts shutting down
ADVANCED 	  	  	 
BL_PING 	No 	No 	Broadcast periodically, clients should respond with a ping to indicate liveness
BL_DISCONNECTED 	Yes 	No 	Broadcast when a client has not pinged in a while to indicate disconnection.
6.6 bl_server.c main function

Define a main() entry point in bl_server that ties all of the server operations together into a function unit.
Listen, Select, Respond

The server main() function in its simplest form boils down to the following pseudocode.

REPEAT:
  check all sources
  handle a join request if one is ready
  for each client{
    if the client is ready handle data from it
  }
}

The advanced features build somewhat on this but not by much.
Signal Handling

The server should run indefinitely without interaction from an interactive user. To stop it, send signals. The server should handle SIGTERM and SIGINT by shutting down gracefully: exit the main computation, call server_shutdown() and return 0. A typical means to set this up is to control the main loop with a global variable which is changed in signal handlers to terminate the loop. After ending the loop, perform shutdown tasks.

Keep in mind that certain system calls will return immediately if the process is signaled and may need to check for this in certain functions in server_funcs.c.
7 The Client

While bl_server is a perpetual, non-interactive program, it is fairly boring in the absence of clients.. bl_client is a simple program that allows a user to type into the terminal to send text to the server and receive text back from the server which is printed to the screen. The sections below describe how to build it.
7.1 Simplified Terminal I/O

Standard terminal input and output work well in many situations but bl_client must intermingle the user typing while data may be printed to the screen. This calls for somewhat finer control. To simplify the murky area that is terminal control, the file simpio.c provides functions which have the following net effect.

    A prompt can be set that is always displayed as the last final advancing line in the terminal.
    Text can be typed and deleted by the user.
    Any thread printing with the iprintf() function will advance the prompt forwards without disrupting the text a user may be typing.

The program simpio_demo.c demonstrates this facility by spinning up a user and background thread which both print to the screen. Text printed by the background thread using iprintf() does not disrupt text being typed by the user at the prompt.

Use simpio_demo.c as a template to begin you development on bl_client.c noting the following.

    Initialize the simplified terminal I/O with the following sequence

      simpio_set_prompt(simpio, prompt);         // set the prompt
      simpio_reset(simpio);                      // initialize io
      simpio_noncanonical_terminal_mode();       // set the terminal into a compatible mode

    When threads want to print, use the iprintf() function. It works like printf() but takes as its first argument a simpio_t object which manages the prompt and typed text.
    Analyze the input loop in simpio_demo.c to see how to read characters from the prompt, detect a completed line, and end of input.

7.2 bl_client.c Features

Unlike the server, there are no required C functions for the client. Instead, there are a series of required features and suggestions on how to implement them.
Client Name and FIFO Creation

The client program is run as follows

$> bl_client server1 Lois
# join the server1 as a user Lois

$> bl_client dc-serv Bruce
# join the dc-serv server as user Bruce

The first argument is the server to join and corresponds to a FIFO name that is owned by the server.

The client should create its own to-client and to-server FIFOs which become part of the join request to the server. The names of these FIFOs can be selected arbitrarily so long as they do not conflict with other user FIFO names. Examples are to create temporary file names or use the PID of the client process to choose the FIFO names.
Joining the Server

The client should construct a join_t with its name and the names of its to-client and to-server FIFOs. This should be written to the name of the server's FIFO which will notify the server and other clients of the new user's arrival.
Messages Handled by Client

The client should handle the following kinds of messages from the server.
kind 	Name 	Body? 	Action 	Print Format / Example
BL_MESG 	Yes 	Yes 	Print 	[Bruce] : check this out
BL_JOINED 	Yes 	No 	Print 	-- Bruce JOINED --
BL_DEPARTED 	Yes 	No 	Print 	-- Clark DEPARTED --
BL_SHUTDOWN 	No 	No 	Print 	!!! server is shutting down !!!
ADVANCED 	  	  	  	 
BL_DISCONNECTED 	Yes 	No 	Print 	-- Clark DISCONNECTED --
BL_PING 	No 	No 	Reply 	Send BL_PING message back to server
User Input and Messages from the Server

To make the user experience reasonable, server messages should be received and printed to the screen as soon as they are available. Printing server messages should not disrupt the text input too much. This combination of requirements suggests the following.

    Use two threads, one for user interactions and the other for server interactions.
    The user thread performs an input loop until the user has completed a line. It then writes message data into the to-server FIFO to get it to the server and goes back to reading user input.
    The server thread reads data from the to-client FIFO and prints to the screen as data is read.
    Both user and server threads use the iprintf() function to ensure that data appears correctly on the screen and cooperates with input being typed.

A few additional notes are in order on the interaction of these two threads.

    Should the user thread receive and end of input, it should send a message to the server with kind BL_DEPARTED to indicate the client is shutting down. This will notify other users of the departure. The user thread should cancel the server thread and return.
    If the server thread receives a BL_SHUTDOWN message, the server is shutting down. This means the client should exit as well. The server thread should cancel the user thread and return.

Typed Input is Sent to the Server, not Printed

When users type input, it is echoed onto the screen by simpio at the prompt. However, on hitting enter, the data is sent immediately to the server as a mesg_t with kind MESG, name filled in with the user's name and body filled in with the text line typed.

Though the message body is known by the client which just sent it, the body is not printed to the screen. Instead, the client waits for the server to broadcast the message back and prints its own message just as if it were a message from another user. This ensures that no special actions need to be taken to avoid duplicate messages.

This "print only what the server sends" feature restriction is somewhat modified by some of the advanced features described later.
The Client has No Signal Handlers

Unlike the server, bl_client should not establish an signal handlers. It will be assumed during testing that the client behaves in the following way.

    Pressing Ctrl-d while running the client indicates the end of input and should cause the client to depart but it is NOT a signal. It should be detected with the client and trigger a normal shutdown of the client after using iprintf() to display the message:

    End of Input, Departing

    Pressing Ctrl-c while running the client will send an interrupt signal and should trigger the client to terminate immediately and ungracefully.

7.3 Client Main Approach

To summarize, bl_client will roughly take the following steps.

read name of server and name of user from command line args
create to-server and to-client FIFOs
write a join_t request to the server FIFO
start a user thread to read inpu
start a server thread to listen to the server
wait for threads to return
restore standard terminal output

user thread{
  repeat:
    read input using simpio
    when a line is ready
    create a mesg_t with the line and write it to the to-server FIFO
  until end of input
  print "End of Input, Departing"
  write a DEPARTED mesg_t into to-server
  cancel the server thread

server thread{
  repeat:
    read a mesg_t from to-client FIFO
    print appropriate response to terminal with simpio
  until a SHUTDOWN mesg_t is read
  cancel the user thread

8 Grading Criteria   grading

The following criteria will be examined during manual inspection of code by graders. Use this as a guide to avoid omitting important steps or committing bad style fouls.
Location 	Wgt 	Criteria
  	40 	AUTOMATED TESTS
  	  	20 Tests
  	  	2pts per Test Passed
  	  	MANUAL INSPECTION
Makefile 	5 	A Makefile is provided which compiles bl_server, bl_client and any ADVANCED programs implemented
  	  	Required test_Makefile is included via include test_Makefile
server_funcs.c 	  	 
server_start() 	5 	creates and opens FIFO correctly for joins, removes existing FIFO
server_shutdown() 	  	closes and removes FIFO to prevent additional joins
  	  	broadcasts a shutdown message
server_add_client() 	5 	does bounds checking to prevent overflow on add
  	  	adds client to end of array and increments n_clients
  	  	fills in fixed fields of client data based on join parameter
  	  	makes use of strncpy() to prevent buffer overruns
  	  	opens to-client and to-server FIFOs for reading/writing
  	  	 
server_remove_client() 	  	uses server_get_client() for readability
  	  	closes to-client and from-client FIFOs
  	  	correctly shifts array of clients to maintain contiguous client array and order of joining
server_check_sources() 	5 	makes use of poll() system call to detect ready clients/joins
  	  	checks the return value of poll() to determine if it completed or was signaled
  	  	checks join FIFO and all clients in poll() call
  	  	does not read data but sets read flags for join and clients
server_handle_join() 	5 	reads a join_t from join FIFO correctly
  	  	adds client with server_add_client()
  	  	broadcasts join
  	  	 
server_handle_client() 	5 	reads a mesg_t from the to-server FIFO
  	  	processes message properly and broadcasts if needed
  	  	 
Overall Style 	5 	makes use of the log_printf() function to print required log messages
  	  	makes some use of the dbg_printf() function to print debug messages
  	  	does basic error checking of system calls to check for errors, possibly using the check_fail() function
  	  	handles interrupt/term signals but avoids calling non-reentrant functions in the handlers
  	  	includes comments on complex portions of the code which require explanation
bl_server.c 	  	 
  	5 	main loop makes use of the server_funcs.c routines extensively
  	  	clear code to check input sources and handle ready input
  	  	signal handling for graceful shutdown is clear
bl_client.c 	  	 
main and overall 	10 	proper setup of terminal with simpio
  	  	use of simpio for input and output
  	  	clear creation and joining of threads
  	  	resets terminal at end of run
user thread 	5 	clear use of a user thread to handle typed input
  	  	user thread sends data to server
  	  	cancels server thread at end of input before returning
server thread 	5 	clear use of a server thread to listen data on to-client FIFO
  	  	clear handling of different kinds of messages coming from server
  	  	cancels user thread on receiving shutdown message and returns
  	60 	Manual Inspection Total
9 ADVANCED Features

The following advanced features may be implemented to garner Makeup Credit on the total project score for the semester. Each additional feature is worth the Makeup credit indicated.

Update: To be compatible with the automated tests, only enable advanced features if the environment variable BL_ADVANCED is set. Particularly the "Ping" feature may cause the server to output different log messages when pinging than normal. A standard way to do this that we have discussed is early in the main() function for bl_server, have code that looks like following.

int main(...) {
  ...
  if(getenv("BL_ADVANCED")){
    DO_ADVANCED=1;
  }
  ..

Here DO_ADVANCED is a variable that, when 0, will do normal server operations but when non-zero will enable the ADVANCED features described in this section.

Later in bl_main.c one do

  if( DO_ADVANCED ){
    ... // code an advanced feature like server pinging
  }

which will only utilized the advanced feature when enabled.
9.1 Server Ping and Disconnected Clients (10%)

Clients may terminate abruptly in the real world before they have a chance to send a BL_DEPARTED message to the server. This additional feature adds period pinging to the server and clients. A ping is a short message just to establish whether a communication partner is still present. In this case, it is a message with kind BL_PING.

Implementing this feature will cause the server broadcast a ping to all clients every second. Clients should respond immediately by sending a ping back.

To detect disconnected clients, the server should maintain its time_sec incrementing it each second. Whenever a message is received from a client, including BL_PING messages, the server updates the client's last_contact_time to the current server time. Any client that abruptly departs will eventually have a large gap between its last_contact_time and the time_sec.

Every second, the server runs the function server_remove_disconnected(server, disconnect_secs). Any client with a difference in its client->last_contact_time and server->time_sec that is larger than the argument disconnect_secs should be removed. A BL_DISCONNECTED message should be broadcast for this client to notify other clients of the change.

A good way to implement periodic activity is to use the alarm() function. This arranges for a periodic signal to be sent to the server process. This should be done in main() for bl_server.c. The signal handler for the SIGALRM that is raised can set a variable to indicate that 1 second has passed. In the main loop of the server, a condition checks this variable and performs the ping broadcast and checks for disconnected clients.

All changes for this feature are on the server side. Additional functions to write or modify are as follows.

// server_funcs.c
// Modify
int server_handle_client(server_t *server, int idx)

// Write 
void server_tick(server_t *server)
void server_ping_clients(server_t *server)
void server_remove_disconnected(server_t *server, int disconnect_secs)

// bl_server.c
int main(int argc, char *argv[])

Update: Make sure only to start the pinging / alarm() if the BL_ADVANCED environment variable is set. Alarms send signals to a process which will cause the output of the server to change if the Pinging feature is implemented. This may break compatibility with automated tests if it is always done. By starting the alarms ONLY when BL_ADVANCED is set, one can produce "normal" output in those cases get the pinging effect when the environment variable indicates it is desired.
9.2 Binary Server Log File (10%)

There is cause to want a log of all messages that pass through a server. This feature will cause the server to write such a log in a binary file.

The file should be named after the server name similarly to the join FIFO. For example

$> bl_server server1               
# creates server1.fifo
# creates server1.log

$> bl_server batcave          
# creates batcave.fifo
# creates batcave.log

The format of this binary log file is as follows.

    The first part should contain a binary who_t structure which has the active users in it. To complete the logging feature, the who_t does not need to be kept to date.
    After the who_t is a growing sequence of mesg_t structs in the log file. Any message that is broadcast by the server EXCEPT for pings is written to the end of this log, NOT in text format but as the direct binary representation of a mesg_t.
    Each new message that is broadcast is appended to the end of the log in binary format.

When the server starts up, the log file should be created and the log_fd field filled with its file descriptor. A good place to write log messages is during server_broadcast(). In it, any message that is not a ping should be written to the end of log.

When the server shuts down, close the file descriptor of the log but do not remove the file. On startup, re-open any existing log and continue appending to it.

This feature is implemented entirely in the server and requires modification of the following functions.

// server_funcs.c
void server_start(server_t *server, char *server_name, int perms)
void server_shutdown(server_t *server)
int server_broadcast(server_t *server, mesg_t *mesg)
void server_write_who(server_t *server)
void server_log_message(server_t *server, mesg_t *mesg)

// bl_server.c
int main(int argc, char *argv[])

In addition, provide a file called bl_showlog.c which will read the binary log file and print its output. This should include the who_t at the beginning and all log messages. Here is a demo run including the presumed Makefile target. Output is nearly identical to what is shown in the client.

> make bl_showlog
gcc -Wall -g -c bl_showlog.c
gcc -Wall -g -c util.c
gcc -Wall -g -o bl_showlog bl_showlog.o util.o -lpthread

> file server1.log
server1.log: data

> ./bl_showlog server1.log
3 CLIENTS
0: Clark
1: Lois
2: Barbara
MESSAGES
-- Bruce JOINED --
-- Clark JOINED --
-- Lois JOINED --
[Bruce] : hey, want to know secret identity?
[Clark] : dude, I have x-ray vision
[Lois] : wait, you have x-ray vision
[Clark] : ah, er, I mean...
[Bruce] : ha ha. I'd never do something dumb like that.
[Lois] : why not?
[Clark] : oh boy, here it comes...
[Bruce] : because...
[Lois] : ??
[Bruce] : AHYM BAAATMAN!!!
[Clark] : walked into that one
[Lois] : yup
-- Barbara JOINED --
[Barbara] : hey guys
[Clark] : hey Barbara
[Lois] : what up!
[Barbara] : Did I miss anything important?
[Clark] : the big "reveal" from Bruce...
[Barbara] : what, that he's Batman?
[Lois] : yup
[Bruce] : wait, how did you know that?
[Barbara] : you told me when we were dating
[Lois] : me too
[Bruce] : awkward. time for a SMOKE BOMB!!!
-- Bruce DEPARTED --
[Barbara] : that guy
> 

9.3 Last Messages for Client (10%)

This feature requires the Binary Server Log File Feature.

Clients that recently join a server have no idea what has transpired prior to their joining. This can be ameliorated if clients can access the binary log to examine the last N messages. For example:

$> bl_client server1 Lois
-- Lois JOINED --
[Clark] : eek!
-- Clark DEPARTED --
[Lois] : what's his deal?
[Bruce] : um..

Lois> %last 10
====================
LAST 10 MESSAGES
[Bruce] : what up man?
[Clark] : not much, just down in the fortress of solitude
[Bruce] : with lois
[Clark] : nope. lana
[Bruce] : nice
-- Lois JOINED --
[Clark] : eek!
-- Clark DEPARTED --
[Lois] : what's his deal?
[Bruce] : um..
====================
Lois>> wtf?!?

To accomplish this bl_client should examine the typed line that is entered to see if it starts with the string %last in which case it will be followed by an integer which is the number of messages requested.

On detecting this command request, client should open the binary log file for the server and seek to the end of it using lseek(). Note that the exact position in the binary log file can be calculated immediately by using.

    sizeof(mesg_t
    The number of messages requested
    The argument SEEK_END to lseek()

Thus no looping is required to find that position.

The client should then read off these messages and print them to the screen. It is a good idea to write function that will print logged messages to keep the main loop and thread loops simple in the client.

No server interaction is required for this feature so long as the server is producing the binary log file. Modify only bl_client.c.

Modify the following code for this feature.

// bl_client.c
// helper functions and maybe main()
int main(int argc, char *argv[])

9.4 who_t for Client and Server (10%)

Users who log into the server have no way of knowing who is logged in. This feature ameliorates that.

    The server writes its active users as a who_t structure to the beginning of its binary log every second at the same time as it would check for disconnected users.
    Clients can read the who_t at the beginning of the log file to determine which users are connected to the server. The client the provides a %who command which prints active users.
    To prevent the server from writing the who_t structure while a client is reading or vice versa, the server creates a semaphore which must be acquired to access this part of the log.

The who_t structure is a simple, fixed size array of strings.

// who_t: data to write into server log for current clients (ADVANCED)
typedef struct {
  int n_clients;                   // number of clients on server
  char names[MAXCLIENTS][MAXNAME]; // names of clients
} who_t;

This fixed size means every time it is written it will overwrite the same bytes. The server should write it every second in the same block using the same approach as is used to check for disconnected clients (alarm(1)). Use of pwrite() is suggested to allow the log_fd to stay in position to write to the end of the log file.

The client should implement the %who command by checking the simpio->buf for the string. It should then acquire the semaphore, read the who_t and print the results the screen similar to the following.

-- Barbara JOINED --
[Barbara] : Bruce?
[Barbara] : Bruce, are you there?
[Clark] : Ummm....
[Barbara] : hang on

Barbara>> %who
====================
3 CLIENTS
0: Batman
1: Clark
2: Barbara
====================
[Barbara] : (sigh) Batman?
[Batman] : I AM THE NIGHT!
Barbara>> 

The server is responsible for creating the semaphore which controls access to the who_t portion of the log. Follow the same naming convention as before.

$> bl_server server1               
# creates server1.fifo  FIFO
# creates server1.log   binary log file
# creates /server1.sem  POSIX semaphore

$> bl_server batcave          
# creates batcave.fifo  FIFO
# creates batcave.log   binary log file
# creates /batcave.sem  POSIX semaphore

Recall the following about POSIX semaphores which are relevant to their use here.

    They are created / accessed with the sem_open() function which takes a name which must start with a forward slash, thus the /server1.sem convention.
    POSIX semaphores use sem_wait() (decrement) and sem_post() (increment) operations to lock/unlock the semaphore.
    When the server wants to write a who_t into the log, it acquires the semaphore with sem_wait(), writes, then releases with sem_post().
    Clients follow a similar protocol when reading the who_t.
    When shutting down the, server should unlink the semaphore with sem_unlink().

Modify the following code for this feature.

// server_funcs.c
void server_start(server_t *server, char *server_name, int perms)
void server_shutdown(server_t *server)
void server_write_who(server_t *server)

// bl_server.c
int main(int argc, char *argv[])


// bl_client.c
// helper functions and maybe main()
int main(int argc, char *argv[])


