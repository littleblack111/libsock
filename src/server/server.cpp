#include "libsock/server/server.hpp"
#include "libsock/types.hpp"
#include "misc/FileDescriptor.hpp"
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <sys/socket.h>

using namespace LibSock::Server;

Server::Server(uint16_t port, bool reuseaddr, bool keepalive)
	: m_sockfd(
		  std::make_shared<LibSock::CFileDescriptor>(socket(AF_INET, SOCK_STREAM, 0))
	  ) {
	if (pServer)
		throw std::runtime_error("server:server Server already exist");
	std::lock_guard<std::mutex>
		lk(m_mutex);
	if (!m_sockfd->isValid() || m_sockfd->get() < 0)
		throw std::runtime_error("server:server Failed to create socket");

	constexpr int opt  = 1;
	constexpr int size = sizeof(opt);
	if (keepalive &&
		setsockopt(m_sockfd->get(), SOL_SOCKET, SO_REUSEADDR, &opt, size) < 0)
		throw std::runtime_error("server:server setsockopt(SO_REUSEADDR) failed");

	if (reuseaddr &&
		setsockopt(m_sockfd->get(), SOL_SOCKET, SO_KEEPALIVE, &opt, size) < 0)
		throw std::runtime_error("server:server setsockopt(SO_KEEPALIVE) failed");

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family	   = AF_INET;
	m_addr.sin_port		   = htons(port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(m_sockfd->get(), reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr)) ||
		listen(m_sockfd->get(), 5))
		throw std::runtime_error("server:server Failed to bind or listen on socket");

	pServer = SP<Server>(this);
};

Server::~Server() { m_sockfd.reset(); }

std::shared_ptr<LibSock::CFileDescriptor> Server::getSocket() const { return m_sockfd; }
