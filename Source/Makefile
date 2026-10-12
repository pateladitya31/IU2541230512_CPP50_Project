# Linux / macOS / MinGW (make)
CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
SRC      = main.cpp DeliverySystem.cpp FileManager.cpp Order.cpp Route.cpp DeliveryAgent.cpp Customer.cpp Person.cpp Record.cpp Utils.cpp
OUT      = ../Executable/Project

all: $(OUT)
$(OUT): $(SRC) $(wildcard *.h)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)
clean:
	rm -f $(OUT)
