// QuotedPrintable.h: interface for the QuotedPrintable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUOTEDPRINTABLE_H__F17C7955_6630_4252_97B1_14F5058BADB4__INCLUDED_)
#define AFX_QUOTEDPRINTABLE_H__F17C7955_6630_4252_97B1_14F5058BADB4__INCLUDED_

#include "Encoder.h"

class QuotedPrintable : public Encoder  
{
private:
	binaryblock bin;
public:
	QuotedPrintable();
	virtual std::string Encode(const std::string &data);
	virtual std::string Decode(const std::string &data);
	virtual binaryblock &DecodeBin(const std::string &data) { bin << of.str().c_str(); return bin;};
	virtual ~QuotedPrintable();

};

#endif // !defined(AFX_QUOTEDPRINTABLE_H__F17C7955_6630_4252_97B1_14F5058BADB4__INCLUDED_)
