// Encoder.h: interface for the Encoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENCODER_H__852AF05D_32F9_4269_B766_53212849C34F__INCLUDED_)
#define AFX_ENCODER_H__852AF05D_32F9_4269_B766_53212849C34F__INCLUDED_

#include <string>
//#include <strstream>
#include <sstream>
// using namespace std;

#include "binaryblock.h"

class Encoder  
{
public:
	
	Encoder();
	virtual std::string Encode(const std::string &data) = 0;
	virtual std::string Decode(const std::string &data) = 0;
	virtual binaryblock &DecodeBin(const std::string &data) = 0;
	virtual ~Encoder();

protected:
	std::ostringstream of;
};

#endif // !defined(AFX_ENCODER_H__852AF05D_32F9_4269_B766_53212849C34F__INCLUDED_)
