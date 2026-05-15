#include "motore_stce.h"
#include "geometria.h"
#include <algorithm>
#include <cfloat>
#include <cmath> 
#include <vector>

using namespace std;

// =================================================================
// COSTRUZIONE BASE E MULTI-THREADING (Invariati)
// =================================================================

void costruisciMappaIntelligente(Citta* citta, int N, vector<Triangolo>& universo, vector<Citta*>& orfane) {
    for (int i = 0; i < N - 2; i += 3) {
        Triangolo t;
        t.vertici[0] = &citta[i];
        t.vertici[1] = &citta[i+1];
        t.vertici[2] = &citta[i+2];
        t.visitato = false;
        universo.push_back(t);
    }
    for (int i = N - (N % 3); i < N; i++) {
        orfane.push_back(&citta[i]);
    }
}

int trovaIndiceCatalizzatore(Citta* citta, int N, vector<Triangolo>& universo) {
    float sum_x = 0, sum_y = 0;
    for (int i = 0; i < N; i++) { sum_x += citta[i].x; sum_y += citta[i].y; }
    Citta centroide = {-1, "Centro", sum_x / N, sum_y / N, nullptr};

    float min_dist = FLT_MAX; int idx_best = 0;
    for (int i = 0; i < universo.size(); i++) {
        float cx = (universo[i].vertici[0]->x + universo[i].vertici[1]->x + universo[i].vertici[2]->x) / 3.0f;
        float cy = (universo[i].vertici[0]->y + universo[i].vertici[1]->y + universo[i].vertici[2]->y) / 3.0f;
        Citta centro_triangolo = {-1, "T", cx, cy, nullptr};
        float d = calcolaDistanza(centroide, centro_triangolo);
        if (d < min_dist) { min_dist = d; idx_best = i; }
    }
    return idx_best;
}

void dividiEmisferi(Triangolo* catalizzatore, vector<Triangolo>& universo, vector<Triangolo>& emisfero_Sinistro, vector<Triangolo>& emisfero_Destro) {
    if (universo.empty()) return;

    vector<float> centri_x;
    for (int i = 0; i < universo.size(); i++) {
        float cx = (universo[i].vertici[0]->x + universo[i].vertici[1]->x + universo[i].vertici[2]->x) / 3.0f;
        centri_x.push_back(cx);
    }

    int n = centri_x.size();
    std::nth_element(centri_x.begin(), centri_x.begin() + n / 2, centri_x.end());
    float mediana_x = centri_x[n / 2];

    for (int i = 0; i < universo.size(); i++) {
        if (&universo[i] == catalizzatore) continue;
        float cx = (universo[i].vertici[0]->x + universo[i].vertici[1]->x + universo[i].vertici[2]->x) / 3.0f;
        if (cx <= mediana_x) emisfero_Sinistro.push_back(universo[i]); 
        else emisfero_Destro.push_back(universo[i]);                 
    }
}

Rotta ricercaGreedyLocale(Citta* posizione_attuale, vector<Triangolo>& emisfero_privato) {
    Rotta rotta_migliore = {FLT_MAX, nullptr, -1}; 
    for (int i = 0; i < emisfero_privato.size(); i++) {
        if (!emisfero_privato[i].visitato) {
            for (int v = 0; v < 3; v++) {
                float peso = calcolaPeso(*posizione_attuale, *emisfero_privato[i].vertici[v]);
                if (peso < rotta_migliore.peso) {
                    rotta_migliore = {peso, &emisfero_privato[i], v}; 
                }
            }
        }
    }
    if (rotta_migliore.cluster == nullptr) return {-1, nullptr, -1};
    return rotta_migliore; 
}

Citta* espandiEmisfero(Citta* partenza, vector<Triangolo>& emisfero_privato) {
    Citta* attuale = partenza;
    while(true) {
        Rotta next = ricercaGreedyLocale(attuale, emisfero_privato);
        if(next.cluster == nullptr) break;
        
        next.cluster->visitato = true;
        int v1 = next.vertice_scelto;
        int v2 = (v1 + 1) % 3;
        int v3 = (v1 + 2) % 3;

        attuale->prossimo = next.cluster->vertici[v1];
        attuale = attuale->prossimo;

        float d2 = calcolaDistanza(*attuale, *next.cluster->vertici[v2]);
        float d3 = calcolaDistanza(*attuale, *next.cluster->vertici[v3]);

        if (d2 < d3) {
            attuale->prossimo = next.cluster->vertici[v2];
            attuale = attuale->prossimo;
            attuale->prossimo = next.cluster->vertici[v3];
            attuale = attuale->prossimo;
        } else {
            attuale->prossimo = next.cluster->vertici[v3];
            attuale = attuale->prossimo;
            attuale->prossimo = next.cluster->vertici[v2];
            attuale = attuale->prossimo;
        }
    }
    return attuale; 
}

void eseguiScambio2Opt(vector<Citta*>& tour, int i, int k) {
    std::reverse(tour.begin() + i, tour.begin() + k + 1);
}

// =================================================================
// IL RADAR SPAZIALE (KD-TREE)
// =================================================================

struct KDNode {
    Citta* citta;
    KDNode* left;
    KDNode* right;
    int asse; // 0 per X, 1 per Y
};

KDNode* costruisciKD(vector<Citta*>& pts, int depth) {
    if(pts.empty()) return nullptr;
    int asse = depth % 2;
    int mid = pts.size() / 2;
    
    // Troviamo la mediana spaziale
    std::nth_element(pts.begin(), pts.begin() + mid, pts.end(),
        [asse](Citta* a, Citta* b) { return asse == 0 ? (a->x < b->x) : (a->y < b->y); });

    KDNode* nodo = new KDNode{pts[mid], nullptr, nullptr, asse};
    
    vector<Citta*> left_pts(pts.begin(), pts.begin() + mid);
    vector<Citta*> right_pts(pts.begin() + mid + 1, pts.end());
    
    nodo->left = costruisciKD(left_pts, depth + 1);
    nodo->right = costruisciKD(right_pts, depth + 1);
    return nodo;
}

void cercaNelRaggio(KDNode* radice, Citta* centro, float raggio, vector<Citta*>& risultati) {
    if(!radice) return;
    
    float d = calcolaDistanza(*centro, *(radice->citta));
    if(d <= raggio) risultati.push_back(radice->citta);

    // Decidiamo se il radar deve controllare i rami dell'albero o ignorarli
    float dist_asse = (radice->asse == 0) ? (centro->x - radice->citta->x) : (centro->y - radice->citta->y);

    if(dist_asse <= 0) {
        cercaNelRaggio(radice->left, centro, raggio, risultati);
        // Se la sfera del radar "tocca" il confine, controlliamo anche l'altro lato
        if(std::abs(dist_asse) <= raggio) cercaNelRaggio(radice->right, centro, raggio, risultati);
    } else {
        cercaNelRaggio(radice->right, centro, raggio, risultati);
        if(std::abs(dist_asse) <= raggio) cercaNelRaggio(radice->left, centro, raggio, risultati);
    }
}

void distruggiKD(KDNode* radice) {
    if(!radice) return;
    distruggiKD(radice->left);
    distruggiKD(radice->right);
    delete radice;
}

// =================================================================
// CHIRURGIA 2-OPT ACCELERATA CON KD-TREE
// =================================================================

void ottimizzaCon2Opt(vector<Citta*>& tour) {
    int n = tour.size();
    if (n < 4) return;

    // 1. Mappiamo le posizioni in tempo reale (O(1) lookup)
    int max_id = 0;
    for(Citta* c : tour) if(c->id > max_id) max_id = c->id;
    vector<int> tour_idx(max_id + 1, 0);
    for(int i = 0; i < n; i++) tour_idx[tour[i]->id] = i;

    // 2. Costruiamo il Radar (KD-Tree) una sola volta
    vector<Citta*> pts = tour; 
    KDNode* radice = costruisciKD(pts, 0);

    bool miglioramento = true;
    int iterazioni = 0; 

    while (miglioramento && iterazioni < 30) {
        miglioramento = false;
        
        for (int i = 0; i < n - 1; i++) {
            // Il Raggio: la distanza del taglio. Perché cercare più lontano di così?
            float raggio = calcolaDistanza(*tour[i], *tour[i + 1]); 

            vector<Citta*> vicini;
            cercaNelRaggio(radice, tour[i], raggio, vicini);

            for (Citta* vicino : vicini) {
                int k = tour_idx[vicino->id];
                
                // k deve essere successivo ad i nel tour per validare il 2-Opt
                if (k > i + 1 && k < n) {
                    float d_vecchia = calcolaDistanza(*tour[i], *tour[i + 1]) + calcolaDistanza(*tour[k], *tour[(k + 1) % n]);
                    float d_nuova = calcolaDistanza(*tour[i], *tour[k]) + calcolaDistanza(*tour[i + 1], *tour[(k + 1) % n]);
                    
                    if (d_nuova < d_vecchia - 0.01) { 
                        eseguiScambio2Opt(tour, i + 1, k);
                        
                        // Aggiorniamo istantaneamente le posizioni solo per il frammento invertito
                        for(int j = i + 1; j <= k; j++) {
                            tour_idx[tour[j]->id] = j;
                        }
                        miglioramento = true;
                    }
                }
            }
        }
        iterazioni++;
    }
    
    // Spegniamo il radar per non lasciare residui nella RAM
    distruggiKD(radice); 
}