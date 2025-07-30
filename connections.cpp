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
	std::shared_ptr<CState> res_state = std::make_shared<CState>(p, sockfd); 	
	this->fd_con[sockfd] = res_state; 
	this->pid_con[p->peer_id] = res_state; 

	return 0; 
}

int ConnectionManager::get_ready_peers() {

}

int ConnectionManager::recieve_message(std::shared_ptr<Peer> p); 
	
//send protocol message functions
int ConnectionManager::send_keep_alive(int fd); 
int ConnectionManager::send_choke(int fd); 
int ConnectionManager::send_unchoke(int fd); 
int ConnectionManager::send_interested(int fd); 
int ConnectionManager::send_not_interested(int fd); 
int ConnectionManager::send_have(int fd); 
int ConnectionManager::send_bitfield(int fd); 
int ConnectionManager::send_request(int fd, int ind, int beg, int len); 
int ConnectionManager::send_piece(int fd, int ind, int beg, std::vector<uint8_t> block); 

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
