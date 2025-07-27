#include "tracker.h"

size_t curl_callback(void* contents, size_t size, size_t nmemb, void* out) {
	size_t t_size = size*nmemb; 
	std::string* res = static_cast<std::string*>(out); 
	res->append(static_cast<char*>(contents), t_size); 
	return size*nmemb; 
}

Peer::Peer(std::string pd, std::string ip, int port) {
	this->peer_id = pd; 
	this->ip = ip; 
	this->port = port; 
}


void Tracker::parse_url(std::string to_parse) {
	this->proto = to_parse.substr(0, to_parse.find(':')); 
	std::string strip = to_parse.substr(7, to_parse.size()); 

	if(strip.find('/') == std::string::npos)
		strip.push_back('/'); 

	if(strip.find(':') != std::string::npos) {
		this->dn = strip.substr(7, strip.find(':')); 
		this->port = strip.substr(strip.find(':'), strip.find('/')); 	
	} else {
		this->dn = to_parse.substr(0, to_parse.find('/')); 
		this->port = ""; 
	}
}

void Tracker::parse_response() {
		std::string to_parse = this->last_response; 
		this->t_parser.load_string(to_parse); 
		this->peer_info = t_parser.parse_file(); 
		this->peer_dict = 
			std::get<std::map<std::string, std::shared_ptr<Bencode>>>(this->peer_info->val); 
}

std::string Tracker::append_params(std::string url, std::vector<std::string> params) {
	CURL* curl = curl_easy_init(); 
	std::string result = url; 
	result.push_back('?'); 
	std::vector<std::string> encoded; 

	for(int i = 0; i < params.size(); i++) {
		char* enc = 
			curl_easy_escape(curl, params[i].data(), params[i].size()); 
		std::string esc = enc; 
		curl_free(enc); 
		encoded.push_back(esc); 	
	}

	result.append("info_hash="); result.append(encoded[0]); result.push_back('&'); 
	result.append("peer_id="); result.append(encoded[0]); result.push_back('&'); 
	result.append("port="); result.append(encoded[0]); result.push_back('&'); 
	result.append("uploaded="); result.append(encoded[0]); result.push_back('&'); 
	result.append("downloaded="); url.append(encoded[0]); result.push_back('&'); 
	result.append("left="); result.append(encoded[0]); result.push_back('&'); 
	result.append("compact="); result.append(encoded[0]);

	curl_easy_cleanup(curl); 
	return result; 
}

void Tracker::send_get() {
	CURL* curl = curl_easy_init(); 
	CURLcode res; 

	std::string response; 
	std::string url = this->url; 
	std::vector<std::string> params; 
	
	params.push_back(this->state->get_info_hash()); 
	params.push_back(this->state->get_peer_id()); 
	params.push_back(std::to_string(this->state->get_port())); 
	params.push_back(std::to_string(this->state->get_uploaded())); 
	params.push_back(std::to_string(this->state->get_downloaded()));
	params.push_back(std::to_string(this->state->get_left())); 
	params.push_back(std::to_string(this->state->get_compact())); 

	url = this->append_params(url, params); 
	curl_easy_setopt(curl, CURLOPT_URL, url.data()); 
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response.data()); 
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 10L); 

	res = curl_easy_perform(curl); 
	if(res != CURLE_OK)
		throw std::runtime_error("curl http request failed"); 

	curl_easy_cleanup(curl); 
	this->last_response = response; 
}

Tracker::Tracker(std::shared_ptr<TState> state) {
	this->state = state; 
	this->url = this->state->get_tracker_url(); 
	this->parse_url(this->url); 
}

void Tracker::get_peers() {
	this->send_get(); 
	this->parse_response(); 

	//get interval info
	int tri = static_cast<int>(std::get<int64_t>(this->peer_dict["interval"]->val)); 
	this->state->set_tri(tri); 
	
	//get peer info
	std::vector<std::shared_ptr<Bencode>> p_list = 
		std::get<std::vector<std::shared_ptr<Bencode>>>(this->peer_dict["peers"]->val);
	
	for(int i = 0; i < p_list.size(); i++) {
		std::map<std::string, std::shared_ptr<Bencode>> cur_peer = 
			std::get<std::map<std::string, std::shared_ptr<Bencode>>>(p_list[i]->val); 

		std::string peer_id = std::get<std::string>(cur_peer["peer id"]->val); 
		std::string ip = std::get<std::string>(cur_peer["ip"]->val); 
		int port = static_cast<int>(std::get<int64_t>(cur_peer["port"]->val));  
		std::shared_ptr<Peer> to_push = std::make_shared<Peer>(peer_id, ip, port); 
		this->peers.push_back(to_push); 
	}
}

