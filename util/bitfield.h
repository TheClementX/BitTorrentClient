#ifndef BITFIELD_H
#define BITFIELD_H

#include <cstdint>

class BitField {
private:
	int size; 
	std::vector<uint8_t> bits; 

public: 
	bool BitField(int size); 

	void get_value(int i); 
	void set_true(int i); 
	void set_false(int i); 
	void toggle(int i); 
}; 

#endif
