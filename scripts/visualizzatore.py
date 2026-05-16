import pandas as pd
import matplotlib.pyplot as plt

# 1. Carica le coordinate dal file CSV generato dal tuo motore C++
print("Caricamento rotta in corso...")
df = pd.read_csv('rotta_stce.csv')

X = df['X'].tolist()
Y = df['Y'].tolist()

# 2. Configura la tela grafica
plt.figure(figsize=(12, 8))
plt.title(f"S.T.C.E. Engine - Percorso Ottimizzato ({len(X)-1} nodi)", fontsize=16)

# 3. Disegna le linee e i punti
# Per mappe enormi (es. 85.000 nodi) disegniamo solo linee sottili per non bloccare il PC
plt.plot(X, Y, color='blue', linewidth=0.5, alpha=0.8, label='Rotta 2-Opt (KD-Tree)')
plt.scatter(X, Y, color='red', s=1, alpha=0.5) # Dimensione punti minima

# 4. Mostra a schermo
plt.xlabel("Coordinate X")
plt.ylabel("Coordinate Y")
plt.grid(True, linestyle='--', alpha=0.5)
plt.legend()
plt.tight_layout()

print("Mappa generata! Chiudi la finestra grafica per terminare lo script.")
plt.show()