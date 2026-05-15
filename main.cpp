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

float eseguiNearestNeighbor(Citta* citta, int N) {
    vector<bool> visitato(N, false);
    int curr = 0; visitato[0] = true; float dist_totale = 0;
    for(int step = 1; step < N; step++) {
        int best_next = -1; float min_dist = FLT_MAX;
        for(int i = 0; i < N; i++) {
            if(!visitato[i]) {
                float d = calcolaDistanza(citta[curr], citta[i]);
                if(d < min_dist) { min_dist = d; best_next = i; }
            }
        }
        dist_totale += min_dist; visitato[best_next] = true; curr = best_next;
    }
    dist_totale += calcolaDistanza(citta[curr], citta[0]);
    return dist_totale;
}

int main() {
    cout << "\n[ S.T.C.E. ENGINE - BENCHMARK V1.4 ]\n\n";
    int scelta; cout << "1. Mappa Casuale | 2. Carica TSPLIB: "; cin >> scelta;
    int N = 0; Citta* citta = nullptr;

    if (scelta == 1) {
        cout << "N. citta: "; cin >> N;
        if (N < 3) return 0;
        srand(time(NULL)); citta = new Citta[N]; 
        for(int i = 0; i < N; i++) { citta[i].id = i; citta[i].x = (rand() % 20000) / 10.0; citta[i].y = (rand() % 20000) / 10.0; citta[i].prossimo = nullptr; }
    } else if (scelta == 2) {
        string file; cout << "File .tsp: "; cin >> file;
        citta = caricaMappaTSPLIB(file, N);
        if (!citta) return 0;
    } else return 0;

    auto start_NN = high_resolution_clock::now(); 
    float dist_NN = eseguiNearestNeighbor(citta, N);
    auto duration_NN = duration_cast<milliseconds>(high_resolution_clock::now() - start_NN);

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
    float dist_STCE = 0;
    for(int i = 0; i < tour.size(); i++) dist_STCE += calcolaDistanza(*tour[i], *tour[(i+1)%tour.size()]);
    auto duration_STCE = duration_cast<milliseconds>(high_resolution_clock::now() - start_STCE);

    cout << "\n-------------------------------------------\n NN: " << dist_NN << " (" << duration_NN.count() << "ms)\n STCE: " << dist_STCE << " (" << duration_STCE.count() << "ms)\n-------------------------------------------\n";

    delete[] citta; return 0;
}