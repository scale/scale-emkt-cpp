
// *********************************************************************
// * Base64 - a simple base64 encoder and decoder.
// *
// *     Copyright (c) 1999, Bob Withers - bwit@pobox.com
// *
// * This code may be freely used for any purpose, either personal
// * or commercial, provided the authors copyright notice remains
// * intact.
// *
// * Enhancements by Stanley Yamane:
// *     o reverse lookup table for the decode function 
// *     o reserve string buffer space in advance
// *
// *********************************************************************

#include "Base64.h"

Base64::Base64() {
}

Base64::~Base64() {
}

std::string 
Base64::Encode(const std::string& data)
{
    std::string::size_type  i;
    char               c;
    std::string::size_type  len = data.length();
    std::string             ret;

	ret.reserve(len * 2);

    for (i = 0; i < len; ++i)
    {
        c = (data[i] >> 2) & 0x3f;
        ret.append(1, Base64Table[c]);
        c = (data[i] << 4) & 0x3f;
        if (++i < len)
            c |= (data[i] >> 4) & 0x0f;

        ret.append(1, Base64Table[c]);
        if (i < len)
        {
            c = (data[i] << 2) & 0x3f;
            if (++i < len)
                c |= (data[i] >> 6) & 0x03;

            ret.append(1, Base64Table[c]);
        }
        else
        {
            ++i;
            ret.append(1, fillchar);
        }

        if (i < len)
        {
            c = data[i] & 0x3f;
            ret.append(1, Base64Table[c]);
        }
        else
        {
            ret.append(1, fillchar);
        }
    }

    return(ret);
}

std::string 
Base64::Decode(const std::string& data)
{
    std::string::size_type  i;
    char               c;
    char               c1;
    std::string::size_type  len = data.length();
    std::string             ret;

	ret.reserve(len);

    for (i = 0; i < len; ++i)
    {
        c = (char) DecodeTable[(unsigned char)data[i]];
        ++i;
        c1 = (char) DecodeTable[(unsigned char)data[i]];
        c = (c << 2) | ((c1 >> 4) & 0x3);
        ret.append(1, c);
        if (++i < len)
        {
            c = data[i];
            if (fillchar == c)
                break;

            c = (char) DecodeTable[(unsigned char)data[i]];
            c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
            ret.append(1, c1);
        }

        if (++i < len)
        {
            c1 = data[i];
            if (fillchar == c1)
                break;

            c1 = (char) DecodeTable[(unsigned char)data[i]];
            c = ((c << 6) & 0xc0) | c1;
            ret.append(1, c);
        }
    }

    return(ret);
}

binaryblock &Base64::DecodeBin(const std::string &data)
{
    std::string::size_type  i;
    char               c;
    char               c1;
	char *			   buf;
	char *			   pbuf;
    std::string::size_type len = data.length();

	//ret.rdbuf()->setbuf(0,len);

	//of.clear();
	//of.freeze(false);

	buf = new char[len+1];
	memset(buf, 0, len+1);
	pbuf = buf;

    for (i = 0; i < len; ++i)
    {
        c = (char) DecodeTable[(unsigned char)data[i]];
        ++i;
        c1 = (char) DecodeTable[(unsigned char)data[i]];
        c = (c << 2) | ((c1 >> 4) & 0x3);
        //of << c;
		*pbuf++ = c;
        if (++i < len)
        {
            c = data[i];
            if (fillchar == c)
                break;

            c = (char) DecodeTable[(unsigned char)data[i]];
            c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
            //of << c1;
			*pbuf++ = c1;
        }

        if (++i < len)
        {
            c1 = data[i];
            if (fillchar == c1)
                break;

            c1 = (char) DecodeTable[(unsigned char)data[i]];
            c = ((c << 6) & 0xc0) | c1;
            //of << c;
			*pbuf++ = c;
        }
    }
	//of << '\0';

    bin << buf; //of.str();

	delete[] buf;
	return bin;
}
