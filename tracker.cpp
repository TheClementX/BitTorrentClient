size_t curl_callback(void* contents, size_t size, size_t nmemb, void* out) {
	(std::string*)out->append((char*)out, size*nmemn); 
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

	if(strip.find(':') != std::string::npos) {
		this->dns = strip.substr(7, strip.find(':')); 
		this->port = strip.substr(strip.find(':'), strip.find('/')); 	
	} else {
		this->dns = to_parse.substr(0, to_parse.find('/')); 
		this->port = ""; 
	}
}

void Tracker::parse_response() {
		std::string to_parse = this->last_response; 
		this->t_parser.load_string(to_parse); 
		this->peer_info = t_parser.parse_file(); 
		this->peer_dict = 
			std::get<std::map<std::string, std::shared_ptr<Bencode>>>(this->peer_info); 
}

std::string Tracker::append_params(std::string url, std::vector<std::string> params) {
	CURL* curl = curl_easy_init(); 
	std::string result = url; 
	std::vector<std::string> encoded; 

	for(int i = 0; i < params.size; i++) {
		char* enc = 
			curl_easy_escape(curl, params[i].data(), params[i].size())
		std::string esc = enc; 
		curl_easy_free(enc); 
		encoded.push_back(esc); 	
	}

	url.append("info_hash="); url.append(encoded[0]); url.push_back('&'); 
	url.append("peer_id="); url.append(encoded[0]); url.push_back('&'); 
	url.append("port="); url.append(encoded[0]); url.push_back('&'); 
	url.append("uploaded="); url.append(encoded[0]); url.push_back('&'); 
	url.append("downloaded="); url.append(encoded[0]); url.push_back('&'); 
	url.append("left="); url.append(encoded[0]); url.push_back('&'); 
	url.append("compact="); url.append(encoded[0]);

	curl_easy_cleanup(curl); 
	return result; 
}

void Tracker::send_get() {
	CURL* curl = curl_easy_init(); 
	CURLcode res; 

	std::string response; 
	std::string url = this->url; 
	std::vector<std::string> params; 
	
	params.push_back(this->state.get_info_hash()); 
	params.push_back(this->state.get_peer_id()); 
	params.push_back(std::to_string(this->state.get_port())): 
	params.push_back(std::to_string(this->state.get_uploaded())); 
	params.push_back(std::to_string(this->state.get_downloaded()));
	params.push_back(std::to_string(this->state.get_left())); 
	params.push_back(std::to_string(this->state.get_compact())); 

	url = this->append_params(url, params); 
	curl_easy_setopt(curl, CURLOPT_URL, url.data()); 
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response.data()); 
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 10L); 

	res = curl_easy_perfrom(); 
	if(res != CURL_OK)
		throw std::runtime_error("curl http request failed"); 

	curl_easy_cleanup(curl); 
	this->last_response = response; 
}

Tracker::Tracker(std::string url, std:shared_ptr<Bencode> t_file, TState state) {
	this->state = state; 
	this->url = url; 
	this->parse_url(url); 
	con_interval = -1; 
	bencode_peers = ""; 
	this->t_file = t_file; 
	this->uploaded = 0; 
	this->downloaded = 0; 
}

void get_peers() {
	this->send_get(); 
	this->parse_response(); 

	//get interval info
	int tri = static_cast<int>(std::get<int64_t>(this->peer_dict["interval"])); 
	this->state.set_tri(tri); 
	
	//get peer info
	std::vector<std::shared_ptr<Bencode>> p_list = 
		std::get<std::vector<std::shared_ptr<Bencode>>>(this->peer_dict["peers"]);
	
	for(int i = 0; i < p_list.size(); i++) {
		std::map<std::string, std::shared_ptr<Bencode>> cur_peer = 
			std::get<std::map<std::string, std::shared_ptr<Bencode>>>(p_list[i]); 

		std::string peer_id = std::get<std::string>(cur_peer["peer id"]); 
		std::string ip = std::get<std::string>(cur_peer["ip"]); 
		int port = static_cast<int>(std::get<int64_t>(cur_peer["port"]));  
		std::shared_ptr<Peer> to_push = std::make_shared<Peer>(peer_id, ip, port); 
		this->peers.push_back(to_push); 
	}
}

