# ==========================================
# Makefile per l'Architettura STCE
# ==========================================

# Definiamo il compilatore e i flag
CXX = g++

# -O3 attiva l'ottimizzazione matematica estrema in fase di compilazione.
# -pthread prepara il terreno per il multi-threading che faremo tra poco.
CXXFLAGS = -O3 -std=c++14 -pthread 

# Il nome del file eseguibile finale
TARGET = stce_engine

# I file oggetto che verranno creati
OBJS = main.o geometria.o motore_stce.o

# Regola principale: come costruire l'eseguibile unendo i pezzi
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Come compilare i singoli moduli
main.o: main.cpp strutture.h geometria.h motore_stce.h
	$(CXX) $(CXXFLAGS) -c main.cpp

geometria.o: geometria.cpp geometria.h strutture.h
	$(CXX) $(CXXFLAGS) -c geometria.cpp

motore_stce.o: motore_stce.cpp motore_stce.h geometria.h strutture.h
	$(CXX) $(CXXFLAGS) -c motore_stce.cpp

# Comando di pulizia (digita "make clean" per pulire la cartella)
clean:
	rm -f *.o $(TARGET) *.exe