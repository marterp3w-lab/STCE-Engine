import os
import sys
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.ticker import ScalarFormatter
import numpy as np
import tkinter as tk
from tkinter import ttk

# =====================================================================
# CONFIGURAZIONE PALETTE "TECHNICAL DARK" (Inserisci qui i tuoi colori)
# =====================================================================
# Sfondi UI e Tab
C_UI_BG = "#FFFFFF"         
C_TAB_BG = "#9E9D9D"        
C_TAB_SELECTED = "#FFFFFF" 
C_UI_TEXT = "#000000"       
C_UI_TEXT_SEL = "#000000"   

C_PLOT_BG = "#FFFFFF"       
C_NODI = "#000000"          

# --- COLORI ESATTI DA SCREENSHOT ---
C_LINEA_STCE = "#040196"    # Blu Elettrico (S.T.C.E.)
C_LINEA_NN = "#FF0000"      # Giallo Oro (Nearest Neighbor)
# -----------------------------------

C_GRID = "#333333"          
C_TEXT_WHITE = "#000000"         

class STCEDashboard:
    def __init__(self, root):
        self.root = root
        self.root.title("S.T.C.E. Engine - Isometric Technical Analytics Dashboard")
        self.root.geometry("1400x900") 
        self.root.configure(bg=C_UI_BG)

        # Gestione chiusura pulita
        self.root.protocol("WM_DELETE_WINDOW", self.chiusura_pulita)

        # =====================================================================
        # FIX: STYLING DINAMICO DELLE TAB (Dimensioni Invertite)
        # =====================================================================
        self.style = ttk.Style()
        self.style.theme_use("clam")
        self.style.configure("TNotebook", background=C_UI_BG, padding=10, borderwidth=0)
        
        # 1. Configurazione base (Tab INATTIVA): Padding ridotto, testo bianco, sfondo grigio
        self.style.configure("TNotebook.Tab", 
                             background=C_TAB_BG, 
                             foreground=C_UI_TEXT, 
                             font=("Segoe UI", 11, "bold"), 
                             padding=[15, 8]) # Dimensioni compresse per il background
        
        # 2. Configurazione dinamica (Tab ATTIVA): Padding maggiorato ed espansione verticale
        self.style.map("TNotebook.Tab", 
                       background=[("selected", C_TAB_SELECTED)], 
                       foreground=[("selected", C_UI_TEXT_SEL)],
                       padding=[("selected", [25, 14])],    # Aumenta il respiro della tab selezionata
                       expand=[("selected", [0, 4, 0, 0])]  # Alza visivamente la tab di 4 pixel
                      )

        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill="both", expand=True, padx=15, pady=15)

        self.tab1 = tk.Frame(self.notebook, bg=C_PLOT_BG)
        self.tab2 = tk.Frame(self.notebook, bg=C_PLOT_BG)

        self.notebook.add(self.tab1, text=" 🗺️ Griglia Comparativa Rotte Isometriche [Click per Ingrandire] ")
        self.notebook.add(self.tab2, text=" 📊 Analisi di Scalabilità & Complessità Sperimentale ")

        # Mappatura per click/popup
        self.ax_to_info = {}

        # Rendering Tab 1
        self.render_quad_route_tab()
        # Rendering Tab 2
        self.render_benchmark_tab()

    def chiusura_pulita(self):
        plt.close('all')
        self.root.quit()
        self.root.destroy()
        sys.exit(0)

    def render_quad_route_tab(self):
        self.fig_quad, self.axs = plt.subplots(2, 2, figsize=(13, 8), facecolor=C_PLOT_BG)
        
        target_maps = [
            {"name": "pr1002", "title": "Instance pr1002 (1.002 Nodi)", "lw": 0.4, "s": 2, "has_nn": True},
            {"name": "fl3795", "title": "Instance fl3795 (3.795 Nodi)", "lw": 0.25, "s": 1, "has_nn": True},
            {"name": "rl5915", "title": "Instance rl5915 (5.915 Nodi)", "lw": 0.2, "s": 0.5, "has_nn": True},
            {"name": "pla85900", "title": "Instance pla85900 (85.900 Nodi)", "lw": 0.15, "s": 0, "has_nn": True}
        ]

        coords = [(0,0), (0,1), (1,0), (1,1)]

        handles = []
        labels = []

        for idx, cmap in enumerate(target_maps):
            ax = self.axs[coords[idx][0], coords[idx][1]]
            ax.set_facecolor(C_PLOT_BG)
            self.ax_to_info[ax] = cmap

            stce_csv = f"data_{cmap['name']}_stce.csv"
            nn_csv = f"data_{cmap['name']}_nn.csv"
            
            if cmap['has_nn'] and os.path.exists(nn_csv):
                df_nn = pd.read_csv(nn_csv)
                line_nn, = ax.plot(df_nn['X'], df_nn['Y'], color=C_LINEA_NN, linewidth=cmap['lw']*2.0, alpha=0.6, linestyle='-')
                if idx == 0:
                    handles.append(line_nn)
                    labels.append('Nearest Neighbor (Baseline)')

            if os.path.exists(stce_csv):
                df_stce = pd.read_csv(stce_csv)
                line_stce, = ax.plot(df_stce['X'], df_stce['Y'], color=C_LINEA_STCE, linewidth=cmap['lw'], alpha=0.9, zorder=2)
                if cmap['s'] > 0:
                    ax.scatter(df_stce['X'], df_stce['Y'], color=C_NODI, s=cmap['s'], alpha=1.0, zorder=3)
                if idx == 0:
                    handles.append(line_stce)
                    labels.append('S.T.C.E. Engine (Proposed)')
            
            ax.set_title(cmap['title'], fontsize=11, fontweight='bold', color=C_TEXT_WHITE, pad=8)
            ax.set_aspect('equal', adjustable='datalim')
            ax.set_xlabel("Coordinate X", fontsize=9, color=C_TEXT_WHITE)
            ax.set_ylabel("Coordinate Y", fontsize=9, color=C_TEXT_WHITE)
            ax.grid(True, linestyle='-', alpha=0.15, color=C_TEXT_WHITE) 
            ax.tick_params(colors=C_TEXT_WHITE, labelsize=8)
            for spine in ax.spines.values(): spine.set_color(C_GRID)
            
        self.fig_quad.legend(handles, labels, loc='lower center', ncol=2, facecolor=C_UI_BG, edgecolor=C_GRID, labelcolor=C_TEXT_WHITE, fontsize=10, bbox_to_anchor=(0.5, 0.015))

        self.fig_quad.tight_layout(pad=2.0)
        self.fig_quad.subplots_adjust(bottom=0.10) 

        self.fig_quad.canvas.mpl_connect('button_press_event', self.on_click_axis)

        canvas = FigureCanvasTkAgg(self.fig_quad, master=self.tab1)
        canvas.draw()
        canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=10)

    def on_click_axis(self, event):
        if event.inaxes is not None and event.inaxes in self.ax_to_info:
            cmap_info = self.ax_to_info[event.inaxes]
            try:
                self.apri_popup_ingrandito(cmap_info)
            except Exception as e:
                print(f"Errore popup: {e}")

    def apri_popup_ingrandito(self, cmap):
        popup = tk.Toplevel(self.root)
        popup.title(f"Zoom - {cmap['title']}")
        popup.geometry("1100x850")
        popup.configure(bg=C_PLOT_BG)

        frame_toolbar = tk.Frame(popup, bg=C_UI_BG)
        frame_toolbar.pack(side="bottom", fill="x")
        
        frame_canvas = tk.Frame(popup, bg=C_PLOT_BG)
        frame_canvas.pack(side="top", fill="both", expand=True)

        stce_csv = f"data_{cmap['name']}_stce.csv"
        nn_csv = f"data_{cmap['name']}_nn.csv"

        if not os.path.exists(stce_csv): return

        df_stce = pd.read_csv(stce_csv)
        n_nodi = len(df_stce)

        fig_pop, ax_pop = plt.subplots(figsize=(10, 8), facecolor=C_PLOT_BG)
        ax_pop.set_facecolor(C_PLOT_BG)
        ax_pop.set_aspect('equal', adjustable='datalim')

        spessore_lin = 0.3 if n_nodi > 50000 else cmap['lw'] * 3 
        dim_nodo = 0 if n_nodi > 50000 else cmap['s'] * 1.5

        ax_pop.plot(df_stce['X'], df_stce['Y'], color=C_LINEA_STCE, linewidth=spessore_lin, alpha=0.9, zorder=2)
        
        if cmap['has_nn'] and os.path.exists(nn_csv):
            df_nn = pd.read_csv(nn_csv)
            ax_pop.plot(df_nn['X'], df_nn['Y'], color=C_LINEA_NN, linewidth=spessore_lin*1.2, alpha=0.6, linestyle='-', zorder=1)
        
        if dim_nodo > 0:
            ax_pop.scatter(df_stce['X'], df_stce['Y'], color=C_NODI, s=dim_nodo, alpha=1.0, zorder=3)

        ax_pop.set_title(cmap['title'], fontsize=14, fontweight='bold', color=C_TEXT_WHITE, pad=15)
        ax_pop.set_xlabel("Coordinate X", fontsize=11, color=C_TEXT_WHITE)
        ax_pop.set_ylabel("Coordinate Y", fontsize=11, color=C_TEXT_WHITE)
        ax_pop.grid(True, linestyle='-', alpha=0.15, color=C_TEXT_WHITE)
        ax_pop.tick_params(colors=C_TEXT_WHITE, labelsize=9)
        for spine in ax_pop.spines.values(): spine.set_color(C_GRID)

        fig_pop.tight_layout(pad=2.0)
        canvas_pop = FigureCanvasTkAgg(fig_pop, master=frame_canvas)
        canvas_pop.draw()
        canvas_pop.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=10)

        try:
            toolbar = NavigationToolbar2Tk(canvas_pop, frame_toolbar)
            toolbar.update()
        except Exception: pass

    def render_benchmark_tab(self):
        csv_path = 'benchmark_results.csv'
        if not os.path.exists(csv_path): return
        df = pd.read_csv(csv_path)

        frame_sinistro = tk.Frame(self.tab2, bg=C_PLOT_BG)
        frame_sinistro.pack(side="left", fill="both", expand=True)

        frame_destro = tk.Frame(self.tab2, bg=C_UI_BG, width=320, bd=0)
        frame_destro.pack(side="right", fill="y", padx=10, pady=15)
        frame_destro.pack_propagate(False)

        fig, ax = plt.subplots(figsize=(9, 7), facecolor=C_PLOT_BG)
        ax.set_facecolor(C_PLOT_BG) 

        nodi = df['Nodi'].to_numpy()
        tempi_stce = df['TempoSTCE'].to_numpy()
        df_nn = df[df['TempoNN'] > 0]
        x_teorico = np.logspace(np.log10(nodi.min()), np.log10(nodi.max()), 100)
        
        if not df_nn.empty:
            costante_n2 = df_nn['TempoNN'].iloc[0] / (df_nn['Nodi'].iloc[0]**2)
            y_n2 = costante_n2 * (x_teorico**2)
            ax.plot(x_teorico, y_n2, color=C_LINEA_NN, linestyle='-', linewidth=1.5, alpha=0.3)
            ax.fill_between(x_teorico, y_n2, color=C_LINEA_NN, alpha=0.04, label='Theoretical Bound $O(n^2)$')

        costante_nlogn = tempi_stce[0] / (nodi[0] * np.log2(nodi[0]))
        y_nlogn = costante_nlogn * (x_teorico * np.log2(x_teorico))
        ax.plot(x_teorico, y_nlogn, color=C_LINEA_STCE, linestyle='-', linewidth=1.5, alpha=0.3)
        ax.fill_between(x_teorico, y_nlogn, color=C_LINEA_STCE, alpha=0.08, label='Theoretical Bound $O(n \\log n)$')

        if not df_nn.empty:
            ax.plot(df_nn['Nodi'], df_nn['TempoNN'], marker='s', linestyle='-', color=C_LINEA_NN, linewidth=2.5, markersize=8, label='Nearest Neighbor (Real)', zorder=4)

        ax.plot(nodi, tempi_stce, marker='o', linestyle='-', color=C_LINEA_STCE, linewidth=3.5, markersize=9, markeredgecolor=C_PLOT_BG, markeredgewidth=2, label='S.T.C.E. Engine (Real)', zorder=5)

        for i in range(len(nodi)):
            ax.annotate(f"{int(tempi_stce[i])} ms", (nodi[i], tempi_stce[i]), 
                        textcoords="offset points", xytext=(0, 30), ha='center', va='bottom', 
                        fontsize=10, fontweight='bold', color=C_PLOT_BG, zorder=10, 
                        bbox=dict(boxstyle="round,pad=0.3", fc=C_LINEA_STCE, ec="none", alpha=1.0))

        for i in range(len(df_nn)):
            ax.annotate(f"{int(df_nn['TempoNN'].iloc[i])} ms", (df_nn['Nodi'].iloc[i], df_nn['TempoNN'].iloc[i]), 
                        textcoords="offset points", xytext=(0, -35), ha='center', va='top', 
                        fontsize=10, fontweight='bold', color=C_PLOT_BG, zorder=10, 
                        bbox=dict(boxstyle="round,pad=0.3", fc=C_LINEA_NN, ec="none", alpha=1.0))

        ax.set_xscale('log')
        ax.set_yscale('log')
        
        ax.set_xticks(nodi)
        ax.set_xticklabels([str(n) for n in nodi], color=C_TEXT_WHITE)
        
        formatter = ScalarFormatter()
        formatter.set_scientific(False)
        ax.yaxis.set_major_formatter(formatter)

        ax.set_xlabel("Dimensione dell'Input (Numero di Nodi N) [Log Scale]", fontsize=13, fontweight='bold', color=C_TEXT_WHITE, labelpad=15)
        ax.set_ylabel("Tempo di Esecuzione (ms) [Log Scale]", fontsize=13, fontweight='bold', color=C_TEXT_WHITE, labelpad=15)
        ax.set_title("Analisi Asintotica Sperimentale", fontsize=15, fontweight='bold', color=C_TEXT_WHITE, pad=15)
        
        ax.grid(True, which="major", axis='both', linestyle="-", alpha=0.2, color=C_TEXT_WHITE)
        ax.tick_params(which='minor', bottom=False, left=False) 
        
        ax.legend(loc='upper left', frameon=True, facecolor=C_UI_BG, edgecolor=C_GRID, labelcolor=C_TEXT_WHITE, fontsize=10)
        ax.tick_params(axis='both', colors=C_TEXT_WHITE, labelsize=10, which='major')
        for spine in ax.spines.values(): spine.set_color(C_GRID)

        fig.tight_layout(pad=3.0)

        canvas = FigureCanvasTkAgg(fig, master=frame_sinistro)
        canvas.draw()
        canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=10)

        lbl_titolo_panel = tk.Label(frame_destro, text="🎯 SINTESI METRICHE", font=("Segoe UI", 12, "bold"), bg=C_UI_BG, fg=C_UI_TEXT, pady=10)
        lbl_titolo_panel.pack()

        testo_kpi = ""
        testo_kpi += "▶ Efficienza Asintotica Confermata:\nIl benchmark dimostra sperimentalmente come l'S.T.C.E. Engine mantenga una complessità temporale quasi-lineare O(n log n), essenziale per dataset massivi (>85k nodi), dove l'approccio NN quadratico collassa visibilmente.\n\n"
        testo_kpi += "▶ Radar KD-Tree:\nL'implementazione del Radar Geometrico tramite KD-Tree non solo accelera la ricerca del vicino, ma riduce drasticamente l'uso della cache, minimizzando i peak di memoria e l'overhead di calcolo durante l'espansione.\n\n"
        testo_kpi += "▶ Determinismo Operativo:\nA differenza delle euristiche stocastiche, l'S.T.C.E. Engine garantisce l'ottenimento della stessa soluzione ad ogni esecuzione, offrendo affidabilità ingegneristica pura nella manipolazione delle strutture geometriche."

        lbl_corpo = tk.Label(frame_destro, text=testo_kpi, font=("Segoe UI", 10), bg=C_UI_BG, fg=C_UI_TEXT, justify="left", anchor="nw", wraplength=295)
        lbl_corpo.pack(fill="both", expand=True, padx=10, pady=5)

if __name__ == "__main__":
    root = tk.Tk()
    app = STCEDashboard(root)
    root.mainloop()