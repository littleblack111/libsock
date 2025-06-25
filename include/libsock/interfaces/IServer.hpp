#pragma once

#include "../types.hpp"
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <optional>

namespace LibSock::Abstract {
template <typename Derived>
class IServer : public std::enable_shared_from_this<Derived> {
  public:
	static SP<Derived> make(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	virtual ~IServer();
	SP<LibSock::CFileDescriptor> getSocket() const;

  protected:
	IServer(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	virtual void				 init(std::optional<sockaddr_in> addr = std::nullopt) = 0;
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	uint16_t					 m_port;

	WP<Derived> get();

	WP<Derived> m_self;

	mutable std::mutex m_mutex;
};
} // namespace LibSock::Abstract
