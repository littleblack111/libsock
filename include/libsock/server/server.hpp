#include "../interfaces/IServer.hpp"

namespace LibSock::Server {
class Server : public Abstract::IServer {
  private:
	void init(std::optional<sockaddr_in> addr = std::nullopt) override;
};
} // namespace LibSock::Server
