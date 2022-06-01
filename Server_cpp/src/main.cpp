/******************************************************************************
	Main script that uses the Server library.

	@file main.cpp
	@author Tomuta Gabriel
	@version 1.0 01.06.2022
*/

#include "Server_lib.hpp"

int main(int argc, char *argv[]) {
	// Read arguments
	// Possible configurations:
	// _.exe -port <PORT>

	int PORT = 12345;
	int i = 1;
	while (i < argc) {
		string arg(argv[i]);
		if (arg.compare("-port") == 0) {
			PORT = atoi(argv[i + 1]);
			i++;
		}

		i++;
	}

	// Create server instance
	ServerClass server(PORT); // Main server object

	// Start server
	if (server.start_server())
		server.set_state(true);

	// Loop server
	while (server.is_running()) {
		server.server_loop();
	}
}
