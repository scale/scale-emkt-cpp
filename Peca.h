/*
 * Peca.h
 *
 *  Created on: 24/10/2010
 *      Author: Flavio
 */

#ifndef PECA_H_
#define PECA_H_

class Peca {
public:
	Peca();
	virtual ~Peca();
	bool isValida();

	string from;
	string subject;
	string html;
	string txt;

	int pecaId;
	int campanhaId;

	Debug debug;
};

#endif /* PECA_H_ */
