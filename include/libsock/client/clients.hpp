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

namespace LibSock::Client {

class Client;

struct SData {
	std::string				  msg;
	std::optional<WP<Client>> sender = std::nullopt;
};

class Clients : public std::enable_shared_from_this<Clients> {
  public:
	static SP<Clients> make(WP<Server> server);
	~Clients();

	std::pair<SP<Client>, std::future<void>> createClient(bool track = true, bool wait = false, std::function<void(SP<Client> &)> cb = [](SP<Client>) {});

	void broadcast(const SData &msg); // second param only specified when we
									  // want to exclude the sender
	void kick(WP<Client> client, const bool kill = false, std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>>::iterator &)>> cb = std::nullopt);

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
} // namespace LibSock::Client
