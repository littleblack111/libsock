#pragma once

#include "../types.hpp"
#include "clients.hpp"
#include "server.hpp"
#include <any>
#include <format>
#include <functional>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>

namespace LibSock {
namespace Server {

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

	UP<SRecvData> read(std::optional<std::function<std::any(const SRecvData &)>> cb = std::nullopt);
	UP<SRecvData> read(const std::string &msg, std::optional<std::function<std::any(const SRecvData &)>> cb = std::nullopt);
	bool		  write(const std::string &msg, std::optional<std::function<std::any(const SRecvData &)>> cb = std::nullopt);
	template <typename... Args>
	bool write(std::format_string<Args...> fmt, Args &&...args);

	void run();

	bool isValid();

  private:
	Client(SP<Server> server = pServer, SP<Clients> clients = pClients, bool track = false, bool oneShot = true);

	WP<Client>					 self;
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	socklen_t					 m_addrLen = sizeof(m_addr);
	std::string					 m_ip;
	int							 m_port;
	bool						 m_track;
	bool						 m_oneShot;

	std::optional<std::string> m_szReading = std::nullopt;

	void recvLoop();

	friend class Clients;
};

} // namespace Server
} // namespace LibSock
