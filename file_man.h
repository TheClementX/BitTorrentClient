#ifndef FILE_MAN_H
#define FILE_MAN_H

#include "util/parts.h"
#include "track_sate.h" 

class PieceManager {
	private:
		std::shared_ptr<TState> state; 
		std::vector<std::shared_ptr<Piece>> pieces; 
		std::vector<bool> recieved_pieces; 
		int cur_piece; 
		bool complete; 

		bool verify_piece(int i); 
		bool piece_has_all_blocks(int i); 
		bool verify_file(); 
		std::vector<uint8_t> merge_piece(std::shared_ptr<Piece> piece); 

	public: 
		PieceManager(std::shared_ptr<TState> state); 

		void write_file(); 
		std::vector<std::shared_ptr<BlockReq>> choose_next_blocks(); 
		void write_block(std::shared_ptr<BlockReq> block, std::vector<uint8_t>& data); 
		bool check_complete(); 
}; 

#endif 
