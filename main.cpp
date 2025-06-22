#include <iostream>
#include <libsock/server/client.hpp>
#include <libsock/server/server.hpp>
#include <print>
#include <thread>

int main(int argc, char *argv[]) {
	auto server	 = LibSock::Server::Server::create(8080);
	auto clients = LibSock::Server::Clients::create(server);
	auto client1 = clients->newClient();
	auto client2 = clients->newClient();
	auto client3 = clients->newClient();
	auto client4 = clients->newClient();
	client1.wait();
	client2.wait();
	clients->broadcast({.msg = "Hello, World!"});
	client1.get().reset();
	client2.get().reset();
	client3.get().reset();
	clients.reset();
	server.reset();

	return 0;
}
