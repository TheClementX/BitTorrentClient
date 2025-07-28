#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "util/parts.h" 

class ConnectionManager {
	private:
		std::vector<std::shared_ptr<Peer>> peers; 
		std::shared_ptr<TState> state; 
		int e_poll_fd; 
		std::vector<bool> ready(200); 

		std::map<int, std::shared_ptr<CState>> fd_con; 
		std::map<std::string, std::shared_ptr<CState>> pid_con; 

		int open_connection(Peer p); 
		int get_ready_peers(); 
		int recieve_message(); 
			
		//send protocol message functions
		int send_keep_alive(int fd); 
		int send_choke(int fd); 
		int send_unchoke(int fd); 
		int send_interested(int fd); 
		int send_not_interested(int fd); 
		int send_have(int fd); 
		int send_bitfield(int fd); 
		int send_request(int fd, int ind, int beg, int len); 
		int send_piece(int fd, int ind, int beg, std::vector<uint8_t> block); 

		//recieve protocol message functions
		int recv_keep_alive(int fd); 
		int recv_choke(int fd); 
		int recv_unchoke(int fd); 
		int recv_interested(int fd); 
		int recv_not_interested(int fd); 
		int recv_have(int fd); 
		int recv_bitfield(int fd); 
		int recv_request(int fd); 
		std::vector<unit8_t> recv_piece(int fd); 

	public: 
		ConnectioManager(std::vector<std::shared_ptr<Peer>>& peers, 
						 std::shared_ptr<TState> state
		); 

		int refresh_peers(); 
		void set_peers(std::vector<std::shared_ptr<Peer>>& peers); 
		Piece get_piece(size_t offset); 
		void handle_peer_cycle(); 
}; 

#endif
