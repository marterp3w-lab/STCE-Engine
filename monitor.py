import matplotlib.pyplot as plt
import csv

asse_x = []
asse_y = []

try:
    with open('rotta_stce.csv', 'r') as file:
        lettore = csv.reader(file)
        next(lettore) 
        for riga in lettore:
            asse_x.append(float(riga[0]))
            asse_y.append(float(riga[1]))
except FileNotFoundError:
    print("Errore: File 'rotta_stce.csv' non trovato. Esegui prima il C++.")
    exit()

plt.figure(figsize=(10, 8))
plt.plot(asse_x, asse_y, marker='o', markersize=3, markerfacecolor='blue', linestyle='-', color='black', linewidth=0.8)

plt.title("Diagnostica STCE Hybrid - Analisi Tracciato")
plt.xlabel("Asse X")
plt.ylabel("Asse Y")
plt.grid(True, linestyle='--', alpha=0.5)

plt.show()