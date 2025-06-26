import libsock.server;
import libsock.fileDescriptor;
import std;

using namespace LibSock::Client;

void Server::init(std::optional<sockaddr_in> addr) {
	if (connect(m_sockfd->get(), reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr)) < 0)
		throw std::runtime_error("client: Failed to connect to server");
}
