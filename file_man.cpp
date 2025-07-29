#include "file_man.h" 

PieceManager::PieceManager(std::shared_ptr<TState> state) {
	this->state = state; 
	this->pieces.resize(state.get_num_pieces()); 
	this->recieved_pieces = std::vector<bool>(false, this->pieces.size()); 
	this->cur_piece = 0;  
	this->complete = false; 
}

bool PieceManager::verify_piece(int i) {
	std::string true_hash = this->state->get_piece_hash(i); 	
	std::string trial_hash(32, '\0'); 
	std::vector<uint8_t> piece = this->merge_piece(this->pieces[i]); 

	if(i != this->state->get_num_piecs()-1) {
		SHA1(reinterpret_cast<const unsigned char*>(piece.data()), 
			   piece.size(), 
			   reinterpret_cast<unsigned char*>(trial_hash.data())
	); 	
	} else {
		int size = 
			((this->state->get_nl_blocks()-1) * 
			 this->state->get_block_size()) + 
			(this->state->get_l_block_size()); 

		SHA1(reinterpret_cast<const unsigned char*>(piece.data()), 
			   size, 
			   reinterpret_cast<unsigned cahr*>(trial_hash.data())
	); 
	}
	if(true_hash == trial_hash) 
		this->recieved_pieces[i] = true; 
		return true; 
	else {
		this->pieces[i].clear(); 
		this->pieces[i].resize(state.get_blocks_per_piece()); 
	}
	return false; 
}

bool PieceManager::piece_has_all_blocks(int i) {
	std::shared_ptr<Piece> piece = this->pieces[i]; 
	int np = this->state->get_num_pieces(); 
	if(i != this->state->get_num_pieces()-1) {
		int bpp = this->state->get_blocks_per_piece(); 
		for(int i = 0; i < bpp; i++) {
			if(!piece->have[i])
				return false; 
		}
	} else {
		int bpp = this->state->get_nl_blocks(); 
		for(int i = 0; i < bpp; i++) {
			if(!piece->have[i])
				return false; 
		}
	}
	return true; 
}

std::vector<uint8_t> PieceManager::merge_piece(std::shared_tr<Piece> piece) {
	std::vector<uint8_t> result; 
	int num_blocks = piece.data.size(); 
	for(int i = 0; i < num_blocks; i++) {
		Block curb = piece->data[i]; 
		int bs = curb.size(); 
		for(int j = 0; j < bs; j++) 
			result.push_back(curb->data[i]); 
	}
	return result; 
}

bool PieceManager::verify_file() {
	for(int i = 0; i < this->pieces.size(); i++) {
		if(!this->verify_piece(i))
			return false; 
	}
	return true; 
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
	std::shared_ptr<Piece> l_piece = this->pieces[this->pieces.size()-1];
	for(int i = 0; i < nl_blocks-1; i++) {
		std::shared_ptr<Block> curb = l_piece->data[j]; 
		ofile.write(curb->data.data(), block_size); 
	}

	std::shared_ptr<Block> lblock = l_piece->data[nl_blocks-1]; 
	ofile.write(lblock->data.data(), l_block_size); 
}

std::vector<std::shared_ptr<BlockReq>> PieceManager::choose_next_blocks() {
	if(this->piece_has_all_blocks(this->cur_piece)) {
		if(this->verify_piece(this->cur_piece))
			this->cur_piece++; 
	}
	if(this->cur_piece >= this->pieces.size()) {
		if(this->check_complete())
			return nullptr; 
	}

	std::vector<std::shared_ptr<BlockReq>> result; 
	Piece p = this->pieces[this->cur_piece]; 
	if(this->cur_piece != this->state->get_num_pieces()-1) {
		int count = 0; 
		int nb = p->data.size(); 
		for(int i = 0; i < nb; i++) {
			if(count >= 3) break; 
			if(!p->have[i]) {
				result.push_back(
					std::make_shared<BlockReq>(this->cur_piece, i, this->state->get_block_size()
				); 
			}
			count++; 
		}
	} else {
		int count = 0; 
		int nb = this->state->get_nl_blocks()
		for(int i = 0; i < nb; i++) {
			if(count >= 3) break; 
			if(!p->have[i] && i = nb-1) {
				result.push_back(
					std::make_shared<BlockReq>(this->cur_piece, i, this->state->get_block_size()
				); 
			}
			else {
				result.push_back(
:					std::make_shared<BlockReq>(this->cur_piece, i, this->state->get_l_block_size()
				); 
			}
			count++; 
		}
	}
}

void PieceManager::write_block(std::shared_ptr<BlockReq> block, std::vector<uint8_t>& data) {
	std::shared_ptr<Piece> p = this->pieces[block->p_index]; 
	std::shared_ptr<Block> b = p->data[block->b_offset]; 
	b->data = data; 
	p->have[block->b_offset] = true; 
}

bool PieceManager::check_complete() {
	if(!this->complete) {
		for(int i = 0; i < this->pieces.size(); i++) {	
			if(!piece_has_all_blocks(i))
				return false; 
			if(!this->verify_piece(i))
				return false; 
		}
		this->complete = true; 
	}
	return true; 
}

