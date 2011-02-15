// binaryblock.h: interface for the binaryblock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BINARYBLOCK_H__6F7ED2A4_9E99_41A4_98BE_9BFDC7E3BD06__INCLUDED_)
#define AFX_BINARYBLOCK_H__6F7ED2A4_9E99_41A4_98BE_9BFDC7E3BD06__INCLUDED_

#ifdef  __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef  __cplusplus
}
#endif


//typedef long unsigned int size_t;

class binaryblock  
{
private:
	char *data;
	char *end;
	size_t aloc_size;
	bool realloc(size_t new_size);
	bool alloc(size_t new_size);

public:
	binaryblock();
	virtual ~binaryblock();
	inline const char * c_data() const { return data; };
	inline size_t size() { return aloc_size; }
	binaryblock & operator << (binaryblock &b);
	binaryblock & operator << (const char *p);
	binaryblock & operator += (binaryblock &b);
	binaryblock & operator += (const char *p);
	binaryblock & operator +  (binaryblock &b);
	binaryblock & operator +  (const char *p);
	size_t offset();
	void clear();
};

#endif // !defined(AFX_BINARYBLOCK_H__6F7ED2A4_9E99_41A4_98BE_9BFDC7E3BD06__INCLUDED_)
