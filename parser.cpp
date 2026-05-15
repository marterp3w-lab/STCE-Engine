#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

Citta* caricaMappaTSPLIB(const string& nome_file, int& N) {
    ifstream file(nome_file);
    if (!file.is_open()) {
        cerr << "ERRORE CRITICO: Impossibile aprire il file " << nome_file << endl;
        N = 0;
        return nullptr;
    }

    string riga;
    bool leggendo_coordinate = false;
    vector<Citta> citta_temporanee;

    while (getline(file, riga)) {
        if (riga.find("EOF") != string::npos) break;
        if (riga.find("NODE_COORD_SECTION") != string::npos) {
            leggendo_coordinate = true;
            continue;
        }

        if (leggendo_coordinate) {
            istringstream iss(riga);
            int id; float x, y;
            if (iss >> id >> x >> y) {
                Citta nuova_citta;
                nuova_citta.id = id - 1; 
                nuova_citta.nome = "Nodo_" + to_string(id);
                nuova_citta.x = x;
                nuova_citta.y = y;
                nuova_citta.prossimo = nullptr;
                citta_temporanee.push_back(nuova_citta);
            }
        }
    }
    file.close();

    N = citta_temporanee.size();
    if (N == 0) return nullptr;

    Citta* citta = new Citta[N];
    for (int i = 0; i < N; i++) citta[i] = citta_temporanee[i];

    return citta;
}