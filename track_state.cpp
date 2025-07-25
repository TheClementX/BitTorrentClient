TState::TState(std::string t_file_path) {
	this->t_file_path = t_file_path; 
	this->p.load_file(this->t_file_path); 
	this->t_file = this->p.parse_file(); 
	this->t_file_dict = 
		std::get<std::map<std::string, std::shared_ptr<Bencode>>>(this->t_file->val); 

	this->tracker_url =	
		std::get<std::string>(this->t_file_dict["announce"]); 
	this->make_info_hash(); 
	this->make_peer_id(); 
	this->uploaded = 0; 
	this->downloaded = 0;
	this->left = 0; 
	this->compact = 0; 
	this->port = 0; 
}

void TState::make_info_hash() {
	std::string info = p->encode(t_file_dict["info"]); 
	std::string hash(20, '\0'); 
	SHA1(info.data(), info.size(), hash.data()); 
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

std::string TState::get_info_hash() {
	return this->info_hash; 
}

std::string TState::get_peer_id() {
	return this->peer_id;
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
	if(n != 1 || n != 0)
		return; 
	this->compact = n; 
}

void TState::get_compact() {
	return this->compact; 
}

void TState::set_tri(int n) {
	this->tracker_renew_interval = n; 
}

int TState::get_tri() {
	return this->tracker_renew_interval = n; 
}
