#include "libsock/client/client.hpp"
#include "libsock/types.hpp"
#include "misc/FileDescriptor.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

using namespace LibSock::Client;

Server::Server(std::string ip, uint16_t port, bool reuseaddr, bool keepalive)
	: m_sockfd(std::make_shared<LibSock::CFileDescriptor>(socket(AF_INET, SOCK_STREAM, 0)))
	, m_port(port) {
	std::lock_guard<std::mutex> lk(m_mutex);
	if (!m_sockfd->isValid() || m_sockfd->get() < 0)
		throw std::runtime_error("client:server Failed to create socket");

	constexpr int opt  = 1;
	constexpr int size = sizeof(opt);
	if (reuseaddr && setsockopt(m_sockfd->get(), SOL_SOCKET, SO_REUSEADDR, &opt, size) < 0)
		throw std::runtime_error("client:server setsockopt(SO_REUSEADDR) failed");

	if (keepalive && setsockopt(m_sockfd->get(), SOL_SOCKET, SO_KEEPALIVE, &opt, size) < 0)
		throw std::runtime_error("client:server setsockopt(SO_KEEPALIVE) failed");

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port	  = htons(m_port);
	if (inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr) <= 0)
		throw std::runtime_error("client:server inet_pton failed");
}

SP<Server> Server::make(std::string ip, uint16_t port, bool reuseaddr, bool keepalive) {
	auto c = SP<Server>(new Server(ip, port, reuseaddr, keepalive));
	vpServer.emplace_back(c);
	c->m_self = c;
	return c;
}

WP<Server> Server::get() {
	std::lock_guard<std::mutex> lk(m_mutex);
	if (m_self.expired())
		m_self = shared_from_this();
	return m_self;
}

Server::~Server() {
	m_sockfd.reset();
	vpServer.erase(std::remove_if(vpServer.begin(), vpServer.end(), [this](const SP<Server> &sptr) { return sptr.get() == this; }), vpServer.end());
}

SP<LibSock::CFileDescriptor> Server::getSocket() const {
	return m_sockfd;
}

SP<sockaddr_in> Server::getAddr() const {
	return std::make_shared<sockaddr_in>(m_addr);
}
