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

// ==========================================
// ALGORITMO SFIDANTE: NEAREST NEIGHBOR
// ==========================================
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
    cout << "===========================================" << endl;
    cout << " STCE ENGINE - ARCHITETTURA FINALE V1.3    " << endl;
    cout << " (DYNAMIC RATIO & DEEP 2-OPT ATTIVATI)     " << endl;
    cout << "===========================================" << endl;

    int scelta;
    cout << "> [1] Usa Mappa Casuale (Simulazione)" << endl;
    cout << "> [2] Carica File TSPLIB (Mondo Reale)" << endl;
    cout << "> Scegli: ";
    cin >> scelta;

    int N = 0;
    Citta* citta = nullptr;

    if (scelta == 1) {
        cout << "> Inserisci numero citta per il test: "; cin >> N;
        if (N < 3) return 0;
        srand(time(NULL)); 
        citta = new Citta[N]; 
        for(int i = 0; i < N; i++) {
            citta[i].id = i; 
            citta[i].x = (rand() % 20000) / 10.0; 
            citta[i].y = (rand() % 20000) / 10.0; 
            citta[i].prossimo = nullptr;
        }
    } else if (scelta == 2) {
        string nome_file;
        cout << "> Nome del file (es. pla85900.tsp): "; cin >> nome_file;
        citta = caricaMappaTSPLIB(nome_file, N);
        if (citta == nullptr || N < 3) return 0;
    } else return 0;

    cout << "> Avvio benchmark..." << endl;

    // --- NEAREST NEIGHBOR (Sfidante) ---
    auto start_NN = high_resolution_clock::now(); 
    float distanza_NN = eseguiNearestNeighbor(citta, N);
    auto stop_NN = high_resolution_clock::now();  
    auto duration_NN = duration_cast<milliseconds>(stop_NN - start_NN);


    // --- STCE HYBRID (IL TUO MOTORE MULTI-CORE) ---
    auto start_STCE = high_resolution_clock::now(); 

    // -------------------------------------------------------------
    // BOUSTROPHEDON SORT CON ASPECT RATIO DINAMICO
    // -------------------------------------------------------------
    float min_x = citta[0].x, max_x = citta[0].x;
    float min_y = citta[0].y, max_y = citta[0].y;
    
    for(int i = 1; i < N; i++) {
        if(citta[i].x < min_x) min_x = citta[i].x;
        if(citta[i].x > max_x) max_x = citta[i].x;
        if(citta[i].y < min_y) min_y = citta[i].y;
        if(citta[i].y > max_y) max_y = citta[i].y;
    }
    
    float larghezza = max_x - min_x;
    float altezza = max_y - min_y;
    if(larghezza <= 0) larghezza = 1.0f;
    if(altezza <= 0) altezza = 1.0f;

    // Calcoliamo la proporzione reale della mappa
    float ratio = larghezza / altezza;
    
    // Il numero di strisce ora scala perfettamente con l'allungamento della griglia
    int num_strisce = std::max(1, (int)std::sqrt(N * ratio));
    float ampiezza_striscia = larghezza / num_strisce;

    std::sort(citta, citta + N, [min_x, ampiezza_striscia](const Citta& a, const Citta& b) {
        int striscia_a = (a.x - min_x) / ampiezza_striscia;
        int striscia_b = (b.x - min_x) / ampiezza_striscia;
        
        if (striscia_a != striscia_b) {
            return striscia_a < striscia_b; 
        }
        
        if (striscia_a % 2 == 0) {
            return a.y < b.y; 
        } else {
            return a.y > b.y; 
        }
    });

    // -------------------------------------------------------------
    // COSTRUZIONE E MULTI-THREADING
    // -------------------------------------------------------------
    vector<Triangolo> universo;
    vector<Citta*> orfane;
    costruisciMappaIntelligente(citta, N, universo, orfane);

    int idx_cat = trovaIndiceCatalizzatore(citta, N, universo);
    Triangolo* catalizzatore = &universo[idx_cat];

    vector<Triangolo> emisfero_Sinistro_Privato;
    vector<Triangolo> emisfero_Destro_Privato;
    dividiEmisferi(catalizzatore, universo, emisfero_Sinistro_Privato, emisfero_Destro_Privato);
    catalizzatore->visitato = true;

    Citta* poloNord_A = nullptr;
    Citta* poloNord_B = nullptr;

    thread commessoA([&]() { poloNord_A = espandiEmisfero(catalizzatore->vertici[1], emisfero_Sinistro_Privato); });
    thread commessoB([&]() { poloNord_B = espandiEmisfero(catalizzatore->vertici[2], emisfero_Destro_Privato); });

    commessoA.join();
    commessoB.join();

    vector<Citta*> tour_completo;
    tour_completo.push_back(catalizzatore->vertici[0]); 
    Citta* curr = catalizzatore->vertici[1];
    while(curr != nullptr) { 
        tour_completo.push_back(curr); 
        if(curr == poloNord_A) break; 
        curr = curr->prossimo; 
    }

    vector<Citta*> ramoB;
    curr = catalizzatore->vertici[2];
    while(curr != nullptr) { 
        ramoB.push_back(curr); 
        if(curr == poloNord_B) break; 
        curr = curr->prossimo; 
    }
    reverse(ramoB.begin(), ramoB.end()); 
    for(int i = 0; i < ramoB.size(); i++) {
        tour_completo.push_back(ramoB[i]);
    }

    for(int o = 0; o < orfane.size(); o++) {
        Citta* orfana = orfane[o];
        float min_costo = FLT_MAX; 
        int best_index = 0;
        for(int i = 0; i < tour_completo.size(); i++) {
            Citta* c1 = tour_completo[i]; 
            Citta* c2 = tour_completo[(i+1) % tour_completo.size()]; 
            float costo_rottura = calcolaDistanza(*c1, *orfana) + calcolaDistanza(*orfana, *c2) - calcolaDistanza(*c1, *c2);
            if(costo_rottura < min_costo) { 
                min_costo = costo_rottura; 
                best_index = i; 
            }
        }
        tour_completo.insert(tour_completo.begin() + best_index + 1, orfana);
    }

    ottimizzaCon2Opt(tour_completo);

    float distanza_STCE = 0;
    for(int i = 0; i < tour_completo.size(); i++) {
        distanza_STCE += calcolaDistanza(*tour_completo[i], *tour_completo[(i+1) % tour_completo.size()]);
    }

    auto stop_STCE = high_resolution_clock::now(); 
    auto duration_STCE = duration_cast<milliseconds>(stop_STCE - start_STCE);

    cout << "-------------------------------------------" << endl;
    cout << "[ALGORITMO SFIDANTE: NEAREST NEIGHBOR]" << endl;
    cout << "- Distanza Totale : " << distanza_NN << endl;
    cout << "- Tempo Impiegato : " << duration_NN.count() << " ms" << endl;
    cout << "-------------------------------------------" << endl;
    cout << "[IL TUO ALGORITMO: S.T.C.E. HYBRID V1.3]" << endl;
    cout << "- Distanza Totale : " << distanza_STCE << endl;
    cout << "- Tempo Impiegato : " << duration_STCE.count() << " ms" << endl;
    cout << "===========================================" << endl;

    ofstream file_out("rotta_stce.csv");
    file_out << "X,Y\n"; 
    for(int i = 0; i < tour_completo.size(); i++) {
        file_out << tour_completo[i]->x << "," << tour_completo[i]->y << "\n";
    }
    if (!tour_completo.empty()) {
        file_out << tour_completo[0]->x << "," << tour_completo[0]->y << "\n";
    }
    file_out.close();
    cout << "> Tracciato salvato in 'rotta_stce.csv' per la diagnostica visiva." << endl;

    delete[] citta;
    return 0; 
}