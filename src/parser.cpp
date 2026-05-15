#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

Citta* caricaMappaTSPLIB(const std::string& nome_file, int& N) {
    std::ifstream file(nome_file);
    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire il file " << nome_file << std::endl;
        return nullptr;
    }

    std::string linea;
    bool sezione_coordinate = false;
    Citta* citta = nullptr;
    int count = 0;

    while (std::getline(file, linea)) {
        if (linea.find("DIMENSION") != std::string::npos) {
            size_t pos = linea.find(":");
            N = std::stoi(linea.substr(pos + 1));
            citta = new Citta[N];
        }
        if (linea.find("NODE_COORD_SECTION") != std::string::npos) {
            sezione_coordinate = true;
            continue;
        }
        if (linea.find("EOF") != std::string::npos) break;

        if (sezione_coordinate && count < N) {
            std::stringstream ss(linea);
            ss >> citta[count].id >> citta[count].x >> citta[count].y;
            citta[count].prossimo = nullptr;
            count++;
        }
    }
    return citta;
}