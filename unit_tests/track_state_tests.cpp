#include "../track_state.h"
#include <cassert>

int main(int argc, char* argv[]) {
	if(argc != 2) 
		throw std::runtime_error("invalid arguments"); 
	
	std::string path{argv[1]}; 
	TState state{path}; 
	std::cout << "state initialization done" << std::endl; 

	std::cout << "info hash\n" << state.get_info_hash() << std::endl; 
	std::cout << "peer_id\n" << state.get_peer_id() << std::endl; 
	std::cout << "tracker url\n" << state.get_tracker_url() << std::endl; 

	assert(state.get_uploaded() == 0); 
	assert(state.get_downloaded() == 0); 
	assert(state.get_left() == 0); 
	assert(state.get_compact() == 0); 
	assert(state.get_port() == 0); 
	assert(state.get_tri() == 0); 

	std::cout << "constructor initialization asserts passed" << std::endl; 

	state.set_uploaded(10); 
	state.set_downloaded(10); 
	state.set_left(10); 
	state.set_port(10); 
	state.set_compact(1); 
	state.set_tri(10); 
	
	assert(state.get_uploaded() == 10); 
	assert(state.get_downloaded() == 10); 
	assert(state.get_left() == 10); 
	assert(state.get_compact() == 1); 
	assert(state.get_port() == 10); 
	assert(state.get_tri() == 10); 

	std::cout << "set method asserts passed" << std::endl; 

	std::cout << "printing torrent file fields" << std::endl; 
	std::cout << "----------------------------------------------------------" << std::endl; 
	state.print_t_file(); 	
	std::cout << "----------------------------------------------------------" << std::endl;  

	return 0; 
}
