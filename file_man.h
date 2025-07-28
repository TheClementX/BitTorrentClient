#ifndef FILE_MAN_H
#define FILE_MAN_H

#include "util/parts.h"
#include "track_sate.h" 

class PieceManager {
	private:
		std::shared_ptr<TState> state; 
		std::vector<std::shared_ptr<Piece>> pieces; 
		std::vector<bool> recieved_pieces; 

		int verify_piece(int i); 
		int verify_file(); 
		std::vector<uint8_t> merge_piece(std::shared_ptr<Piece> piece); 

	public: 
		PieceManager(std::shared_ptr<TState> state); 

		void write_file(); 
		std::vector<BlockReq> choose_next_blocks(); 
		int write_block(BlockReq& block, std::vector<uint8_t>& data); 
		bool check_complete(); 
}; 

#endif 
