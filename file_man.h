#ifndef FILE_MAN_H
#define FILE_MAN_H

#include "util/parts.h"
#include "track_sate.h" 

class PieceManager {
	private:
		std::shared_ptr<TState> state; 
		std::vector<std::shared_ptr<Piece>> pieces; 
		std::vector<bool> recieved_pieces; 

		int verify_file(std::shared_ptr<Piece>); 

	public: 
		PieceManager(std::shared_ptr<TState>); 

		int verify_piece(std::shared_ptr<Piece>); 
		void write_file(); 
		size_t choose_next_piece(); 
		bool check_complete(); 

}; 

#endif 
