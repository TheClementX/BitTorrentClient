#ifndef TRACK_STATE_H
#define TRACK_STATE_H

#include "b_parser/bencoder.h"

class TState {
private:
	Parser p{}; 

	//probably add file manager aswell

	//t_file fields
	std::string t_file_path; 
	std::shared_ptr<Bencode> t_file; 
	std::map<std::string, std::shared_ptr<Bencode>> t_file_dict;

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

	void make_info_hash(); 
	void make_peer_id(); 

public: 
	TState(std::string t_file_path); 

	std::string get_info_hash(); 
	std::string get_peer_id(); 

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
}; 

#endif
