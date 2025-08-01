#ifndef TRACKER_H
#define TRACKER_H

#include "b_parser/bencoder.h"
#include <curl/curl.h>
#include "track_state.h"
#include <cstddef>
#include <string>
#include <map>
#include <vector>
#include <memory> 
#include <stdexcept>
#include <cstdint>
#include <util/parts.h>

size_t curl_callback(void* contents, size_t size, size_t nmemb, void* out); 


class Tracker {
private:
	//parser
	Parser t_parser{}; 
	std::shared_ptr<TState> state; 

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
	Tracker(std::shared_ptr<TState> state); 

	//assumes not compact response
	std::vector<std::shared_ptr<Peer>> get_peers(); 
}; 

#endif
