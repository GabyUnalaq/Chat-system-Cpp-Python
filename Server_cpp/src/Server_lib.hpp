/******************************************************************************
	Header file for creating and running the server program.

	@file Server_lib.hpp
	@author Tomuta Gabriel
	@version 1.0 01.06.2022
*/

#ifndef RC_SERVER_LIB
#define RC_SERVER_LIB

#include<iostream>
#include<string>
#include<string_view>
#include<format>
#include<vector>
#include<map>

#include<WinSock2.h>
#include<WS2tcpip.h>
#include<windows.h>
#include<csignal>

#pragma comment (lib, "ws2_32.lib")
//#define no_init_all deprecated

constexpr int BUFF_LEN = 512; // Maximum size for messages to be sent/received.
constexpr int NAME_SIZE = 10; // Maximum size for the name of the client.

using namespace std; // To make easier to use cout, endl, string, map, vector
static volatile sig_atomic_t flagStop = 0; // Flag that helps stop the program.

/******************************************************************************
	Structure that helps save every client connection.
 */
typedef struct Client {
	bool connected; // Client is connected or not
	string cl_ip;  // Client's IP
	int cl_port; // Client's PORT
	SOCKET cl_socket; // Client's Socket
};

/******************************************************************************
	Enum with codes that represent the information sent between server and
	clients as for status.
 */
enum class StatusCodes {
	ConnAccepted = 0, // Client connected successfully
	Disconnect = 1, // Client disconnected from the server
	InvalidName = 2, // Received name is taken
	MissingName = 3, // Name not received, stop connection attempt

	MsgReceived = 4, // Message successfully received
	MsgSuccess = 5, // Message successfully arrived at the destination
	MsgFailed = 6, // Message could not be sent
	InvalidDest = 7, // A client with the specified name does not exist

	ReqClients = 8 // A client requested a list with all the connected clients
};

/******************************************************************************
	A simple split function
	@param[in] str the string that is to be split
	@param[in] delim the chosen delimitator
	@param[out] parts the vector containing the split string
 */
static void split(const string& str, const string& delim, 
	              std::vector<string>& parts);

/******************************************************************************
	The main class for implementing a TCP type Server and handle all 
	client-server interactions.
 */
class ServerClass
{
public: // Functions
	/**
		Constructor of ServerClass
		@param port of which the server will crate it's connection
	*/
	ServerClass(int port);

	/**
		Deconstructor of ServerClass
	*/
	~ServerClass();

	/**
		Function that handles the startup of the server
		@return true/false
	*/
	bool start_server();

	/**
		Setter for the state of the server
		@param[in] state true/false
	*/
	void set_state(bool state);

	/**
		Getter for the state of the server
		@return true/false
	*/
	bool is_running();

	/**
		Function that gets called in a continuous loop
	*/
	void server_loop();

private:
	/**
		Method that handles the connection of a client. 
		@param[in] sock socket of the connection
		@param[in] addr address of the socket
	*/
	void connect_client(SOCKET sock, SOCKADDR_IN addr);

	/**
		Method that handles the forwarding of a message from a client to another.
		@param[in] name of the source client
		@param[in] client the related Client object
	*/
	void forward_message(string name, Client client);

	/**
		Handles the sending of messages towards clients
		@param[in] sock socket of the destination client
		@param[in] msg message to be sent
	*/
	void send_message(SOCKET sock, const char* msg);

	/**
		Handles the sending of status codes towards clients
		@param[in] sock socket of the destination client
		@param[in] code to be sent
	*/
	void send_message(SOCKET sock, StatusCodes code);

	/**
		Handles the stopping of the server
		@return true/false
	*/
	bool stop_server();

	/**
		Getter for the number of connected clients
		@return number indicating the number of connected clients
	*/
	int get_clients_num();

	/**
		Outputs to console all the connected clients with their info
	*/
	void print_clients();

	/**
		Function outputs to console the given message
		@param[in] msg the message
	*/
	void log(string msg);

	/**
		Function outputs to console the given message
		@param[in] msg the message
		@param[in] error boolean that toggles the customisation of the msg
	*/
	void log(string msg, bool error);

	/**
		Function outputs to console the given message
		@param[in] msg the message
		@param[in] error boolean that toggles the customisation of the msg
		@param[in] print boolean that toggles the console print
	*/
	void log(string msg, bool error, bool print);

private: // Functions
	/**
		Handles the adition of a new client into the clients map structure
		@param[in] name of the new client
		@param[in] client the related Client object
	*/
	void add_client(string name, Client client);

	/**
		Function gathers all the client names and returns them in a vector
		@return vector containing all the client names
	*/
	vector<string> get_client_names();

	/**
		Getter for the related Client object
		@param[in] name of the required client
	*/
	Client get_client(string name);

	/**
		Handles the deletion of a client from the map structure
		@param[in] name of the client
	*/
	void erase_client(string name);

private: // Members
	string ip; // IP of the server
	int port; // PORT of the server
	bool running = false; // boolean that indicates if the server works
	_crt_signal_t __cdecl CtrlC; // Signal that is used to handle the Ctrl+C

	int receive_res; // Number of received bytes
	SOCKET server_socket = INVALID_SOCKET; // The server socket
	sockaddr_in server; // The address of the server
	char rec_buff[BUFF_LEN] = ""; // The buffer that fills with the received data
	Client temp_client; // Client object used to handle the new connections

	map<string, Client> clients; // Structure that helps save the clients
	int clients_num = 0; // Indicates the number of connected clients
};

#endif /* RC_SERVER_LIB */