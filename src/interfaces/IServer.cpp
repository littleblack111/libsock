import libsock.interfaces;
import libsock.types;
import libsock.fileDescriptor;
import std;

using namespace LibSock::Abstract;

IServer::IServer(uint16_t port, bool reuseaddr, bool keepalive)
	: m_sockfd(std::make_shared<LibSock::CFileDescriptor>(socket(AF_INET, SOCK_STREAM, 0)))
	, m_port(port) {
	std::lock_guard<std::mutex> lk(m_mutex);
	if (!m_sockfd->isValid() || m_sockfd->get() < 0)
		throw std::runtime_error("server:server Failed to create socket");

	constexpr int opt  = 1;
	constexpr int size = sizeof(opt);
	if (reuseaddr && setsockopt(m_sockfd->get(), SOL_SOCKET, SO_REUSEADDR, &opt, size) < 0)
		throw std::runtime_error("server:server setsockopt(SO_REUSEADDR) failed");

	if (keepalive && setsockopt(m_sockfd->get(), SOL_SOCKET, SO_KEEPALIVE, &opt, size) < 0)
		throw std::runtime_error("server:server setsockopt(SO_KEEPALIVE) failed");

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family	   = AF_INET;
	m_addr.sin_port		   = htons(m_port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
}

SP<IServer> IServer::make(uint16_t port, bool reuseaddr, bool keepalive) {
	auto c	  = SP<IServer>(new IServer(port, reuseaddr, keepalive));
	c->m_self = c;
	return c;
}

WP<IServer> IServer::get() {
	std::lock_guard<std::mutex> lk(m_mutex);
	if (m_self.expired())
		m_self = shared_from_this();
	return m_self;
}

IServer::~IServer() {
	m_sockfd.reset();
}

SP<LibSock::CFileDescriptor> IServer::getSocket() const {
	return m_sockfd;
}
