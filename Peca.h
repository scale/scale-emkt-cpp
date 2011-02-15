/*
 * Peca.h
 *
 *  Created on: 24/10/2010
 *      Author: Flavio
 */

#ifndef PECA_H_
#define PECA_H_

#include "Debug.h"
#include <string>

class Peca {
public:
	Peca();
	virtual ~Peca();
	bool isValida();

	std::string from;
	std::string subject;
	std::string html;
	std::string txt;

	int pecaId;
	int campanhaId;

	Debug debug;
};

#endif /* PECA_H_ */
