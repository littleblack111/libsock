#pragma once

#include "../types.hpp"
#include "server.hpp"
#include <any>
#include <format>
#include <functional>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>

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

class Clients;
inline SP<Clients> pClients;

class Client {
  public:
	Client(SP<Server> server = SP<Server>(pServer), SP<Clients> clients = pClients, bool track = false, bool oneShot = true);
	~Client();

	const std::string &getName() const;
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
	WP<Client>					 self;
	SP<LibSock::CFileDescriptor> m_sockfd;
	sockaddr_in					 m_addr;
	socklen_t					 m_addrLen = sizeof(m_addr);
	std::string					 m_name;
	std::string					 m_ip;
	int							 m_port;
	bool						 m_track;
	bool						 m_oneShot;

	std::optional<std::string> m_szReading = std::nullopt;

	void recvLoop();

	friend class Clients;
};

struct SData {
	std::string				  msg;
	std::optional<WP<Client>> sender = std::nullopt;
	bool					  admin	 = false;
};

class Clients {
  public:
	Clients();
	~Clients();

	WP<Client> newClient(std::optional<std::function<std::any(WP<Client>)>> cb = std::nullopt);

	void broadcast(const std::string &msg, std::optional<WP<Client>> self = std::nullopt); // second param only specified when we want to exclude the sender
	void kick(WP<Client> client, const bool kill = false, const std::string &reason = "");
	void addClient(const Client &client);

	bool nameExists(const std::string &name);
	void shutdownClients();

	SP<Client>				getByName(const std::string &name) const;
	SP<Client>				getByIp(const std::string &ip) const;
	std::vector<SP<Client>> getClients() const;
	std::vector<SData>		getDatas() const;

  private:
	void broadcast(const SData &msg);

	std::vector<std::pair<std::jthread, SP<Client>>> m_vClients;
	std::vector<SData>								 m_vDatas;
};
} // namespace Server
} // namespace LibSock
