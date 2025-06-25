#pragma once

#include "interfaces/IServer.hpp"
#include "types.hpp"
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace LibSock {

class Client;

struct SData {
	std::string				  msg;
	std::optional<WP<Client>> sender = std::nullopt;
};

class ClientManager : public std::enable_shared_from_this<ClientManager> {
  public:
	static SP<ClientManager> make(WP<Abstract::IServer> server);
	~ClientManager();

	std::pair<SP<Client>, std::future<void>> createClient(bool track = true, bool wait = false, std::function<void(SP<Client> &)> cb = [](SP<Client>) {});

	void broadcast(const SData &msg); // second param only specified when we
									  // want to exclude the sender
	void kick(WP<Client> client, const bool kill = false, std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>>::iterator &)>> cb = std::nullopt);
	void addClient(const Client &client);

	void shutdownClientManager(std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>> &)>> cb = std::nullopt);

	SP<Client>				getByIp(const std::string &ip) const;
	std::vector<SP<Client>> getClientManager() const;
	std::vector<SData>		getDatas() const;

  private:
	std::vector<std::pair<std::jthread, SP<Client>>> m_vClientManager;
	std::vector<SData>								 m_vDatas;

	WP<ClientManager> get();

	WP<ClientManager>	  m_self;
	WP<Abstract::IServer> m_wpServer;

	friend class Client;
};
} // namespace LibSock
