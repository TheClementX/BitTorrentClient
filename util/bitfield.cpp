BitField::BitField(int size) : size(size) {
	this->bits.resize(std::ceil(size / 8)); 
	this->bits.fill(this->bits.begin(), this->bits.end(), 0); 
}

BitField::BitField(std::string field) {
	int buckets = std::ceil(field.size() / 8); 
	this->bits.resize(buckets); 
	this->bits.fill(this->bits.begin(), this->bits.end(), 0); 
	
	for(int i = 0; i < buckets; i++) {
		std::string s_byte = field.substr(0, 8); 
		field = field.substr(8); 
		uint8_t i_byte = string_to_byte(s_byte); 
		this->bits[i] = i_byte; 
	}
}

//low is 0 high is len
uint8_t BitField::string_to_byte(std::string byte) {
	uint8_t result; 
	for(int i = 0; i < byte.size(); i++) {
		result |= (1 << (7 - i))	
	}
	return result; 
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

std::vector<uint8_t> BitField::get_bits() {
	return this->bits; 
}
