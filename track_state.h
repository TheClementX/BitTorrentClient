#ifndef TRACK_STATE_H
#define TRACK_STATE_H

#include <openssl/sha.h>
#include "b_parser/bencoder.h"

class TState {
private:
	Parser p{}; 

	//t_file fields
	std::string t_file_path; 
	std::shared_ptr<Bencode> t_file; 
	std::map<std::string, std::shared_ptr<Bencode>> t_file_dict;
	std::map<std::string, std::shared_ptr<Bencode>> info_dict;

	//piece and block information
	size_t piece_size; 
	size_t file_size; 
	size_t num_pieces; 
	size_t block_size;
	size_t blocks_per_piece; 
	std::string s_pieces_hash; 
	std::vector<std::string> pieces_hashes; 

	//last piece and block info
	size_t nl_blocks; 
	size_t l_block_size; 

	//tracker fields
	std::string tracker_url; 
	int tracker_renew_interval; 

	std::string info_hash; 
	std::string peer_id; 
	int uploaded; 
	int downloaded; 
	int left; 
	int port; 
	int compact; 

	void extract_piece_hash(); 
	void make_info_hash(); 
	void make_peer_id(); 

public: 
	TState(std::string t_file_path); 

	std::string get_info_hash(); 
	std::string get_peer_id(); 
	std::string get_tracker_url(); 
	void print_t_file(); 

	size_t get_piece_size();
	size_t get_num_pieces(); 
	size_t get_file_size(); 
	std::string get_piece_hash(int i); 

	void set_port(int n); 
	int get_port(); 

	void set_uploaded(int n); 
	int get_uploaded(); 
	
	void set_downloaded(int n); 
	int get_downloaded(); 

	void set_left(int n); 
	int get_left(); 

	void set_compact(int n); 
	int get_compact(); 

	void set_tri(int n); 
	int get_tri(); 

	std::string get_t_name(); 

	size_t get_nl_blocks(); 
	size_t get_l_block_size(); 
	size_t get_num_pieces();  
	size_t get_blocks_per_piece(); 
}; 

#endif
