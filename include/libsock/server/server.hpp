#pragma once

#include "../types.hpp"
#include <memory>
#include <mutex>
#include <netinet/in.h>

namespace LibSock {
namespace Server {

class Server : public std::enable_shared_from_this<Server> {
  public:
	static SP<Server> create(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	~Server();
	SP<LibSock::CFileDescriptor> getSocket() const;

  private:
	Server(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	uint16_t					 m_port;

	SP<Server> get();

	mutable std::mutex m_mutex;
};
inline SP<Server> pServer;
} // namespace Server
} // namespace LibSock
