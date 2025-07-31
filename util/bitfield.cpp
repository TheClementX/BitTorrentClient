BitField::BitField(int size) : size(size) {
	this->bits.resize(size); 
	this->bits.fill(this->bits.begin(), this->bits.end(), 0); 
}

bool BitField::get_value(int i) {
	if(i > this->bits.size()) return; 
	int bucket = i / 8; 
	int offset = i % 8; 
	uint8_t mask = 1 << (7 - offset); 

	if((this->bits[bucket] & mask) > 0)
		return true; 
	return false; 
}

void BitField::set_true(int i) {
	if(i > this->bits.size()) return; 
	int bucket = i / 8; 
	int offset = i % 8; 
	
	uint8_t mask = 1 << (7 - offset); 
	this->bits[bucket] |= mask; 
}

void BitField::set_false(int i) {
	if(i > this->bits.size()) return; 
	int bucket = i / 8; 
	int offset = i % 8; 

	uint8_t mask = 0xff; 
	mask ^= 1 << (7 - offset); 

	this->bits[bucket] &= mask; 
}

void BitField::toggle(int i) {
	if(i > this->bits.size()) return; 
	int bucket = i / 8; 
	int offset = i % 8; 

	uint8_t mask = 1 << (7 - offset); 
	this->bits[bucket] ^= mask; 
}
