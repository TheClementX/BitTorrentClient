#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "util/parts.h" 
#include "util/common.h" 
#include "util/bitfield.h"

#define MAX_CON 40; 

class ConnectionManager {
	private:
		std::vector<std::shared_ptr<Peer>> peers; 
		std::shared_ptr<TState> state; 
		std::shared_ptr<BitField> field; 
		int epoll_fd; //only for peer connections
		struct epoll_event[MAX_CON]; 
		std::pair<int, int> serv_socket; //poll seperately
		int active_out_con; 
		int active_in_con; 

		std::map<int, std::shared_ptr<CState>> fd_con; 
		std::map<std::string, std::shared_ptr<CState>> pid_con; 

		//connection management functions
		int start_server_sock(); 
		int open_connection(std::shared_ptr<Peer> p); 
		int accept_connection(); 
		int send_handshake(int fd, std::shared_ptr<Peer> p); 
		std::shared_ptr<Peer> recv_handshake(int fd); 
		bool verify_handshake(std::string handshake, std::shared_ptr<Peer> p); 
		int get_ready_peers(); 
		int recieve_message(std::shared_ptr<Peer> p); 
			
		std::string bytes_to_string(uint32_t val); 

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
		std::shared_ptr<Block> get_block(std::shared_ptr<BlockReq> req); 
		void handle_peer_cycle(); 
		void set_bit_field(std::shared_ptr<BitField> field); 
		
		void connection_cleanup(); 
}; 

#endif
