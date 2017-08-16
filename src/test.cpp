////////////////////////////////////////////////////////////////////////////////
// alphabet.hpp
// Testprogramm für das Hopcroft Projekt
// Marco Akrutat, 05.05.2016
// Compiler: MSVC++ 14
////////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include "../include/DFA.hpp"

double start;
double end;

int main(int argc, char* argv[]){
  if (argc > 2) {
    std::cerr << "Usage:  'test [tfsm-file]' or 'test < [lexicon-file]'";
    exit(1);
  }
  else if (argc == 2){
    std::ifstream dfa_in(argv[1]);
    
    DFA* dfa = new DFA(dfa_in);
    
    std::cout << *dfa;
    std::cout << std::endl;
    dfa->draw("test.dot");
    
    Hopcroft* minimize = new Hopcroft();
    
    start = clock();
    (*minimize)(*dfa);
    end = clock();
    std::cout << "Minimized in " << (end-start) / CLOCKS_PER_SEC << "s.\n";
    
    std::cout << *dfa;
    std::cout << std::endl;
    dfa->draw("test_minimal.dot");
       
  }
  else{
    DFA* dfa = new DFA;
    
    std::string word;
    while (std::cin.good()) {
      std::getline(std::cin,word);
      dfa->add_word(word);
    }
    
    std::cout << *dfa;
    std::cout << std::endl;
    dfa->draw("test.dot");
    
    Hopcroft* minimize = new Hopcroft;
    
    start = clock();
    (*minimize)(*dfa);
    end = clock();
    std::cout << "Minimized in " << (end-start) / CLOCKS_PER_SEC << "s.\n";
    
    std::cout << *dfa;
    std::cout << std::endl;
    dfa->draw("test_minimal.dot");
  }
}