#include "geometria.h"
#include <cmath>

float calcolaDistanza(Citta A, Citta B) {
    return std::sqrt(std::pow(A.x - B.x, 2) + std::pow(A.y - B.y, 2));
}

float calcolaPeso(Citta attuale, Citta bersaglio) {
    // Distanza euclidea pura, nessuna penalità direzionale per mappe reali
    return calcolaDistanza(attuale, bersaglio);
}