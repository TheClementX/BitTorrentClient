#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "util/parts.h" 
#include "util/common.h" 
#include "util/bitfield.h"
#include <random>
#include <cstdint>

#define MAX_CON 40;
#define MAX_MES_LEN 

class ConnectionManager {
	private:
		std::vector<std::shared_ptr<Peer>> peers; 
		std::shared_ptr<TState> state; 
		std::shared_ptr<BitField> field; 
		std::shared_ptr<std::vector<std::shared_ptr<Piece>>> file_bits
		int epoll_fd; //only for peer connections
		struct epoll_ready[MAX_CON]; 
		std::pair<int, int> serv_socket; //poll seperately
		int active_out_con; 
		int active_in_con; 

		std::stack<std::shared_ptr<RecBlock>> recieved; 
		std::stack<std::shared_ptr<CliReq>> cli_requests; 
		std::vector<std::shared_ptr<BlockReq>> my_requsts; 
		std::map<int, std::shared_ptr<CState>> fd_con; 
		std::map<std::string, std::shared_ptr<CState>> pid_con; 

		//connection management functions
		int start_server_sock(); 
		int open_connection(std::shared_ptr<Peer> p); 
		int close_connection(std::shared_ptr<CState> p); 
		int accept_connection(); 
		int send_handshake(int fd, std::shared_ptr<Peer> p); 
		std::shared_ptr<Peer> recv_handshake(int fd); 
		bool verify_handshake(std::string handshake, std::shared_ptr<Peer> p); 
		int get_ready_peers(); 
		int recieve_message(std::shared_ptr<CState> p); 
			
		std::string bytes_to_string(uint32_t val); 
		uint32_t string_to_bytes(std::string val); 

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
		int send_cancel(int fd, int ind, int beg, int len);  
		int send_port(int fd); 

		//recieve protocol message functions
		int recv_keep_alive(std::shared_ptr<CState> p); 
		int recv_choke(std::shared_ptr<CState> p); 
		int recv_unchoke(std::shared_ptr<CState> p); 
		int recv_interested(std::shared_ptr<CState> p); 
		int recv_not_interested(std::shared_ptr<CState> p); 
		int recv_have(std::string m, std::shared_ptr<CState> p); 
		int recv_bitfield(std::string m, std::shared_ptr<CState> p); 
		int recv_request(std::string m, std::shared_ptr<CState> p); 
		int recv_piece(std::string m); 
		int recv_cancel(std::string m);  
		int recv_port(std::string m, std::shared_ptr<CState> p); 

		std::vector<uint8_t> join_piece(std::shared_ptr<Piece> p); 
		std::vector<uint8_t> get_send_data(int i, int b, int l); 

	public: 
		ConnectioManager(std::vector<std::shared_ptr<Peer>>& peers, 
						 std::shared_ptr<TState> state
		); 

		void refresh_peers(); 
		void set_peers(std::vector<std::shared_ptr<Peer>>& peers); 
		void push_block_req(std::shared_ptr<BlockReq> req); 
		void handle_peer_cycle(); 
		void set_bit_field(std::shared_ptr<BitField> field); 
		void set_file_bits(std::shared_ptr<std::vector<std::shared_ptr<Piece>>> file); 
		std::stack<std::shared_ptr<RecBlock>> get_recieved_blocks(); 
		
		void connection_cleanup(); 
}; 

#endif
