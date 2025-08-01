#include "connections.h"
#include "file_man.h"
#include "track_state.h"
#include "tracker.h"
#include <chrono>

int main(int argc, char* argv[]) {
	if(argc != 2)
		std::cout << "b_client <path/to/.torrent>" << std::endl; 
	std::string path(argv[1]); 

	//open managerial objects
	std::shared_ptr<TState> t_state = std::make_shared<TState>(path); 
	Tracker tracker{t_state}; 
	PieceManager file_man{t_state}; 
	ConnectionManager connections{tracker.get_peers(), t_state}; 

	//serv up and set necessary fields
	connections.set_file_bits(file_man.get_file_bits()); 
	connections.start_server_sock(); 
	
	//main loop
	auto start_t = std::chrono::steady_clock::now(); 
	while(true) {
		std::vector<std::shared_ptr<BlockReq>> reqs = file_man.choose_next_blocks(); 
		for(auto r : reqs)
			connections.push_block_req(r); 

		connections->handle_peer_cycle(); 
		std::stack<std::shared_ptr<RecBlock>> rec = connections.get_recieved_blocks(); 

		while(rec.size() > 0)
			file_man.write_bloc(rec.pop()); 
		
		if(file_man.check_complete()) {
			file_man.write_file(); 
			std::cout << "file download complete" << std::endl; 
			break; 
		}
		auto end_t = std::chrono::steady_clock::now(); 
		int elapsed = duration_cast<seconds>(end_t - start_t).count(); 
		if(elapsed >= t_state->get_tri()) {
			start_t = end_t; 
			connections.set_peers(tracker.get_peers()); 
			connections.refresh_peers(); 
		}
	}
	
	connections.connection_cleanup(); 
	return 0; 
}
