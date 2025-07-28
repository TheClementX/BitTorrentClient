#include "file_man.h" 

PieceManager::PieceManager(std::shared_ptr<TState> state) {
	this->state = state; 
	this->pieces.resize(state.get_num_pieces()); 
	this->recieved_pieces = std::vector<bool>(false, this->pieces.size()); 
}

int PieceManager::verify_piece(int i) {
	std::string true_hash = this->state->get_piece_hash(i); 	
	std::string trial_hash(32, '\0'); 
	std::vector<unit8_t> piece

	if(i != this->state->get_num_piecs()) {
		
	} else {

	}

	
}

std::vector<uint8_t> PieceManager::merge_piece(std::shared_tr<Piece> piece) {

}

int PieceManager::verify_file(std::shared_ptr<Piece> pieces) {

}

void PieceManager::write_file() {
	this->verify_file(); 

	std::ofstream ofile(this->state->get_t_name(), std::ios::binary); 

	size_t block_size = this->state->get_block_size(); 
	size_t num_pieces = this->state->get_num_pieces(); 
	size_t blocks_per_piece = this->state->get_blocks_per_piece(); 

	//write out all but last piece
	for(int i = 0; i < num_pieces-1; i++) {
		std::shared_ptr<Piece> curp = this->pieces[i]; 
		for(int j = 0; j < blocks_per_piece; i++) {
			std::shared_ptr<Block> curb = curp->data[j]; 
			ofile.write(curb->data.data(), block_size); 
		}
	}

	//write out last piece
	size_t nl_blocks = this->state->get_nl_blocks(); 
	size_t l_block_size = this->state->get_l_block_size(); 
	std::shared_pr<Piece> l_piece = this->pieces[this->pieces.size()];
	for(int i = 0; i < nl_blocks-1; i++) {
		std::shared_ptr<Block> curb = l_piece->data[j]; 
		ofile.write(curb->data.data(), block_size); 
	}

	std::shared_ptr<Block> lblock = l_piece->data[nl_blocks-1]; 
	ofile.write(lblock->data.data(), l_block_size); 
}

std::vector<BlockReq> PieceManager::choose_next_blocks() {

}

int PieceManager::write_block(BlockReq& block, std::vector<uint8_t>& data) {

}

bool PieceManager::check_complete() {

}

