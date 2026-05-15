CXX = g++
CXXFLAGS = -O3 -std=c++14

# Percorsi dei file sorgente aggiornati
SRCS = src/main.cpp src/geometria.cpp src/motore_stce.cpp src/parser.cpp

TARGET = stce_engine

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) *.exe