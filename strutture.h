#ifndef STRUTTURE_H
#define STRUTTURE_H

#include <string>

struct Citta {
    int id;
    std::string nome;
    float x, y;
    Citta* prossimo;
};

struct Triangolo {
    Citta* vertici[3];
    bool visitato;
};

struct Rotta {
    float peso;
    Triangolo* cluster;
    int vertice_scelto;
};

#endif