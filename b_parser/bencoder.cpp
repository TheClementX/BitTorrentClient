#include "bencoder.h"

Parser::Parser() : file_path(""), cur_pos(0), content("") {}

void Parser::load_file(std::string& path) {
	this->cur_pos = 0; 
	this->file_path = path; 

	std::ifstream file(this->file_path, std::ios::binary); 
	if(!file) 
		throw std::runtime_error("could not read .torrent file"); 

	file.seekg(0, std::ios::end); 
	std::streamsize size = file.tellg(); 
	file.seekg(0, std::ios::beg); 
	
	this->content = std::string(size, '\0'); 
	if(!file.read(this->content.data(), size)) {
		throw std::runtime_error("read function error"); 
	}
}

void Parser::load_string(std::string& to_load) {
	this->file_path = ""; 
	this->cur_pos = 0; 
	this->content = to_load; 
}

std::shared_ptr<Bencode> Parser::parse_file() {
	std::shared_ptr<Bencode> result; 

	char cur = this->content[cur_pos]; 
	if(cur == 'i') {
		result = this->parse_int();  
	} else if (cur == 'l') {
		result = this->parse_list(); 
	} else if (cur == 'd') {
		result = this->parse_dict(); 
	} else if (isdigit(cur)) {
		result = this->parse_string(); 
	} else {
		throw std::runtime_error("invalid Bencoding parse failed");
	}
	return result; 
}

std::shared_ptr<Bencode> Parser::parse_int() {
	this->cur_pos++; 

	int64_t parsed; 	
	std::string si; 

	while(this->content[this->cur_pos] != 'e') {
		si.push_back(this->content[this->cur_pos]); 
		this->cur_pos++; 
	}
	this->cur_pos++; 
	
	parsed = std::stoll(si);

	std::shared_ptr<Bencode> result = std::make_shared<Bencode>();
	result->val = parsed; 
	return result; 
}

std::shared_ptr<Bencode> Parser::parse_string() {
	std::string slen; 
	std::string sresult; 
	int len; 

	while(this->content[this->cur_pos] != ':') {
		slen.push_back(this->content[this->cur_pos]); 
		this->cur_pos++; 
	}
	this->cur_pos++; 

	len = std::stoi(slen); 

	for(int i = 0; i < len; i++) {
		sresult.push_back(this->content[this->cur_pos]); 
		this->cur_pos++; 
	}
	
	std::shared_ptr<Bencode> result = std::make_shared<Bencode>(); 
	result->val = sresult; 
	return result; 
}

std::shared_ptr<Bencode> Parser::parse_list() {
	this->cur_pos++; 

	std::vector<std::shared_ptr<Bencode>> vresult; 
	while(this->content[this->cur_pos] != 'e') {
		vresult.push_back(this->parse_file()); 
	}
	this->cur_pos++; 

	std::shared_ptr<Bencode> result = std::make_shared<Bencode>(); 
	result->val = vresult; 
	return result; 
}

std::shared_ptr<Bencode> Parser::parse_dict() {
	this->cur_pos++; 

	std::map<std::string, std::shared_ptr<Bencode>> mresult; 
	while(this->content[this->cur_pos] != 'e') {
		std::shared_ptr<Bencode> key = this->parse_string(); 
		std::string skey = std::get<std::string>(key->val); 
		std::shared_ptr<Bencode> val = this->parse_file(); 
		mresult[skey] = val; 
	}
	this->cur_pos++; 

	std::shared_ptr<Bencode> result = std::make_shared<Bencode>();
	result->val = mresult; 
	return result; 
}


void Parser::print_file(std::shared_ptr<Bencode> to_print) {
	this->print_help(to_print, 0); 
}


void Parser::print_help(std::shared_ptr<Bencode> to_print, int depth) {
	std::string offset(depth, ' '); 

	if(std::holds_alternative<int64_t>(to_print->val)) {
		std::cout << offset << std::get<int64_t>(to_print->val) <<std::endl; 

	} else if(std::holds_alternative<std::string>(to_print->val)) {
		std::cout << offset << std::get<std::string>(to_print->val) <<std::endl; 

	} else if(std::holds_alternative<std::vector<std::shared_ptr<Bencode>>>(to_print->val)) {
		std::vector<std::shared_ptr<Bencode>> vec = 
			std::get<std::vector<std::shared_ptr<Bencode>>>(to_print->val); 
		int n = vec.size(); 

		std::cout << offset << "list:" << std::endl; 
		for(int i = 0; i < n; i++) 
			this->print_help(vec[i], depth + 1); 	

	} else if(std::holds_alternative<std::map<std::string, std::shared_ptr<Bencode>>>(to_print->val)) {
		std::map<std::string, std::shared_ptr<Bencode>>	mp = 
			std::get<std::map<std::string, std::shared_ptr<Bencode>>>(to_print->val); 

		//finish print implementation here
		std::cout << offset << "dict:" << std::endl; 
		depth += 1; 
		std::string doffset = std::string(depth, ' '); 
		for(auto it = mp.begin(); it != mp.end(); it++) {
			std::cout << doffset << it->first << " : "; 
			this->print_help(it->second, depth + 1); 
		}
	} else {
		throw std::runtime_error("invalid Bencode variant passed to print"); 
	}
}

std::string Parser::encode(std::shared_ptr<Bencode> to_encode) {
	if(std::holds_alternative<int64_t>(to_encode->val)) 
		return this->encode_int(to_encode); 
	else if(std::holds_alternative<std::string>(to_encode->val)) 
		return this->encode_string(to_encode); 
	else if(std::holds_alternative<std::vector<std::shared_ptr<Bencode>>>(to_encode->val)) 
		return this->encode_list(to_encode); 
	else if(std::holds_alternative<std::map<std::string, std::shared_ptr<Bencode>>>(to_encode->val)) 
		return this->encode_dict(to_encode); 
	else 
		throw std::runtime_error("invalid object passed to encoder"); 
}


std::string Parser::encode_int(std::shared_ptr<Bencode> to_encode) {
	std::string result; 

	result.push_back('i'); 
	std::string i = std::to_string(std::get<int64_t>(to_encode->val)); 
	result.append(i); 
	result.push_back('e'); 

	return result; 
}

std::string Parser::encode_string(std::shared_ptr<Bencode> to_encode) {
	std::string result; 

	std::string s = std::get<std::string>(to_encode->val); 
	std::string len = std::to_string(s.size()); 
	result.append(len); 
	result.push_back(':'); 
	result.append(s); 

	return result;
}

std::string Parser::encode_list(std::shared_ptr<Bencode> to_encode) {
	std::string result; 

	std::vector<std::shared_ptr<Bencode>> v = std::get<std::vector<std::shared_ptr<Bencode>>>(to_encode->val); 
	result.push_back('l'); 
	for(int i = 0; i < v.size(); i++) 
		result.append(this->encode(v[i])); 
	result.push_back('e'); 	

	return result; 
}

std::string Parser::encode_dict(std::shared_ptr<Bencode> to_encode) {
	std::string result; 

	std::map<std::string, std::shared_ptr<Bencode>>> d = 
		std::get<std::map<std::string, std::shared_ptr<Bencode>>>(to_encode->val); 

	for(auto it = d.begin(); it != d.end(); it++) {
		result.append(it->first); 
		result.push_back(':'); 
		result.append(this->encode(it->second)); 
	}
	result->push_back('e'); 

	return result; 
}

