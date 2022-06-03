/******************************************************************************
	Source file for creating and running the server program.

	@file Server_lib.cpp
	@author Tomuta Gabriel
	@version 1.0 01.06.2022
*/

#include "Server_lib.hpp"

/******************************************************************************
   Contructors and Deconstructors
 */

ServerClass::ServerClass(int port) {
	ServerClass::port = port;

	// Configure CTRL+C lambda function
	CtrlC = signal(SIGINT, [](int) {flagStop = 1; });
}
ServerClass::~ServerClass() {}

/******************************************************************************
   Server purpose
 */

bool ServerClass::start_server() {
	int wsaresult, i = 1;
	WSADATA wsaData;

	server.sin_family = AF_INET;  // IPV4
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	// Initialize Winsock
	wsaresult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaresult != 0) {
		//printf("WSAStartup failed with error: %d\n", wsaresult);
		log("WSAStartup failed.", true);
		return false;
	}

	// Create a SOCKET for connecting to server
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket == INVALID_SOCKET) {
		//printf("Socket failed with error: %ld\n", WSAGetLastError());
		log("Socket failed.", true);
		WSACleanup();
		return false;
	}
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof(i));

	// Binding part
	wsaresult = bind(server_socket, (sockaddr*)&server, sizeof(server));
	if (wsaresult == SOCKET_ERROR) {
		//printf("Bind failed with error: %d\n", WSAGetLastError());
		log("Bind failed.", true);
		closesocket(server_socket);
		WSACleanup();
		return false;
	}

	// Get server information
	sockaddr_in sin;
	memcpy(&sin, &server, sizeof(sin));
	ip = inet_ntoa(sin.sin_addr); // TODO returns 0.0.0.0
	int aux_port = htons(sin.sin_port);
	
	if (aux_port != port)
		log("Server oppened on a different port than the one specified.", true);

	// Setup the TCP listening socket
	wsaresult = listen(server_socket, SOMAXCONN);
	unsigned long b = 1;

	// Make it non blocking
	ioctlsocket(server_socket, FIONBIO, &b);
	if (wsaresult == SOCKET_ERROR) {
		printf("Listen failed with error: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return false;
	}
	
	if (ip.compare(string("0.0.0.0")))
		log(" # Server started succesfully on " + ip + ":" + to_string(port) + ".");
	else
		log(" # Server started succesfully.");
	return true;
}

void ServerClass::set_state(bool state) {
	running = state;
}

bool ServerClass::is_running() {
	return running;
}

void ServerClass::server_loop() {
	// Check if server is running
	if (!is_running())
		return;

	// Check if stop flag was activated
	if (flagStop == 1) {
		stop_server();
	}

	// Accept connection
	SOCKADDR_IN cl_addr;
	int len = sizeof(cl_addr);
	SOCKET cl_socket = accept(server_socket, (SOCKADDR*)&cl_addr, &len);

	// Check if the client is valid
	if (cl_socket != INVALID_SOCKET) {
		log(" - Connecting client..");
		connect_client(cl_socket, cl_addr);
	}
	
	// Sleep a little
	Sleep(1);

	// Send and receive data
	if (clients_num > 0) {
		for (auto const& client : clients) {
			memset(&rec_buff, 0, sizeof(rec_buff));
			if (client.second.connected) {
				//receive data
				receive_res = recv(client.second.cl_socket, rec_buff, BUFF_LEN, 0);

				if (receive_res > 0) {
					Sleep(10);
					if (isdigit(rec_buff[0])) { // Status code received
						int code = atoi(rec_buff);
						if (static_cast<int>(StatusCodes::Disconnect) == code) {
							log(" # Client " + client.first + " disconnected.");
							erase_client(client.first);
						}
						else {
							log(" - Status code " + string(rec_buff) + " received from " + client.first + ".");
						}
					}
					else { // Message received that has to be forwarded
						forward_message(client.first, client.second);
					}
				}
			}
		}
	}
	return;
}

void ServerClass::connect_client(SOCKET sock, SOCKADDR_IN addr) {
	// Use the temp client
	temp_client.cl_socket = sock;
	temp_client.cl_ip = inet_ntoa(addr.sin_addr);
	temp_client.cl_port = htons(addr.sin_port);
	temp_client.connected = true;

	// Receive name of the client
	char name_char[NAME_SIZE] = {0};
	int tryes = 200;
	do {
		receive_res = recv(temp_client.cl_socket, name_char, NAME_SIZE, 0);
		Sleep(10);
		tryes--;
	} while (receive_res == 0 && tryes != 0);
	if (receive_res > 0) {
		Sleep(10);
		string name_string(name_char);
		bool valid_name = false;

		if (clients.count(name_string)) { // name exists
			if (!clients[name_string].connected) { // Exists and it's not connected
				valid_name = true;
			}
			else { // Exists and it's connected
				log("Name already exists. ", true);
			}
		}
		else { // Name does not exist yet
			valid_name = true;
		}

		if (valid_name) {
			send_message(temp_client.cl_socket, StatusCodes::ConnAccepted);

			add_client(name_string, temp_client);

			log(" # Client connected with name " + name_string + " on " +
				string(temp_client.cl_ip) + ":" + to_string(temp_client.cl_port) + ".");
		}
		else { // Invalid name
			send_message(temp_client.cl_socket, StatusCodes::InvalidName);
		}
	}
	else { // Name not received
		send_message(temp_client.cl_socket, StatusCodes::MissingName);
		log("Name not received. ", true);
	}
}

void ServerClass::forward_message(string name, Client client) {
	log(" - Client data received from " + string(name) + " with message: " + string(rec_buff));

	// Message format: <Src>|<Dest>|<Msg>
	vector<string> parts;
	split(string(rec_buff), string("|"), parts);
	if (!clients.count(parts[0])) { // If src does not exist?
		log("WTF Src not existing?", true);
		return;
	}
	if (!clients.count(parts[1])) { // If dest does not exist
		log("Destination does not exist", true);
		send_message(client.cl_socket, StatusCodes::InvalidDest);
		return;
	}

	// Message <Src>|<Msg>
	SOCKET dest_socket = get_client(parts[1]).cl_socket;
	string new_msg = parts[0] + string("|") + parts[2];

	// Forward message
	send_message(dest_socket, new_msg.c_str());

	// Wait confirmation
	char confirmation[BUFF_LEN] = { 0 };
	int conf_res = 0;
	//DWORD timeout = 0x5000 * 1000;
	//setsockopt(dest_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	do {
		conf_res = recv(dest_socket, confirmation, BUFF_LEN, 0);
	} while (conf_res < 0);
	if (conf_res != SOCKET_ERROR) {
		if (conf_res > 0) { // Confirmation received
			Sleep(10);

			if (atoi(confirmation) == static_cast<int>(StatusCodes::MsgReceived)) {
				log(" - Message forwarded succesfully.");
				send_message(client.cl_socket, StatusCodes::MsgSuccess);
			}
			else {
				log(" - Received status code: " + string(confirmation));
				send_message(client.cl_socket, StatusCodes::MsgFailed);
			}
		}
		else { // Confirmation not received
			log("Message could not be forwarded.", true);
			send_message(client.cl_socket, StatusCodes::MsgFailed);
		}
	}
	else { // Error in receiving data
		log("Confirmation failed.", true);
		send_message(client.cl_socket, StatusCodes::MsgFailed);
	}
}

void ServerClass::send_message(SOCKET sock, const char* msg) {
	send(sock, msg, strlen(msg), 0);
}

void ServerClass::send_message(SOCKET sock, StatusCodes code) {
	char msg[5] = {0};
	sprintf(msg, "%d", code);
	send(sock, msg, strlen(msg), 0);
}

bool ServerClass::stop_server() {
	running = false;
	log(" # Server stopped.");
	return true;
}

/******************************************************************************
   General purpose
 */

int ServerClass::get_clients_num(){
	return clients_num;
}

void ServerClass::add_client(string name, Client client) {
	clients[name] = client;
	clients_num++;
}

Client ServerClass::get_client(string name) {
	return clients[name];
}

void ServerClass::erase_client(string name) {
	clients[name].connected = false;
	//clients.erase(name);
	clients_num--;
}

vector<string> ServerClass::get_client_names() {
	vector<string> names;
	for (auto const& elem : clients)
		names.push_back(elem.first);
	return names;
}

void ServerClass::print_clients() {
	cout << "Clients: \n";

	for (const string& name : get_client_names())
		cout << name << " ";
	cout << endl;
}

void ServerClass::log(string msg) {
	log(msg, false, true);
}

void ServerClass::log(string msg, bool error) {
	log(msg, error, true);
}

void ServerClass::log(string msg, bool error, bool print) {
	if (print) {
		if (error)
			cout << " * Error: " << msg << endl;
		else
			cout << msg << endl;
	}
	// Save msg to logs?
}

/******************************************************************************
   Utils
 */

static void split(const string& str, const string& delim, std::vector<string>& parts) {
	size_t start, end = 0;
	while (end < str.size()) {
		start = end;
		while (start < str.size() && (delim.find(str[start]) != string::npos)) {
			start++;  // skip initial whitespace
		}
		end = start;
		while (end < str.size() && (delim.find(str[end]) == string::npos)) {
			end++; // skip to end of word
		}
		if (end - start != 0) {  // just ignore zero-length strings.
			parts.push_back(string(str, start, end - start));
		}
	}
}
