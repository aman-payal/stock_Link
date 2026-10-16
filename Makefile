CC = gcc
CXX = g++
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2

C_SOURCES = main.c utils.c nikhil_inventory.c omkar_auth.c \
            aman_orders_suppliers.c rahul_relationships.c ayushman_algorithms.c
CPP_SOURCES = stocklink_oop.cpp nikhil_inventory_oop.cpp omkar_auth_oop.cpp \
              aman_orders_oop.cpp rahul_relationships_oop.cpp ayushman_algorithms_oop.cpp
OBJECTS = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o)

stocklink: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)

%.o: %.c stocklink.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp stocklink.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f stocklink *.o sl_inventory.dat stocklink_report.txt

.PHONY: clean
