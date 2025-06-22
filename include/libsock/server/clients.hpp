#pragma once

#include "../types.hpp"
#include "server.hpp"
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace LibSock {
namespace Server {

class Client;

struct SData {
	std::string				  msg;
	std::optional<WP<Client>> sender = std::nullopt;
};

class Clients : public std::enable_shared_from_this<Clients> {
  public:
	static SP<Clients> create(WP<Server> server);
	~Clients();

	SP<Client> newClient(std::function<void(SP<Client> &)> cb = [](SP<Client>) {}, bool track = true, bool wait = false);

	void broadcast(const SData &msg); // second param only specified when we
									  // want to exclude the sender
	void kick(WP<Client> client, const bool kill = false, const std::string &reason = "", std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>>::iterator &)>> cb = std::nullopt);
	void addClient(const Client &client);

	void shutdownClients(std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>> &)>> cb = std::nullopt);

	SP<Client>				getByIp(const std::string &ip) const;
	std::vector<SP<Client>> getClients() const;
	std::vector<SData>		getDatas() const;

  private:
	Clients();

	std::vector<std::pair<std::jthread, SP<Client>>> m_vClients;
	std::vector<SData>								 m_vDatas;

	WP<Clients> get();

	WP<Clients> m_self;
	WP<Server>	m_wpServer;

	friend class Client;
};
inline std::vector<WP<Clients>> vpClients;
} // namespace Server
} // namespace LibSock
