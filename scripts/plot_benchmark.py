import pandas as pd
import matplotlib.pyplot as plt
import os

# Percorso del file CSV (assumendo che esegui lo script dalla root del progetto)
csv_path = 'benchmark_results.csv'

# Controllo se il file CSV esiste
if not os.path.exists(csv_path):
    print(f"Errore: Il file {csv_path} non esiste. Assicurati di aver eseguito il benchmark C++.")
    exit()

print("Lettura dati di benchmark...")
df = pd.read_csv(csv_path)

# =====================================================================
# GENERAZIONE DEL GRAFICO (Stile IEEE Paper)
# =====================================================================
plt.figure(figsize=(10, 6))

# Usa uno stile pulito se disponibile, altrimenti usa il default
try:
    plt.style.use('seaborn-v0_8-whitegrid')
except:
    plt.grid(True, linestyle='--', alpha=0.5)

# Dati S.T.C.E. (sempre presenti)
nodi_stce = df['Nodi']
tempi_stce = df['TempoSTCE']

# Dati Nearest Neighbor (filtriamo i test saltati dove TempoNN è -1)
df_nn = df[df['TempoNN'] > 0]
nodi_nn = df_nn['Nodi']
tempi_nn = df_nn['TempoNN']

# Traccia la curva S.T.C.E. 
plt.plot(nodi_stce, tempi_stce, marker='o', linestyle='-', color='#002147', 
         linewidth=2.5, markersize=8, label='S.T.C.E. Engine')

# Traccia la curva Nearest Neighbor (solo per i nodi elaborati)
if not df_nn.empty:
    plt.plot(nodi_nn, tempi_nn, marker='s', linestyle='--', color='#D40000', 
             linewidth=2, markersize=6, label='Nearest Neighbor (Standard)')

# Formattazione assi e titoli
plt.title('Scalabilità Temporale: S.T.C.E. vs Nearest Neighbor', fontsize=14, fontweight='bold', pad=20)
plt.xlabel('Numero di Nodi (N)', fontsize=12)
plt.ylabel('Tempo di Esecuzione (ms)', fontsize=12)

# Scala logaritmica (fondamentale per mostrare la differenza tra O(n^2) e O(n log n))
plt.xscale('log')
plt.yscale('log')

# Aggiunge le etichette con i millisecondi esatti sopra i punti dell'S.T.C.E.
for i in range(len(nodi_stce)):
    plt.annotate(f"{tempi_stce.iloc[i]} ms", (nodi_stce.iloc[i], tempi_stce.iloc[i]), 
                 textcoords="offset points", xytext=(0,10), ha='center', 
                 fontsize=10, fontweight='bold', color='#002147')

plt.legend(fontsize=11, loc='upper left', frameon=True, shadow=True)
plt.tight_layout()

# Salvataggio in alta risoluzione per il paper
output_img = 'grafico_scalabilita_paper.png'
plt.savefig(output_img, dpi=300)
print(f"[+] Grafico vettoriale salvato con successo: {output_img}")

# Mostra il grafico a schermo
plt.show()