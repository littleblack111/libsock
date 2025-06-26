import libsock.clientManager;
import libsock.client;
import std;

using namespace LibSock;

void ClientManager::shutdownClientManager(std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>> &)>> cb) {
	if (m_vClientManager.empty())
		return;

	if (cb && !(*cb)(m_vClientManager))
		return;

	for (auto &[thread, client] : m_vClientManager) {
		kick(WP<Client>(client));
		if (thread.joinable())
			client->m_wait ? thread.join() : thread.detach();
	}

	m_vClientManager.clear();
}

SP<ClientManager> ClientManager::make(WP<Abstract::IServer> server) {
	auto c		  = SP<ClientManager>(new ClientManager());
	c->m_self	  = c;
	c->m_wpServer = server;
	return c;
}

WP<ClientManager> ClientManager::get() {
	if (m_self.expired())
		m_self = shared_from_this();
	return m_self;
}

ClientManager::~ClientManager() {
	shutdownClientManager();
}

std::pair<SP<Client>, std::future<void>> ClientManager::createClient(bool track, bool wait, std::function<void(SP<Client> &)> cb) {
	auto promise = std::make_shared<std::promise<void>>();
	SP	 client	 = SP<Client>(new Client(m_wpServer.lock(), shared_from_this(), track, wait));
	m_vClientManager.emplace_back(std::jthread([client, cb, promise]() mutable {
									  client->init();
									  promise->set_value();
									  cb(client);
								  }),
								  client);
	client->m_self = WP<Client>(client);
	return {client, promise->get_future()};
}

void ClientManager::broadcast(const SData &msg) {
	for (const auto &[thread, client] : m_vClientManager) {
		if (!client)
			continue;

		client->write(msg.msg);
	}

	m_vDatas.emplace_back(msg);
}

SP<Client> ClientManager::getByIp(const std::string &ip) const {
	auto it = std::ranges::find_if(m_vClientManager, [&ip](const auto &s) { return s.second->getIp() == ip; });
	return it != m_vClientManager.end() ? it->second : nullptr;
}

std::vector<SP<Client>> ClientManager::getClientManager() const {
	return m_vClientManager | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

std::vector<SData> ClientManager::getDatas() const {
	return m_vDatas;
}

void ClientManager::kick(WP<Client> clientWeak, const bool kill, std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>>::iterator &)>> cb) {
	auto client = clientWeak.lock();
	if (!client)
		return;

	for (auto it = m_vClientManager.begin(); it != m_vClientManager.end(); ++it) {
		if (it->second == client) {
			if (client)
				// this only exist when the client is registered
				// not sure why the second is null
				if (cb && !(*cb)(it))
					return;

			const auto native_handle = it->first.native_handle();
			if (it->first.joinable())
				it->second->m_wait ? it->first.join() : it->first.detach();
			it->second.reset();
			m_vClientManager.erase(it);
			if (kill && native_handle != 0)
				// cancel(terminal/kill) the thread if it doesn't exit on its own
				// pthread_cancel cuz it's the safest https://stackoverflow.com/a/3438576
				// need a condition because it might crash if the thread is already dead
				pthread_cancel(native_handle);
			return;
		}
	}
}
