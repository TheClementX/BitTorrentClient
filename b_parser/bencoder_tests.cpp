#include "bencoder.h"

int main(int argc, char* argv[]) {
	if(argc != 2) 
		throw std::runtime_error("incorrect commandline args"); 

	std::string path(argv[1]); 
	Parser test{}; 

	//file loading 
	test.load_file(path); 
	std::shared_ptr<Bencode> parsed = test.parse_file(); 
	std::cout << "file parsed from path" << std::endl; 
	test.print_file(parsed); 

	//string encoding
	std::string encoded = test.encode(parsed); 
	std::cout << "encoded file string" << std::endl; 
	std::cout << encoded << std::endl; 

	//string loading 
	test.load_string(encoded); 
	std::shared_ptr<Bencode> parsed2 = test.parse_file(); 
	std::cout << "parsed file from string" << std::endl; 
	test.print_file(parsed2); 

	//loading fields from the torrent
	std::map<std::string, std::shared_ptr<Bencode>> file = 
		std::get<std::map<std::string, std::shared_ptr<Bencode>>>(parsed->val); 
	test.print_file(file["announce"]); 
	return 0; 
}
