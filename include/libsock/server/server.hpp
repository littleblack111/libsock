#pragma once

#include "../types.hpp"
#include <mutex>
#include <netinet/in.h>
#include <vector>

namespace LibSock {
namespace Server {

class Server {
  public:
	Server(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	~Server();
	SP<LibSock::CFileDescriptor> getSocket() const;

  private:
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	uint16_t					 m_port;

	mutable std::mutex m_mutex;
};
inline SP<Server> pServer;
} // namespace Server
} // namespace LibSock
