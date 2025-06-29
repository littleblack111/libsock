#include "libsock/client/clients.hpp"
#include "libsock/client/client.hpp"
#include <algorithm>
#include <functional>
#include <ranges>
#include <unistd.h>
#include <utility>

using namespace sock::Client;

void Clients::shutdownClients(std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>> &)>> cb) {
	if (m_vClients.empty())
		return;

	if (cb && !(*cb)(m_vClients))
		return;

	for (auto &[thread, client] : m_vClients) {
		kick(WP<Client>(client));
		if (thread.joinable())
			client->m_wait ? thread.join() : thread.detach();
	}

	m_vClients.clear();
}

Clients::Clients() {
	if (std::atexit([]() {
			for (auto c : vpClients)
				if (const auto p = c.lock())
					p->shutdownClients();
		}))
		void(); // failed to register exit handler...
}

SP<Clients> Clients::make(WP<Server> server) {
	auto c = SP<Clients>(new Clients());
	vpClients.emplace_back(c);
	c->m_self	  = c;
	c->m_wpServer = server;
	return c;
}

WP<Clients> Clients::get() {
	if (m_self.expired())
		m_self = shared_from_this();
	return m_self;
}

Clients::~Clients() {
	shutdownClients();
	vpClients.erase(std::remove_if(vpClients.begin(), vpClients.end(), [this](const WP<Clients> &wptr) { return !wptr.owner_before(m_self) && !m_self.owner_before(wptr); }), vpClients.end());
}

std::pair<SP<Client>, std::future<void>> Clients::createClient(bool track, bool wait, std::function<void(SP<Client> &)> cb) {
	auto promise = std::make_shared<std::promise<void>>();
	SP	 client	 = SP<Client>(new Client(m_wpServer.lock(), shared_from_this(), track, wait));
	m_vClients.emplace_back(std::jthread([client, cb, promise]() mutable {
								client->init();
								promise->set_value();
								cb(client);
							}),
							client);
	client->m_self = WP<Client>(client);
	return {client, promise->get_future()};
}

void Clients::broadcast(const SData &msg) {
	for (const auto &[thread, client] : m_vClients) {
		if (!client)
			continue;

		client->write(msg.msg);
	}

	m_vDatas.emplace_back(msg);
}

SP<Client> Clients::getByIp(const std::string &ip) const {
	auto it = std::ranges::find_if(m_vClients, [&ip](const auto &s) { return s.second->getIp() == ip; });
	return it != m_vClients.end() ? it->second : nullptr;
}

std::vector<SP<Client>> Clients::getClients() const {
	return m_vClients | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

std::vector<SData> Clients::getDatas() const {
	return m_vDatas;
}

void Clients::kick(WP<Client> clientWeak, const bool kill, std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client>>>::iterator &)>> cb) {
	auto client = clientWeak.lock();
	if (!client)
		return;

	for (auto it = m_vClients.begin(); it != m_vClients.end(); ++it) {
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
			m_vClients.erase(it);
			if (kill && native_handle != 0)
				// cancel(terminal/kill) the thread if it doesn't exit on its own
				// pthread_cancel cuz it's the safest https://stackoverflow.com/a/3438576
				// need a condition because it might crash if the thread is already dead
				pthread_cancel(native_handle);
			return;
		}
	}
}
