#include "libsock/server/server.hpp"
#include "FileDescriptor.hpp"

using namespace LibSock::Server;

void Server::init(std::optional<sockaddr_in> addr) {
	if (bind(m_sockfd->get(), reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr)) || listen(m_sockfd->get(), 5))
		throw std::runtime_error("server:server Failed to bind or listen on socket");
}
