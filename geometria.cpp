#include "geometria.h"
#include <cmath>

float calcolaDistanza(const Citta& a, const Citta& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

float calcolaPeso(const Citta& a, const Citta& b) {
    return calcolaDistanza(a, b);
}