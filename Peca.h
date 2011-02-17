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
	Peca() : ativa(true) { debug = new Debug("Peca"); };
	virtual ~Peca() { delete debug; };
	bool isValida();

	std::string from;
	std::string fromName;

	std::string subject;
	std::string html;
	std::string txt;

	bool ativa;

	int pecaId;
	int campanhaId;

	Debug *debug;

	bool operator == (Peca & other)
		{
			return campanhaId == other.campanhaId && pecaId == other.pecaId;
		}
};

#endif /* PECA_H_ */
