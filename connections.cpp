#include "connections.h"

ConnectionManager::ConnectioManager(std::vector<std::shared_ptr<Peer>>& peers, 
				 std::shared_ptr<TState> state
) {
	this->peers = peers; 
	this->state = state; 
	this->epoll_fd = epoll_create1(0); 
	this->serv_socket = this->start_server_sock(); 
	this->state->set_port(this->serv_socket.second); 
	this->active_connections = 0; 
}

//connection management functions
std::pair<int, int> ConnectionManager::start_server_sock() {
	std::pair<int, int> result; 

	struct sockaddr_in saddr; 
	socklen_t slen = sizeof(saddr); 
	int servfd, e; 
	std::memset(&saddr, 0, sizeof(saddr)); 
	saddr.sin_addr.s_addr = INADDR_ANY; 
	saddr.sin_port = 0; 
	saddr.sin_family = AF_INET; 

	servfd = socket(AF_INET, SOCK_STREAM, 0); 
	if(servfd < 0)
		throw std::runtime_error("servfd client socket call failed"); 

	e = bind(servfd, (struct sockaddr*) &saddr, slen); 
	if(e < 0)
		throw std::runtime_error("servfd client bind call failed"); 
	getaddrinfo(servfd, (struct sockaddr*) &saddr, slen); 

	struct epoll_event event; 
	std::memset(&event, 0, sizeof(event); 
	event.events = EPOLLIN; 
	event.data.fd = servfd; 
	epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, servfd, &event); 

	listen(servfd, 300); 
		
	result->first = servfd; result->second = ntohs(saddr.sin_port); 
	return result; 
}

int ConnectionManager::open_connection(std::shared_ptr<Peer> p) {
	if(this->active_out_con >= MAX_CON / 2)
		return -1; 
	if(this->pid_con.contains(p->peer_id))
		return -1;

	int sockfd, e; 
	struct sockaddr_in caddr; 
	socklen_t caddr_len = sizeof(caddr); 
	std::memset(&caddr, 0, caddr_len); 
	
	e = inet_pton(AF_INET, p->ip.data(), caddr.sin_addr.s_addr); 
	if(e <= 0)
		return -1; //will check con again which is not good
	caddr->sin_port = htons(p->port); 
	caddr->sin_family = AF_INET; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if(sockfd < 0)
		throw std::runtime_error("call to socker() failed in open_connection()"); 

	e = connect(sockfd, (struct sockaddr*) &caddr, caddr_len); 
	if(e < 0) 
		return -1; 
	
	//do handshake
	if(this->send_handshake(sockfd, p) == -1) {
		close(sockfd); 
		return -1; 
	}

	std::shared_ptr<CState> res_state = std::make_shared<CState>(p, sockfd); 	
	this->fd_con[sockfd] = res_state; 
	this->pid_con[p->peer_id] = res_state; 
	this->active_out_con++; 

	struct epoll_event event; 
	event.data.fd = sockfd; 
	event.events = EPOLLIN | EPOLLHUP; 
	epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, sockfd, &event); 

	return 0; 
}

int ConnectionManager::accept_connection() {
	if(this->active_in_con >= MAX_CON / 2)
		return -1; 

	int clifd; 
	struct sockaddr_in caddr; 
	socklen_t caddr_len = sizeof(caddr); 
	std::memset(&caddr, 0, caddr_len); 
	clifd = accept(this->serv_socket.first, &caddr, caddr_len); 
	if(clifd < 0)
		return -1; 

	std::shared_ptr<Peer> p = this->recv_handshake(clifd); 
	if(p == nullptr)
		return -1; 
	p->port = caddr->sin_port; 
	inet_ntop(AF_INET, caddr.sin_addr.s_addr, p->ip.data(), p->ip.size()); 

	std::shared_ptr<CState> p_state = std::make_shared<CState>(clifd, p);
	if(this->pid.contains(p->peer_id)) {
		close(clifd); 
		return -1; 
	}

	this->pid_con[p->peer_id] = p_state; 
	this->fd_con[clifd] = p_state; 
	this->active_in_con++; 

	struct epoll_event event; 
	event.data.fd = clifd; 
	event.events = EPOLLIN | EPOLLHUP; 
	epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, clifd, &event); 

	return 0; 
}

int ConnectionManager::send_handshake(int fd, std::shared_ptr<Peer> p) {
	int e; 
	std::string handshake; 
	handshake += static_cast<char>(19); 
	handshake.append("BitTorrent protocol"); 
	std::string reserved(8, '\0'); 
	handshake.append(reserved); 
	handshake.append(this->state->get_info_hash()); 
	handshake.append(this->state->get_peer_id()); 

	e = send(sockfd, handshake.data(), handshake.size()); 
	if(e < 0) 
		throw std::runtime_error("send() call failed in send_handshake()"); 

	struct pollfd p[1]; 
	p[0].fd = fd; 
	p[0].events = POLLIN; 
	e = poll(p, 1, 3000); 
	if(e < 0)
		throw std::runtime_error("poll() call failed in send_handshake()"); 
	
	std::string response; 
	if(p.revents & POLLIN) {
		int b_rec = 0; 
		char buff[68]; 
		std::memset(&buff, 0, sizeof(buff));
		while(b_rec < 68) {
			e = recv(fd, buff, 68, 0); 
			if(e <= 0)
				return -1; 
			b_rec += e; 
			response.append(buff, e); 
			std::memset(&buff, 0, sizeof(buff)); 
		}
	} else {
		//client did not respond
		return -1; 
	}
	if(!this->verify_handshake(response, p))
		return -1;
	return 0; 
}

std::shared_ptr<Peer> ConnectionManager::recv_handshake(int fd) {
	int e; 
	std::string handshake; 
	struct pollfd p[1]; 
	p[0].fd = fd; 
	p[0].events = POLLIN; 

	e = poll(p, 1, 3000);  
	if(e < 0) 
		throw std::runtime_error("call to poll() in recv_handshake() failed"); 

	char buff[68]; 
	int b_rec = 0; 
	std::memset(&buff, 0, sizeof(buff)); 
	while(b_rec < 68) {
		e = recv(fd, buff, sizeof(buf), 0); 
		if(e <= 0)
			return -1; 
		b_rec += e; 
		handshake.append(buff, e); 
		std::memset(&buf, 0, sizeof(buf)); 
	}

	int pstrlen = static_cast<int>(handshake[0]); 
	std::string pid = handshake.substr(29+pstrlen, 49+pstrlen); 
	std::shared_ptr<Peer> result = std::make_shared(pid, "", 0); 

	if(!this->verify_handshake(handshake, result)) {
		return nullptr; 
	}
	return result; 
}

bool ConnectionManager::verify_handshake(std::string handshake, std::shared_ptr<Peer> p) {
	int pstrlen = static_cast<int>(handshake[0]); 
	std::string pstr = handshake.substr(1, 1+pstrlen); 
	std::string p_info_hash = handshake.substr(9+pstrlen, 29+pstrlen); 
	std::string pid = handshake.substr(29+pstrlen, 49+pstrlen); 
	if(pstr != "BitTorrent protocol")
		return false; 
	if(p_info_hash != this->state->get_info_hash())
		return false; 
	if(pid != p->peer_id)
		return false; 
	return true; 
}

int ConnectionManager::get_ready_peers() {

}

int ConnectionManager::recieve_message(std::shared_ptr<Peer> p) {

}
	
//send protocol message functions
//0 = success, -1 = failure
int ConnectionManager::send_keep_alive(int fd) {
	int e; 
	std::string keep_alive; 
	keep_alive += static_cast<char>(0); 

	e = send(fd, keep_alive.data(), keep_alive.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_choke(int fd) {
	int e; 
	std::string choke; 
	choke += static_cast<char>(1); 
	choke += static_cast<char>(0); 

	e = send(fd, choke.data(), choke.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_unchoke(int fd) {
	int e; 
	std::string unchoke; 
	unchoke += static_cast<char>(1); 
	unchoke += static_cast<char>(1); 

	e = send(fd, unchoke.data(), unchoke.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_interested(int fd) {
	int e; 
	std::string interested;
	interested += static_cast<char>(1); 
	interested += static_cast<char>(2); 
	
	e = send(fd, interested.data(), interested.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_not_interested(int fd) {
	int e; 
	std::string not_interested; 
	not_interested += static_cast<char>(1); 
	not_interested += static_cast<char>(3); 

	e = send(fd, not_interested.data(), not_interested.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_have(int fd) {

	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_bitfield(int fd) {

	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_request(int fd, int ind, int beg, int len) {

	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_piece(int fd, int ind, int beg, std::vector<uint8_t> block) {

	if(e < 0)
		return -1; 
	return 0; 
}


//recieve protocol message functions
int ConnectionManager::recv_keep_alive(int fd); 
int ConnectionManager::recv_choke(int fd); 
int ConnectionManager::recv_unchoke(int fd); 
int ConnectionManager::recv_interested(int fd); 
int ConnectionManager::recv_not_interested(int fd); 
int ConnectionManager::recv_have(int fd); 
int ConnectionManager::recv_bitfield(int fd); 
int ConnectionManager::recv_request(int fd); 
std::vector<unit8_t> ConnectionManager::recv_piece(int fd); 

//public functions
int ConnectionManager::refresh_peers(); 
void ConnectionManager::set_peers(std::vector<std::shared_ptr<Peer>>& peers); 
std::shared_ptr<Block> ConnectionManager::get_block(std::shared_ptr<BlockReq> req); 
void ConnectionManager::handle_peer_cycle(); 

void ConnectionManager::connection_cleanup(); 
