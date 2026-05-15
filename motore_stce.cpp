#include "motore_stce.h"
#include "geometria.h"
#include <algorithm>
#include <cfloat>
#include <cmath> 
#include <vector>
#include <memory>

using namespace std;

void costruisciMappaIntelligente(Citta* citta, int N, vector<Triangolo>& universo, vector<Citta*>& orfane) {
    for (int i = 0; i < N - 2; i += 3) {
        Triangolo t;
        t.vertici[0] = &citta[i];
        t.vertici[1] = &citta[i+1];
        t.vertici[2] = &citta[i+2];
        t.visitato = false;
        universo.push_back(t);
    }
    for (int i = N - (N % 3); i < N; i++) orfane.push_back(&citta[i]);
}

int trovaIndiceCatalizzatore(Citta* citta, int N, vector<Triangolo>& universo) {
    float sum_x = 0, sum_y = 0;
    for (int i = 0; i < N; i++) { sum_x += citta[i].x; sum_y += citta[i].y; }
    Citta centroide = {-1, "Centro", sum_x / N, sum_y / N, nullptr};
    float min_dist = FLT_MAX; int idx_best = 0;
    for (int i = 0; i < universo.size(); i++) {
        float cx = (universo[i].vertici[0]->x + universo[i].vertici[1]->x + universo[i].vertici[2]->x) / 3.0f;
        float cy = (universo[i].vertici[0]->y + universo[i].vertici[1]->y + universo[i].vertici[2]->y) / 3.0f;
        Citta ct = {-1, "T", cx, cy, nullptr};
        float d = calcolaDistanza(centroide, ct);
        if (d < min_dist) { min_dist = d; idx_best = i; }
    }
    return idx_best;
}

void dividiEmisferi(Triangolo* catalizzatore, vector<Triangolo>& universo, vector<Triangolo>& emisfero_Sinistro, vector<Triangolo>& emisfero_Destro) {
    if (universo.empty()) return;
    vector<float> centri_x;
    for (auto& t : universo) centri_x.push_back((t.vertici[0]->x + t.vertici[1]->x + t.vertici[2]->x) / 3.0f);
    int n = centri_x.size();
    nth_element(centri_x.begin(), centri_x.begin() + n / 2, centri_x.end());
    float mediana_x = centri_x[n / 2];
    for (auto& t : universo) {
        if (&t == catalizzatore) continue;
        float cx = (t.vertici[0]->x + t.vertici[1]->x + t.vertici[2]->x) / 3.0f;
        if (cx <= mediana_x) emisfero_Sinistro.push_back(t);
        else emisfero_Destro.push_back(t);
    }
}

Rotta ricercaGreedyLocale(Citta* attuale, vector<Triangolo>& emisfero) {
    Rotta best = {FLT_MAX, nullptr, -1};
    for (auto& t : emisfero) {
        if (!t.visitato) {
            for (int v = 0; v < 3; v++) {
                float p = calcolaPeso(*attuale, *t.vertici[v]);
                if (p < best.peso) best = {p, &t, v};
            }
        }
    }
    return best;
}

Citta* espandiEmisfero(Citta* partenza, vector<Triangolo>& emisfero) {
    Citta* attuale = partenza;
    while(true) {
        Rotta next = ricercaGreedyLocale(attuale, emisfero);
        if(next.cluster == nullptr) break;
        next.cluster->visitato = true;
        int v1 = next.vertice_scelto, v2 = (v1+1)%3, v3 = (v1+2)%3;
        attuale->prossimo = next.cluster->vertici[v1];
        attuale = attuale->prossimo;
        if (calcolaDistanza(*attuale, *next.cluster->vertici[v2]) < calcolaDistanza(*attuale, *next.cluster->vertici[v3])) {
            attuale->prossimo = next.cluster->vertici[v2]; attuale = attuale->prossimo;
            attuale->prossimo = next.cluster->vertici[v3]; attuale = attuale->prossimo;
        } else {
            attuale->prossimo = next.cluster->vertici[v3]; attuale = attuale->prossimo;
            attuale->prossimo = next.cluster->vertici[v2]; attuale = attuale->prossimo;
        }
    }
    return attuale;
}

struct KDNode {
    Citta* citta;
    unique_ptr<KDNode> left, right;
    int asse;
};

unique_ptr<KDNode> costruisciKD(vector<Citta*>& pts, int depth) {
    if(pts.empty()) return nullptr;
    int asse = depth % 2;
    int mid = pts.size() / 2;
    nth_element(pts.begin(), pts.begin() + mid, pts.end(), [asse](Citta* a, Citta* b) {
        return asse == 0 ? (a->x < b->x) : (a->y < b->y);
    });
    auto nodo = make_unique<KDNode>();
    nodo->citta = pts[mid]; nodo->asse = asse;
    vector<Citta*> left_pts(pts.begin(), pts.begin() + mid);
    vector<Citta*> right_pts(pts.begin() + mid + 1, pts.end());
    nodo->left = costruisciKD(left_pts, depth + 1);
    nodo->right = costruisciKD(right_pts, depth + 1);
    return nodo;
}

void cercaNelRaggio(KDNode* radice, Citta* centro, float raggio, vector<Citta*>& risultati) {
    if(!radice) return;
    if(calcolaDistanza(*centro, *(radice->citta)) <= raggio) risultati.push_back(radice->citta);
    float dist_asse = (radice->asse == 0) ? (centro->x - radice->citta->x) : (centro->y - radice->citta->y);
    if(dist_asse <= 0) {
        cercaNelRaggio(radice->left.get(), centro, raggio, risultati);
        if(abs(dist_asse) <= raggio) cercaNelRaggio(radice->right.get(), centro, raggio, risultati);
    } else {
        cercaNelRaggio(radice->right.get(), centro, raggio, risultati);
        if(abs(dist_asse) <= raggio) cercaNelRaggio(radice->left.get(), centro, raggio, risultati);
    }
}

void ottimizzaCon2Opt(vector<Citta*>& tour) {
    int n = tour.size(); if (n < 4) return;
    int max_id = 0;
    for(auto c : tour) if(c->id > max_id) max_id = c->id;
    vector<int> tour_idx(max_id + 1, 0);
    for(int i = 0; i < n; i++) tour_idx[tour[i]->id] = i;
    vector<Citta*> pts = tour;
    unique_ptr<KDNode> radice = costruisciKD(pts, 0);
    bool miglioramento = true; int iter = 0;
    while (miglioramento && iter < 30) {
        miglioramento = false;
        for (int i = 0; i < n - 1; i++) {
            float raggio = calcolaDistanza(*tour[i], *tour[i + 1]);
            vector<Citta*> vicini; cercaNelRaggio(radice.get(), tour[i], raggio, vicini);
            for (auto vicino : vicini) {
                int k = tour_idx[vicino->id];
                if (k > i + 1 && k < n) {
                    float d_vecchia = calcolaDistanza(*tour[i], *tour[i + 1]) + calcolaDistanza(*tour[k], *tour[(k + 1) % n]);
                    float d_nuova = calcolaDistanza(*tour[i], *tour[k]) + calcolaDistanza(*tour[i + 1], *tour[(k + 1) % n]);
                    if (d_nuova < d_vecchia - 0.01) {
                        reverse(tour.begin() + i + 1, tour.begin() + k + 1);
                        for(int j = i + 1; j <= k; j++) tour_idx[tour[j]->id] = j;
                        miglioramento = true;
                    }
                }
            }
        }
        iter++;
    }
}