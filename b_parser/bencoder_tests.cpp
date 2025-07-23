#include "bencoder.h"

int main(int argc, char* argv[]) {
	if(argc != 2) 
		throw std::runtime_error("incorrect commandline args"); 

	std::string path(argv[1]); 
	Parser test{}; 

	test.load_file(path); 
	std::shared_ptr<Bencode> parsed = test.parse_file(); 
	test.print_file(parsed); 
	return 0; 
}
