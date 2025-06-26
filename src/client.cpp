import libsock.client;
import libsock.clientManager;
import libsock.interfaces;
import libsock.types;
import libsock.fileDescriptor;
import std;

using namespace LibSock;

Client::Client(SP<Abstract::IServer> server, SP<ClientManager> clients, bool track, bool wait)
	: m_track(track)
	, m_wait(wait) {
	m_wpServer		  = std::move(server);
	m_wpClientManager = std::move(clients);

	if (m_wpClientManager.expired())
		throw std::runtime_error("server:client: ClientManager doesn't exist");
	if (m_wpServer.expired())
		throw std::runtime_error("server:client: Server doesn't exist");
}

void Client::init() {
	m_sockfd = std::make_shared<LibSock::CFileDescriptor>(accept(m_wpServer.lock()->getSocket()->get(), reinterpret_cast<sockaddr *>(&m_addr), &m_addrLen)); // if this is in the init list, it will run before
																																							 // m_addrLen, so it won't work :/

	if (!m_sockfd->isValid())
		throw std::runtime_error("server:client: Failed to create socket");

	m_ip.resize(INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &m_addr.sin_addr, &m_ip[0], INET_ADDRSTRLEN);
	m_ip.resize(strlen(m_ip.c_str()));
	m_port = ntohs(m_addr.sin_port);
}

Client::~Client() {
	m_sockfd.reset();
}

void Client::recvLoop(std::optional<std::function<bool(const SRecvData &)>> cb) {
	while (true) {
		auto recvData = read();
		if (!recvData->good)
			break;

		recvData->sanitize();
		if (recvData->isEmpty())
			continue;

		if (cb && !(*cb)(*recvData))
			return;
	}
}

bool SRecvData::isEmpty() const {
	return data.empty() || std::ranges::all_of(data, [](char c) { return std::isspace(c); });
}

void SRecvData::sanitize() {
	if (!data.empty() && data.back() == '\n')
		data.pop_back();

	data.erase(std::remove(data.begin(), data.end(), asciiEscape), data.end()); // don't accept ASCII code, might mess up terminal
																				// // NOLINT

	if (const auto start = data.find_first_not_of(" \t\r\n");
		start != std::string::npos) {
		const auto end = data.find_last_not_of(" \t\r\n");
		data		   = data.substr(start, end - start + 1);
	} else
		data.clear();
}

UP<SRecvData> Client::read(std::optional<std::function<bool(const SRecvData &)>> cb) {
	if (!m_sockfd || !m_sockfd->isValid()) {
		if (cb)
			(*cb)({.data = "", .size = 0, .good = false});
		return nullptr;
	}
	auto recvData = std::make_unique<SRecvData>();

	recvData->data.resize(recvData->size);
	ssize_t size = recv(m_sockfd->get(), &recvData->data[0], recvData->size, 0);
	m_szReading.reset();

	if (size < 0 || size == 0) {
		recvData->good = false;
		if (cb && !(*cb)(*recvData))
			return nullptr;
	} else {
		recvData->data.resize(size);
		recvData->good = true;
	}

	if (m_track)
		m_wpClientManager.lock()->m_vDatas.emplace_back(SData{recvData->data, m_self});

	return recvData;
}

UP<SRecvData> Client::read(const std::string &msg, std::optional<std::function<bool(const SRecvData &)>> cb) {
	write("{}", msg);
	m_szReading = msg;
	return read(cb);
}

void Client::runLoop(bool resumeHist, std::optional<std::function<bool(const SRecvData &)>> cb) {
	if (resumeHist)
		resumeHistory();

	recvLoop(cb);

	m_wpClientManager.lock()->kick(m_self);
}

void Client::resumeHistory() {
	for (const auto &chat : m_wpClientManager.lock()->getDatas())
		write(chat.msg);
}

bool Client::isValid() {
	if (!m_sockfd)
		return false;

	if (!m_sockfd->isValid())
		return false;

	int		  err  = 0;
	socklen_t size = sizeof(err);
	return getsockopt(m_sockfd->get(), SOL_SOCKET, SO_ERROR, &err, &size) >= 0 && err == 0;
}

bool Client::write(const std::string &msg, std::optional<std::function<bool(const SRecvData &)>> cb) {
	auto r = write("{}", msg);
	if (cb)
		r = (*cb)({.data = msg, .size = msg.size(), .good = r});

	return r;
}

const std::string &Client::getIp() const { return m_ip; }

template <typename... Args>
bool Client::write(std::format_string<Args...> fmt, Args &&...args) {
	if (!m_sockfd || !m_sockfd->isValid())
		return false;

	std::string msg = std::format(fmt, std::forward<Args>(args)...);
	if (m_szReading) {
		msg.insert(0, "\r");
		msg.append(*m_szReading);
	}

	if (send(m_sockfd->get(), msg.c_str(), msg.size(), 0) < 0) {
		// cb somehow
		return false;
	}
	return true;
}
