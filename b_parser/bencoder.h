#ifndef BENCODER_H
#define BENCODER_H

#include <vector>
#include <cstdint> 
#include <variant> 
#include <map> 
#include <fstream>
#include <string>
#include <memory> 
#include <iostream>
#include <stdexcept> 

/* A simple library for parsing single
 * element Bencoded .torrent files
 */

/* BENCODE DEFINITION: 
 * strings len:<data>
 * ints i<data>e
 * lists l<data>e
 *   -can contain any other data
 * dicts d<data>e
 *   -can contain any other data 
 *   -keys are always strings	
 */

struct Bencode; 

typedef std::variant<
	int64_t, 
	std::string, 
	std::vector<std::shared_ptr<Bencode>>, 
	std::map<std::string, std::shared_ptr<Bencode>>
>
BencodeValue; 

struct Bencode {
	BencodeValue val; 
}; 
	
class Parser {
	public: 
		Parser(); 

		void load_file(std::string& path); 
		std::shared_ptr<Bencode> parse_file(); 
		void print_file(std::shared_ptr<Bencode> to_print); 
		std::string encode(std::shared_ptr<Bencode> to_encode); //untested

	private: 
		std::string file_path; 
		int cur_pos; 
		std::string content; 

		std::shared_ptr<Bencode> parse_int(); 
		std::shared_ptr<Bencode> parse_string(); 
		std::shared_ptr<Bencode> parse_list(); 
		std::shared_ptr<Bencode> parse_dict(); 

		std::string encode_int(std::shared_ptr<Bencode> to_encode); 
		std::string encode_string(std::shared_ptr<Bencode> to_encode); 
		std::string encode_list(std::shared_ptr<Bencode> to_encode); 
		std::string encode_dict(std::shared_ptr<Bencode> to_encode); 

		void print_help(std::shared_ptr<Bencode> to_print, int depth); 

}; 

#endif
