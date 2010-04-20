
//*********************************************************************
//* C_Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*********************************************************************

#ifndef _Base64_hpp
#define _Base64_hpp

#include "Encoder.h"

class Base64 : public Encoder
{
private:
	binaryblock bin;
public:
	Base64();
	virtual std::string Encode(const std::string &data);
	virtual std::string Decode(const std::string &data);
	virtual binaryblock&DecodeBin(const std::string &data);
	virtual ~Base64();
};

static const char fillchar = '=';

// 0000000000111111111122222222223333333333444444444455555555556666
// 0123456789012345678901234567890123456789012345678901234567890123
static const std::string Base64Table; //("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

static const std::string::size_type np = std::string::npos;
	// Decode Table gives the index of any valid base64 character in the Base64 table]
	// 65 == A, 97 == a, 48 == 0, 43 == +, 47 == /
                                               // 0  1  2  3  4  5  6  7  8  9 
const std::string::size_type DecodeTable[] = {np,np,np,np,np,np,np,np,np,np,  // 0 - 9
											  np,np,np,np,np,np,np,np,np,np,  //10 -19
											  np,np,np,np,np,np,np,np,np,np,  //20 -29
											  np,np,np,np,np,np,np,np,np,np,  //30 -39
											  np,np,np,62,np,np,np,63,52,53,  //40 -49
											  54,55,56,57,58,59,60,61,np,np,  //50 -59
											  np,np,np,np,np, 0, 1, 2, 3, 4,  //60 -69
											  5, 6, 7, 8, 9,10,11,12,13,14,  //70 -79
											  15,16,17,18,19,20,21,22,23,24,  //80 -89
											  25,np,np,np,np,np,np,26,27,28,  //90 -99
											  29,30,31,32,33,34,35,36,37,38,  //100 -109
											  39,40,41,42,43,44,45,46,47,48,  //110 -119
											  49,50,51,np,np,np,np,np,np,np,  //120 -129
											  np,np,np,np,np,np,np,np,np,np,  //130 -139
											  np,np,np,np,np,np,np,np,np,np,  //140 -149
											  np,np,np,np,np,np,np,np,np,np,  //150 -159
											  np,np,np,np,np,np,np,np,np,np,  //160 -169
											  np,np,np,np,np,np,np,np,np,np,  //170 -179
											  np,np,np,np,np,np,np,np,np,np,  //180 -189
											  np,np,np,np,np,np,np,np,np,np,  //190 -199
											  np,np,np,np,np,np,np,np,np,np,  //200 -209
											  np,np,np,np,np,np,np,np,np,np,  //210 -219
											  np,np,np,np,np,np,np,np,np,np,  //220 -229
											  np,np,np,np,np,np,np,np,np,np,  //230 -239
											  np,np,np,np,np,np,np,np,np,np,  //240 -249
											  np,np,np,np,np,np               //250 -256
};


#endif // _Base64_hpp
