#pragma once
#include "misc/FileDescriptor.hpp"
#include "misc/memory.hpp"
#include <mutex>
#include <netinet/in.h>

namespace LibSock {
namespace Server {

class Server {
  public:
	Server(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	~Server();
	SP<CFileDescriptor> getSocket() const;

  private:
	SP<CFileDescriptor> m_sockfd;
	sockaddr_in			m_addr;
	uint16_t			m_port;

	mutable std::mutex m_mutex;
};
inline UP<Server> pServer;
} // namespace Server
} // namespace LibSock
