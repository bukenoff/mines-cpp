CXXFLAGS += -std=c++17
my_program: main.cpp
	echo "CXXFLAGS: $(CXXFLAGS)"
	clang++ -o main main.cpp $(CXXFLAGS) -lncurses

clean:
	rm -f main 
