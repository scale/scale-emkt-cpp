// binaryblock.cpp: implementation of the binaryblock class.
//
//////////////////////////////////////////////////////////////////////
//#include "main.h"
#include "binaryblock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

binaryblock::binaryblock() : data(0), end(0), aloc_size(0)
{

}

binaryblock::~binaryblock()
{
	if (data) {
		delete[] data;
		data = NULL;
		end = NULL;
	}
}

size_t
binaryblock::offset() 
{
	return (end - data);
}

binaryblock & 
binaryblock::operator << (binaryblock &b) {
	bool error = false;

	if (!data || !end) 
		if (!alloc(b.size())) error = true;
	if (aloc_size - (end - data) < b.size() ) 
		if (!realloc(aloc_size + b.size())) error = true;

	if (!error) {
		memcpy(end, b.c_data(), b.size() );
		end += (b.offset());
	}
	return *this;
}

bool binaryblock::realloc(size_t novo_size) {
	char *temp;
	size_t new_size = 2*novo_size + 1;

	temp = new char[new_size+1];

	if (temp) {
		memset(temp, 0, new_size+1);
		memcpy(temp, data, aloc_size);
		if (data)
			delete[] data;
		data = temp;
		end = data + new_size;
		aloc_size = new_size+1;
		return true;
	} else
		return false;
}

bool binaryblock::alloc(size_t initial_size) {
	size_t the_size = 2*initial_size + 1;

	data = new char[the_size+1];

	if (!data) return false;
	memset(data, 0, the_size+1);
	aloc_size = the_size+1;
	end = data;

	return true;
}

binaryblock & 
binaryblock::operator << (const char *p) {
	bool error = false;
	if (!data || !end) 
		if (!alloc(strlen(p))) error = true;
	if (aloc_size - (end - data) < strlen(p) ) 
		if (!realloc(aloc_size + strlen(p)))
			error = true;

	if (!error) {
		memcpy(end, p, strlen(p) );
		end += strlen(p);
	}
	return *this;
}


binaryblock & 
binaryblock::operator += (binaryblock &b) {
	return *this << b;
}

binaryblock & 
binaryblock::operator += (const char *p) {
	return *this << p;
}


binaryblock & 
binaryblock::operator + (binaryblock &b) {
	return *this << b;
}

binaryblock & 
binaryblock::operator + (const char *p) {
	return this->operator << (p);
}


void binaryblock::clear() {
	memset(data, 0, aloc_size);
	end = data;
}
