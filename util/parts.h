#ifndef PARTS_H
#define PARTS_H

#include <cstdint> 
#include "util/bitfield.h"

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
	bool cancelled; 

	BitField peer_pieces; 
	std::shared_ptr<Peer> info; 

	CState(std::shared_ptr<Peer> info, int fd) {
		this->fd = fd; 
		this->i_choked = true; 
		this->i_interested = false; 
		this->p_choked = true; 
		this->p_interested = false; 
		this->count_down = 20; 
		this->cancelled = false; 

		this->info = info;
	}
}; 

class CliReq {
public: 
	std::shared_ptr<CState> p; 
	size_t index; 
	size_t begin; 
	size_t length; 

	CliReq(std::shared_ptr<CState> p, size_t i, size_t b, size_t l) {
		this->p = p; 
		this->index = i; 
		this->begin = b; 
		this->length = l;
	}
}; 

class BlockReq {
public: 
	size_t p_index; 
	size_t b_offset; 
	size_t length; 

	BlockReq(size_t i, size_t o, size_t l) {
		this->p_index = i; 
		this->b_offset = o; 
		this->length = l; 
	}
}; 

class RecBlock {
public:
	size_t index; 
	size_t begin; 
	std::vector<uint8_t> data;

	RecBlock(size_t i, size_t b, std::vector<uint8_t> data) {
		this->index = i; 
		this->begin = b; 
		this->data = data; 
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
	std::vector<bool> have; 

	Piece(int size) {
		this->data.resize(size);
		this->have.resize(size); 
	}
}; 

#endif; 
