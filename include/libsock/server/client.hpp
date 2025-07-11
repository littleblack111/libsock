#pragma once

#include "../types.hpp"
#include "clients.hpp"
#include "server.hpp"
#include <format>
#include <functional>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>

namespace sock::Server {

struct SRecvData {
	std::string	 data;
	const size_t size = 1024;
	bool		 good = false;

	bool isEmpty() const;
	void sanitize();

	static constexpr char asciiEscape = 0x1B; // 
};

enum eEventType : std::uint8_t {
	READ,
	WRITE
};

class Client {
  public:
	~Client();

	const std::string &getIp() const;
	bool			   isAdmin() const;

	UP<SRecvData> read(std::optional<std::function<bool(const SRecvData &)>> cb = std::nullopt);
	UP<SRecvData> read(const std::string &msg, std::optional<std::function<bool(const SRecvData &)>> cb = std::nullopt);
	bool		  write(const std::string &msg, std::optional<std::function<bool(const SRecvData &)>> cb = std::nullopt);
	template <typename... Args>
	bool write(std::format_string<Args...> fmt, Args &&...args);

	void resumeHistory();
	void runLoop(bool resumeHistory = true, std::optional<std::function<bool(const SRecvData &)>> cb = std::nullopt);

	bool isValid();

  private:
	void init();
	Client(SP<Server> server, SP<Clients> clients, bool track = false, bool wait = false);

	WP<Client>	m_self;
	WP<Server>	m_wpServer;
	WP<Clients> m_wpClients;

	SP<CFileDescriptor> m_sockfd;
	sockaddr_in			m_addr;
	socklen_t			m_addrLen = sizeof(m_addr);
	std::string			m_ip;
	int					m_port;
	bool				m_track;
	bool				m_wait;

	std::optional<std::string> m_szReading = std::nullopt;

	void recvLoop(std::optional<std::function<bool(const SRecvData &)>> cb = std::nullopt);

	friend class Clients;
};

} // namespace sock::Server
