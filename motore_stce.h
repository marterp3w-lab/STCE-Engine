#ifndef MOTORE_STCE_H
#define MOTORE_STCE_H

#include "strutture.h"
#include <vector>

void costruisciMappaIntelligente(Citta* citta, int N, std::vector<Triangolo>& universo, std::vector<Citta*>& orfane);
int trovaIndiceCatalizzatore(Citta* citta, int N, std::vector<Triangolo>& universo);

// Modificati per clonazione dati (niente puntatori vector) e correttezza accademica O(n)
void dividiEmisferi(Triangolo* catalizzatore, std::vector<Triangolo>& universo, std::vector<Triangolo>& emisfero_Sinistro, std::vector<Triangolo>& emisfero_Destro);
Rotta ricercaGreedyLocale(Citta* posizione_attuale, std::vector<Triangolo>& emisfero_privato);
Citta* espandiEmisfero(Citta* partenza, std::vector<Triangolo>& emisfero_privato);

void eseguiScambio2Opt(std::vector<Citta*>& tour, int i, int k);
void ottimizzaCon2Opt(std::vector<Citta*>& tour);

#endif