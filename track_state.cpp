#include "track_state.h"
#include <random> 

TState::TState(std::string t_file_path) {
	//set t_file fields; 
	this->t_file_path = t_file_path; 
	this->p.load_file(this->t_file_path); 
	this->t_file = this->p.parse_file(); 
	this->t_file_dict = 
		std::get<std::map<std::string, std::shared_ptr<Bencode>>>(this->t_file->val); 
	std::shared_ptr<Bencode> id = this->t_file_dict["info"]; 
	this->info_dict = 
		std::get<std::string, std::shared_ptr<Bencode>>(id->val); 


	//set pieces and blocks fields
	this->piece_size = 
		static_cast<size_t>(std::get<int64_t>(this->info_dict["piece length"]->val)); 
	this->file_size = 
		static_cast<size_t>(std::get<int64_t>(this->info_dict["length"]->val)); 
	this->s_pieces_hash = 
		std::get<std::string(this->info_dict["pieces"]->val); 
	this->extract_piece_hash(); 
	this->block_size = 16 * 1024; 
	this->blocks_per_piece = this->piece_size / this->block_size; 
	int last_piece_size = 
		this->file_size - ((this->num_pieces-1) * this->piece_size); 
	this->nl_blocks = std::ceil(last_piece_size / this->block_size); 
	this->l_block_size = last_piece_size - ((nl_blocks-1) * this->block_size)

	//set tracker fields 
	this->tracker_url =	
		std::get<std::string>(this->t_file_dict["announce"]->val); 
	this->make_info_hash(); 
	this->make_peer_id(); 
	this->tracker_renew_interval = 0; 
	this->uploaded = 0; 
	this->downloaded = 0;
	this->left = 0; 
	this->compact = 0; 
	this->port = 0; 
}

void TState::make_info_hash() {
	std::string info = this->p.encode(t_file_dict["info"]); 
	std::string hash(20, '\0'); 

	SHA1(reinterpret_cast<const unsigned char*>(info.data()), 
		info.size(), 
		reinterpret_cast<unsigned char*>(hash.data())
	); 

	this->info_hash = hash; 
}

void TState::make_peer_id() {
	std::string result; 
	result.append("-CL3105-"); 
	
	std::string ascii = 
		"0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMOPQRSTUVWXYZ"; 

	std::random_device rd; 
	std::mt19937 gen(rd()); 
	std::uniform_int_distribution<> dist{0, 61}; 

	for(int i = 0; i < 12; i++) {
		int r = dist(gen); 
		result.push_back(ascii[r]); 
	}

	this->peer_id = result; 
}

void TState::extract_piece_hash() {
	std::string to_extract = this->s_pieces_hash; 
	this->num_pieces = this->s_pieces_hash.size() / 32;

	for(int i = 0; i < this->num_pieces; i++) {
		this->pieces_hashes.push_back(to_extract.substr(0, 32)); 
		to_extract = to_extract.substr(32, to_extract.size()); 
	}
}

std::string TState::get_info_hash() {
	return this->info_hash; 
}

std::string TState::get_peer_id() {
	return this->peer_id;
}

std::string TState::get_tracker_url() {
	return this->tracker_url;
}

void TState::print_t_file() {
	this->p.print_file(this->t_file); 
}

size_t TState::get_piece_size() {
	return this->piece_size; 
}

size_t TState::get_num_pices() {
	return this->num_pieces; 
}

size_t TState::get_file_size() {
	return this->file_size; 
}

std::string TState::get_piece_hash(int i) {
	return this->piece_hashes[i]; 
}

void TState::set_port(int n) {
	this->port = n; 
}

int TState::get_port() {
	return this->port; 
}

void TState::set_uploaded(int n) {
	this->uploaded = n; 
}

int TState::get_uploaded() {
	return this->uploaded; 
}

void TState::set_downloaded(int n) {
	this->downloaded = n; 
}

int TState::get_downloaded() {
	return this->downloaded; 
}

void TState::set_left(int n) {
	this->left = n; 
}

int TState::get_left() {
	return this->left; 
}

void TState::set_compact(int n) {
	if(n == 1 || n == 0)
		this->compact = n; 
}

int TState::get_compact() {
	return this->compact; 
}

void TState::set_tri(int n) {
	this->tracker_renew_interval = n; 
}

int TState::get_tri() {
	return this->tracker_renew_interval; 
}

std::string TState::get_t_name() {
	std::string result = 
		std::get<std::string>(this->info_dict["name"]->val); 
	return result; 
}

size_t TState::get_nl_blocks() {
	return this->nl_blocks; 
}

size_t TState::get_l_block_size() {
	return this->l_block_size; 
}

size_t TState::get_num_pieces() {
	return this->num_pieces; 
}

size_t TState::get_blocks_per_piece() {
	return this->blocks_per_piece;
}

