#ifndef PARTS_H
#define PARTS_H

#include <cstdint> 

class Peer {
public:
	std::string peer_id; 
	std::string ip; 
	int port; 

	Peer(std::string pd, std::string ip, int port) {
		this->peer_id = pd; 
		this->ip = ip; 
		this->port = port; 
	}
}; 

class CState {
public: 
	int fd; 

	bool i_choked; 
	bool i_interested; 
	bool p_choked; 
	bool p_interested; 

	int count_down; 

	std::vector<bool> peer_pieces; 
	std::shared_ptr<Peer> info; 

	CState(std::shared_ptr<Peer> info, int fd) {
		this->fd = fd; 
		this->i_choked = true; 
		this->i_interested = false; 
		this->p_choked = true; 
		this->p_interested = false; 
		this->count_down = 20; 

		this->info = info;
	}
}; 


class Block {
public: 
	std::vector<uint8_t> data; 

	Block(int size) {this->data.resize(size);}
}; 

class Piece {
public:
	std::vector<std::shared_ptr<Block>> data; 

	Piece(int size) {this->data.resize(size);}
}; 
