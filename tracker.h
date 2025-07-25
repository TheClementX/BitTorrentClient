#ifndef TRACKER_H
#define TRACKER_H

#include "b_parser/bencoder.h"
#include <curl/curl.h>
#include <openssl/sha.h>

size_t curl_callback(void* contents, size_t size, size_t nmemb, void* out); 

class Peer {
public:
	std::string peer_id; 
	std::string ip; 
	int port; 

	Peer(std::string pd, std::string ip, int port); 
}; 

class Tracker {
private:
	//parser
	Parser t_parser{}; 
	Tstate state; 

	//tracker url fields
	std::string url; 
	std::string dn; 
	std::string port;  
	std::string proto; 

	//encoded info and info
	std::string last_response; 
	std::shared_ptr<Bencode> peer_info; 
	std::map<std::string, std::shared_ptr<Bencode>> peer_dict; 
	std::vector<std::shared_ptr<Peer>> peers; 

	void parse_url(std::string to_parse); 
	void parse_response(); 

	std::string append_params(std::string url, 
			std::vector<std::string> params
	); 

	void send_get(); 

public: 
	Tracker(std::string url, 
			std:shared_ptr<Bencode> t_file, 
			TState state
	); 

	//assumes not compact response
	void get_peers(); 
}; 

#endif
