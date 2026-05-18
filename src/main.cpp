#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono> 
#include <cfloat>
#include <thread>
#include <string>
#include <fstream> 
#include <cmath> 

#include "strutture.h"
#include "geometria.h"
#include "motore_stce.h"
#include "parser.h" 

using namespace std;
using namespace std::chrono; 

struct RisultatoBenchmark {
    string nome_dataset;
    int numero_nodi;
    long long tempo_stce_ms;
    long long tempo_nn_ms;
    float dist_stce;
    float dist_nn;
};

// =====================================================================
// ALGORITMO NEAREST NEIGHBOR CON TRACCIAMENTO DELLA ROTTA
// =====================================================================
float eseguiNearestNeighbor(Citta* citta, int N, vector<Citta*>& tour_nn) {
    vector<bool> visitato(N, false);
    int curr = 0; visitato[0] = true; float dist_totale = 0;
    tour_nn.push_back(&citta[0]);

    for(int step = 1; step < N; step++) {
        int best_next = -1; float min_dist = FLT_MAX;
        for(int i = 0; i < N; i++) {
            if(!visitato[i]) {
                float d = calcolaDistanza(citta[curr], citta[i]);
                if(d < min_dist) { min_dist = d; best_next = i; }
            }
        }
        dist_totale += min_dist; visitato[best_next] = true; curr = best_next;
        tour_nn.push_back(&citta[best_next]);
    }
    dist_totale += calcolaDistanza(citta[curr], citta[0]);
    return dist_totale;
}

// =====================================================================
// FUNZIONE DI ELABORAZIONE UNIFICATA E SICURA (Fix Memoria)
// =====================================================================
void calcolaRotte(Citta* citta, int N, bool calcola_nn, float& dist_NN, long long& tempo_NN, float& dist_STCE, long long& tempo_STCE, string prefisso_file) {
    
    vector<Citta*> tour_nn;
    if (calcola_nn) {
        auto start_NN = high_resolution_clock::now(); 
        dist_NN = eseguiNearestNeighbor(citta, N, tour_nn);
        tempo_NN = duration_cast<milliseconds>(high_resolution_clock::now() - start_NN).count();

        // SALVATAGGIO IMMEDIATO: Esportiamo il NN prima che il sort mescoli i puntatori!
        if (prefisso_file != "") {
            ofstream outNN(prefisso_file + "_nn.csv");
            if (outNN.is_open()) {
                outNN << "X,Y\n";
                for (auto c : tour_nn) outNN << c->x << "," << c->y << "\n";
                outNN << tour_nn[0]->x << "," << tour_nn[0]->y << "\n"; 
                outNN.close();
            }
        }
    } else {
        dist_NN = -1.0f;
        tempo_NN = -1;
    }

    auto start_STCE = high_resolution_clock::now(); 
    
    float min_x = citta[0].x, max_x = citta[0].x, min_y = citta[0].y, max_y = citta[0].y;
    for(int i = 1; i < N; i++) {
        min_x = min(min_x, citta[i].x); max_x = max(max_x, citta[i].x);
        min_y = min(min_y, citta[i].y); max_y = max(max_y, citta[i].y);
    }
    float ratio = (max_x - min_x) / (max_y - min_y);
    int num_strisce = max(1, (int)sqrt(N * ratio));
    float ampiezza = (max_x - min_x) / num_strisce;

    sort(citta, citta + N, [&](const Citta& a, const Citta& b) {
        int sa = (a.x - min_x) / ampiezza, sb = (b.x - min_x) / ampiezza;
        if (sa != sb) return sa < sb;
        return sa % 2 == 0 ? a.y < b.y : a.y > b.y;
    });

    vector<Triangolo> universo; vector<Citta*> orfane;
    costruisciMappaIntelligente(citta, N, universo, orfane);
    Triangolo* cat = &universo[trovaIndiceCatalizzatore(citta, N, universo)];
    vector<Triangolo> emS, emD; dividiEmisferi(cat, universo, emS, emD);
    cat->visitato = true;
    Citta *pN_A = nullptr, *pN_B = nullptr;
    thread tA([&]() { pN_A = espandiEmisfero(cat->vertici[1], emS); });
    thread tB([&]() { pN_B = espandiEmisfero(cat->vertici[2], emD); });
    tA.join(); tB.join();

    vector<Citta*> tour; tour.push_back(cat->vertici[0]);
    Citta* cur = cat->vertici[1]; while(cur) { tour.push_back(cur); if(cur == pN_A) break; cur = cur->prossimo; }
    vector<Citta*> rB; cur = cat->vertici[2]; while(cur) { rB.push_back(cur); if(cur == pN_B) break; cur = cur->prossimo; }
    reverse(rB.begin(), rB.end()); for(auto c : rB) tour.push_back(c);

    for(auto o : orfane) {
        float min_c = FLT_MAX; int best_i = 0;
        for(int i = 0; i < tour.size(); i++) {
            float c = calcolaDistanza(*tour[i], *o) + calcolaDistanza(*o, *tour[(i+1)%tour.size()]) - calcolaDistanza(*tour[i], *tour[(i+1)%tour.size()]);
            if(c < min_c) { min_c = c; best_i = i; }
        }
        tour.insert(tour.begin() + best_i + 1, o);
    }

    ottimizzaCon2Opt(tour);
    
    dist_STCE = 0;
    for(int i = 0; i < tour.size(); i++) dist_STCE += calcolaDistanza(*tour[i], *tour[(i+1)%tour.size()]);
    tempo_STCE = duration_cast<milliseconds>(high_resolution_clock::now() - start_STCE).count();

    if (prefisso_file != "") {
        ofstream outSTCE(prefisso_file + "_stce.csv");
        if (outSTCE.is_open()) {
            outSTCE << "X,Y\n";
            for (auto c : tour) outSTCE << c->x << "," << c->y << "\n";
            outSTCE << tour[0]->x << "," << tour[0]->y << "\n"; 
            outSTCE.close();
        }
    }
}

void gestioneMappaSingola() {
    int scelta; cout << "\n1. Mappa Casuale | 2. Carica TSPLIB: "; cin >> scelta;
    int N = 0; Citta* citta = nullptr;

    if (scelta == 1) {
        cout << "N. citta: "; cin >> N;
        if (N < 3) return;
        srand(time(NULL)); citta = new Citta[N]; 
        for(int i = 0; i < N; i++) { citta[i].id = i; citta[i].x = (rand() % 20000) / 10.0; citta[i].y = (rand() % 20000) / 10.0; citta[i].prossimo = nullptr; }
    } else if (scelta == 2) {
        string file; cout << "Nome file: "; cin >> file;
        citta = caricaMappaTSPLIB("data/" + file, N);
        if (!citta) return;
    } else return;

    float dist_NN = 0, dist_STCE = 0;
    long long tempo_NN = 0, tempo_STCE = 0;

    calcolaRotte(citta, N, true, dist_NN, tempo_NN, dist_STCE, tempo_STCE, "rotta_singola");

    cout << "\n-------------------------------------------";
    cout << "\n NN: " << dist_NN << " (" << tempo_NN << "ms)";
    cout << "\n STCE: " << dist_STCE << " (" << tempo_STCE << "ms)";
    cout << "\n-------------------------------------------\n";

    delete[] citta;
}

void gestioneBenchmarkSuite() {
    cout << "\n[ AVVIO TEST SEQUENZIALE AUTOMATIZZATO ]\n";

    vector<string> mappe = {
        "data/pr1002.tsp",
        "data/fl3795.tsp",
        "data/rl5915.tsp",
        "data/pla85900.tsp"
    };

    vector<RisultatoBenchmark> archivio_risultati;

    for (const auto& percorso : mappe) {
        int N = 0;
        Citta* citta = caricaMappaTSPLIB(percorso, N);
        if (!citta) continue;

        // ACCELERAZIONE SBLOCCATA: l'algoritmo Nearest Neighbor viene eseguito sempre
        bool calcola_nn = true;
        float dist_NN = 0, dist_STCE = 0;
        long long tempo_NN = 0, tempo_STCE = 0;

        size_t last_slash = percorso.find_last_of("/");
        string nome_puro = (last_slash == string::npos) ? percorso : percorso.substr(last_slash + 1);
        nome_puro = nome_puro.substr(0, nome_puro.find_last_of("."));

        calcolaRotte(citta, N, calcola_nn, dist_NN, tempo_NN, dist_STCE, tempo_STCE, "data_" + nome_puro);

        cout << "\nDataset: " << percorso;
        cout << "\n-------------------------------------------";
        cout << "\n NN: " << dist_NN << " (" << tempo_NN << "ms)";
        cout << "\n STCE: " << dist_STCE << " (" << tempo_STCE << "ms)";
        cout << "\n-------------------------------------------\n";

        archivio_risultati.push_back({percorso, N, tempo_STCE, tempo_NN, dist_STCE, dist_NN});
        delete[] citta;
    }

    ofstream csvFile("benchmark_results.csv");
    if (csvFile.is_open()) {
        csvFile << "Dataset,Nodi,TempoSTCE,TempoNN,DistanzaSTCE,DistanzaNN\n";
        for (const auto& res : archivio_risultati) {
            csvFile << res.nome_dataset << "," << res.numero_nodi << "," << res.tempo_stce_ms << "," << res.tempo_nn_ms << "," << res.dist_stce << "," << res.dist_nn << "\n";
        }
        csvFile.close();
        cout << "[+] Dati CSV e rotte geometriche pronti per la Dashboard.\n";
    }
}

int main() {
    cout << "\n[ S.T.C.E. ENGINE - BENCHMARK V1.5 ]\n";
    int scelta_utente; bool attivo = true;
    while (attivo) {
        cout << "\nMenu Selezioni:\n1. Esegui Mappa Singola\n2. Avvia Suite Benchmark Automatica (4 Mappe)\n3. Termina Programma\nScelta: ";
        if (cin >> scelta_utente) {
            switch (scelta_utente) {
                case 1: gestioneMappaSingola(); break;
                case 2: gestioneBenchmarkSuite(); break;
                case 3: cout << "Uscita dal sistema.\n"; attivo = false; break;
                default: cout << "Opzione non disponibile.\n";
            }
        } else {
            cin.clear(); cin.ignore(10000, '\n'); cout << "Input anomalo.\n";
        }
    }
    return 0;
}