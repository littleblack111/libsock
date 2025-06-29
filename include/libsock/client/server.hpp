#pragma once

#include "../types.hpp"
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <vector>

namespace LibSock::Client {

class Server : public std::enable_shared_from_this<Server> {
  public:
	static SP<Server> make(std::string ip, uint16_t port, bool reuseaddr = true, bool keepalive = false);
	~Server();
	SP<LibSock::CFileDescriptor> getSocket() const;
	SP<sockaddr_in>				 getAddr() const;

  private:
	Server(std::string ip, uint16_t port, bool reuseaddr = true, bool keepalive = false);
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	uint16_t					 m_port;

	WP<Server> get();

	WP<Server> m_self;

	mutable std::mutex m_mutex;
};
inline std::vector<SP<Server>> vpServer;
} // namespace LibSock::Client
