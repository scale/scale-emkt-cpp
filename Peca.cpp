/*
 * Peca.cpp
 *
 *  Created on: 24/10/2010
 *      Author: Flavio
 */

#include "Peca.h"

Peca::Peca() {

}

Peca::~Peca() {

}

bool Peca::isValida() {
	if( html.length() < 10 ) {
		debug.error("ERRO: Body HTML invalido.");
		return false;
	}
	if( subject.length() < 3 ) {
		debug.error("ERRO: Subject invalido? {%s}", subject.c_str());
		return false;
	}
}
