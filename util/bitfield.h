#ifndef BITFIELD_H
#define BITFIELD_H

#include <cstdint>

class BitField {
private:
	int size; 
	std::vector<uint8_t> bits; 

	uint8_t string_to_byte(std::string byte); 

public: 
	BitField(int size); 
	BitField(std::string field); 

	void get_value(int i); 
	void set_true(int i); 
	void set_false(int i); 
	void toggle(int i); 

	std::vector<uint8_t> get_bits; 
}; 

#endif
