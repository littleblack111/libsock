export module libsock.interfaces;

import libsock.types;
import std;

export namespace LibSock::Abstract {
class IServer : public std::enable_shared_from_this<IServer> {
  public:
	static SP<IServer> make(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	virtual ~IServer();
	SP<LibSock::CFileDescriptor> getSocket() const;

  protected:
	IServer(uint16_t port, bool reuseaddr = true, bool keepalive = false);
	virtual void				 init(std::optional<sockaddr_in> addr = std::nullopt) = 0;
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	uint16_t					 m_port;

	WP<IServer> get();

	WP<IServer> m_self;

	mutable std::mutex m_mutex;
};
} // namespace LibSock::Abstract
