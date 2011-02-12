/*
 * Peca.cpp
 *
 *  Created on: 24/10/2010
 *      Author: Fabiana
 */

#include "Peca.h"

Peca::Peca() {

}

Peca::~Peca() {

}

bool Peca::isValida() {
	if( strlen(html.c_str()) < 10 ) {
		debug.error("ERRO: Body HTML invalido."); return false;
	}
	if( strlen(subject.c_str()) < 3 ) {
		debug.error("ERRO: Subject invalido? {%s}", subject.c_str()); return false;
	}
}
