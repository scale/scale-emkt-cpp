// QuotedPrintable.cpp: implementation of the QuotedPrintable class.
//
//////////////////////////////////////////////////////////////////////

#include "QuotedPrintable.h"
#include <iostream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QuotedPrintable::QuotedPrintable()
{

}

QuotedPrintable::~QuotedPrintable()
{

}

std::string 
QuotedPrintable::Encode(const std::string& data)
{
	static const std::string Q("?=!\"#$@[\\]^`{|}~");
	std::string Result;
    int Len=0;
	unsigned char b,B;
	unsigned char anterior;
	const char *S = data.c_str();

    while(b=*S++) {

      if(b == '\n'){
	anterior = *S-1;
	if(anterior != '\r'){
          Result+='\r';
          Result+=b;
          Len++;
          Len++;
	}
      } else {

        if(b>127 || (Q.find(b) != std::string::npos)
          || (!*S && (b==' ' || b=='\t'))) {//Preserve trailing spaces
          B=b>>4;
          b&=0x0F;
          B=(B>9) ? B-10+'A' : B+'0';
          b=(b>9) ? b-10+'A' : b+'0';
          char Buffer[3];
          char* ptr=Buffer;
          *ptr++=B;
          *ptr++=b;
          *ptr  =0;
          Result=Result+'='+ Buffer;
        }else{
          Result+=b;
          Len++;
        }
      }

      if((Len==75) && b!='\r' && b!='\n') {
        Len=0;
        Result+="=\r\n"; //Soft break for long lines
      }
	}
    return Result; //May append a NL at EOF
}

std::string 
QuotedPrintable::Decode(const std::string& S)
{
	std::string Result;
	unsigned char b,B;

    char *DST = new char[S.length()+3]; // Result.GetBufferSetLength(strlen(S)+3);
	char *dst = DST;

    if(!S.empty()) {
      const char* src=S.c_str();
      while(b=*src++) {
        if(b=='=') {
          if(b=*src++) {
            b|=0x20;
            B=(*src++)|0x20;
            b=(b>'9') ? b-'a'+10 : b-'0';
            B=(B>'9') ? B-'a'+10 : B-'0';
            b=(b<<4)|B;
          }else{ //Soft Break
            *dst=0;
            // Result = DST;
            return DST; //Result;
          }
		}
        *dst++=b;
	  }
    }
    *dst++='\r';
    *dst++='\n';
    *dst=0;
    // Result = DST
    return DST; // Result;
}
