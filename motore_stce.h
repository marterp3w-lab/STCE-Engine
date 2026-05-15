#ifndef MOTORE_STCE_H
#define MOTORE_STCE_H

#include "strutture.h"
#include <vector>

void costruisciMappaIntelligente(Citta* citta, int N, std::vector<Triangolo>& universo, std::vector<Citta*>& orfane);
int trovaIndiceCatalizzatore(Citta* citta, int N, std::vector<Triangolo>& universo);
void dividiEmisferi(Triangolo* catalizzatore, std::vector<Triangolo>& universo, std::vector<Triangolo>& emisfero_Sinistro, std::vector<Triangolo>& emisfero_Destro);
Citta* espandiEmisfero(Citta* partenza, std::vector<Triangolo>& emisfero_privato);
void ottimizzaCon2Opt(std::vector<Citta*>& tour);

#endif