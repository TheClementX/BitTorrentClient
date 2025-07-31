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

int ConnectionManager::close_connection(std::shared_ptr<CState> p) {
	epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, p->fd, NULL); 
	this->fd_con.erase(p->fd);
	this->pid_con.erase(p->info->peer_id); 
	close(p->fd); 

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

	this->send_bitfield(clifd); 
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

int ConnectionManager::recieve_message(std::shared_ptr<CState> p) {
	std::string message; 
	int len, buf_len, tot_len, e; 
	buf_len = this->state->get_block_size() + 9; 
	char buf[buf_len]; 
	std::memset(&buf, 0, sizeof(buf)); 

	e = recv(p->fd, buf, 4, 0); 
	if(e <= 0 || e != 4)
		return -1; 
	message.append(buf, 4); 
	std::memset(&buf, 0, buf_len); 
	len += 4; 

	tot_len = static_cast<int>(string_to_bytes(message.substr(0, 4))); 
	if(tot_len == 0) {
		this->recv_keep_alive(p); 
		return 0; 
	}

	while(len < tot_len) {
		e = recv(p->fd, buf, buf_len, 0); 
		if(e <= 0)
			return -1; 
		len += e; 
		message.append(buf, e); 
	}

	int m_id = static_cast<int>(result[4]); 
	switch(m_id) {
		case 0:
			e = this->recv_choke(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 1:
			e = this->recv_unchoke(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 2:
			e = this->recv_interested(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 3:
			e = this->recv_not_interested(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 4:
			e = this->recv_have(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 5:
			e = this->recv_bitfield(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 6:
			e = this->recv_request(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 7:
			e = this->recv_piece(message); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 8:
			e = this->recv_cancel(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		case 9:
			e = this->recv_port(message, p); 
			if(e == -1)
				this->close_connection(p); 
			break;
		default:
			this->close_connection(p); 
			return -1; 
			break; 
	}
	return 0; 
}
	
//send protocol message functions
//0 = success, -1 = failure
std::string bytes_to_string(uint32_t val) {
	std::string result; 
	result.push_back((val >> 24) & 0xff); 
	result.push_back((val >> 16) & 0xff); 
	result.push_back((val >> 8) & 0xff); 
	result.push_back(val & 0xff); 
	return result; 
}

//for conversion of bytes over wire
uint32_t string_to_bytes(std::string val) {
	uint32_t result = 0; 
	result |= static_cast<uint32_t>(val[0]) << 24;
	result |= static_cast<uint32_t>(val[1]) << 16; 
	result |= static_cast<uint32_t>(val[2]) << 8; 
	result |= static_cast<uint32_t>(val[3]); 
	return result
}

int ConnectionManager::send_keep_alive(int fd) {
	int e; 
	std::string keep_alive; 
	keep_alive += bytes_to_string(0); 

	e = send(fd, keep_alive.data(), keep_alive.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_choke(int fd) {
	int e; 
	std::string choke; 
	choke += bytes_to_string(1); 
	choke += static_cast<char>(0); 

	e = send(fd, choke.data(), choke.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_unchoke(int fd) {
	int e; 
	std::string unchoke; 
	unchoke += bytes_to_string(1); 
	unchoke += static_cast<char>(1); 

	e = send(fd, unchoke.data(), unchoke.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_interested(int fd) {
	int e; 
	std::string interested;
	interested += bytes_to_string(1); 
	interested += static_cast<char>(2); 
	
	e = send(fd, interested.data(), interested.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_not_interested(int fd) {
	int e; 
	std::string not_interested; 
	not_interested += bytes_to_string(1); 
	not_interested += static_cast<char>(3); 

	e = send(fd, not_interested.data(), not_interested.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_have(int fd, int i) {
	int e; 
	std::string have; 
	have += bytes_to_string(5); 
	have += static_cast<char>(4); 
	have += bytes_to_string(i); 

	e = send(fd, have.data(), have.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_bitfield(int fd) {
	std::string bitfield; 
	std::vector<uint8_t> bits = this->field->get_bits(); 
	uint_32 len = 1 + bits.size(); 
	bitfield += bytes_to_string(len); 
	bitfield += static_cast<char>(5); 

	for(int i = 0; i < bits.size(); i++) 
		bitfield += static_cast<char>(bits[i]); 

	e = send(fd, bitfield.data(), bitfield.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_request(int fd, int ind, int beg, int len) {
	std::string request; 
	uint32_t len = 13; 
	request += bytes_to_string(len); 
	request += static_cast<char>(6); 
	request += bytes_to_string(reinterpret_cast<uint_32>(ind)); 
	request += bytes_to_string(reinterpret_cast<uint_32>(beg)); 
	request += bytes_to_string(reinterpret_cast<uint_32>(len)); 

	e = send(fd, request.data(), request.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_piece(int fd, int ind, int beg, std::vector<uint8_t> block) {
	int e;
	std::string piece; 
	int len = 9 + block.size(); 
	piece += bytes_to_string(len); 
	piece += static_cast<char>(7); 
	piece += bytes_to_string(reinterpret_cast<uint_32>(ind)); 
	piece += bytes_to_string(reinterpret_cast<uint_32>(beg)); 
	
	for(int i = 0; i < block.size(); i++) 
		piece += static_cast<char>(block[i]); 

	e = send(fd, piece.data(), piece.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

int ConnectionManager::send_cancel(int fd, int ind, int beg, int len) {
	int e; 
	std::string cancel; 
	int len = 13; 
	cancel += bytes_to_string(13); 
	cancel += static_cast<char>(8); 
	cancel += bytes_to_string(reinterpret_cast<uint_32>(ind)); 
	cancel += bytes_to_string(reinterpret_cast<uint_32>(beg)); 
	cancel += bytes_to_string(reinterpret_cast<uint_32>(len)); 
	
	e = send(fd, cancel.data(), cancel.size(), 0); 
	if(e < 0)
		return -1; 
	return 0; 
}

//no DHT support 
int ConnectionManager::send_port(int fd) {
	return 0; 
}

//recieve protocol message functions
int ConnectionManager::recv_keep_alive(std::shared_ptr<CState> p) {
	if(p->countdown < 20)
		p->countdown++; 
	return 0; 
}

int ConnectionManager::recv_choke(std::shared_ptr<CState> p) {
	p->i_choked = true; 
	return 0; 
}

int ConnectionManager::recv_unchoke(std::shared_ptr<CState> p) {
	p->i_choked = false; 
	return 0; 
}

int ConnectionManager::recv_interested(std::shared_ptr<CState> p) {
	p->p_interested = true; 
	return 0; 
}

int ConnectionManager::recv_not_interested(std::shared_ptr<CState> p) {
	p->p_interested = false; 
	return 0; 

}

int ConnectionManager::recv_have(std::string m, std::shared_ptr<CState> p) {
	int ind = string_to_bytes(m.substr(5)); 
	p->peer_pieces->set_true(ind); 
	return 0; 
}

int ConnectionManager::recv_bitfield(std::string m, std::shared_ptr<CState> p) {
	std::string bits = m.substr(5); 
	p->peer_pieces = BitField(bits);  
	return 0; 
}

int ConnectionManager::recv_request(std::string m, std::shared_ptr<CState> p) {
	if(p->p_choked)	
		return 0; 
	size_t i = static_cast<size_t>(string_to_bytes(m.substr(5,9)))
	size_t b = static_cast<size_t>(string_to_bytes(m.substr(9,13)))
	size_t l = static_cast<size_t>(string_to_bytes(m.substr(13,17)))
	this->request->push(std::make_shared<CliReq>(p, i, b, l)); 

	return 0; 
}

int ConnectionManager::recv_piece(std::string m) {
	int len = static_cast<int>(string_to_bytes(m.substr(0, 4))); 
	int ind = static_cast<int>(string_to_bytes(m.substr(5, 9))); 
	int beg = static_cast<int>(string_to_bytes(m.substr(9, 13))); 
	std::string s_data = m.substr(13); 
	std::vector<uint8_t> v_data; 

	for(char c : s_data) 
		v_data.push_back(static_cast<uint8_t>(c)); 


	this->recieved.push(std::make_shared<RecBlock>(ind, beg, v_data); 
	return 0; 
}

int ConnectionManager::recv_cancel(std::string m, std::shared_ptr<CState> p) {
	p->cancelled = true; 
	return 0; 
}

//no DHT support
int ConnectionManager::recv_port(std::string m, std::shared_ptr<CState> p) {
	return 0; 
}

//public functions
int ConnectionManager::refresh_peers() {

}

void ConnectionManager::set_peers(std::vector<std::shared_ptr<Peer>>& peers) {

}

std::shared_ptr<Block> ConnectionManager::get_block(std::shared_ptr<BlockReq> req) {

}

void ConnectionManager::handle_peer_cycle() {

}


void ConnectionManager::connection_cleanup() {

}

