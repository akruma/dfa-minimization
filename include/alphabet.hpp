////////////////////////////////////////////////////////////////////////////////
// alphabet.hpp
// Klasse für das Hopcroft Projekt
// Marco Akrutat, 05.05.2016
// Compiler: MSVC++ 14
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>

///@brief Klasse zum Abbilden eines Alphabets auf Zahlen
class Alphabet{
	
public:

/** @brief Konstruktor der Klasse
    @param n optional: ein unsigned um die maximale Größe des Alphabets
                       festzulegen. Default = 63
*/
Alphabet(unsigned n=63){
	max_alphabet_size = n;
}

/** @brief Operator [] nimmt ein Symbol in Form eines char und gibt
           den Index in Form eines unsigned zurück
    @param c ein Symbol als char
*/
unsigned operator[](const char& c){
	return char_to_index[c];
}

/** @brief Operator [] nimmt einen Index in Form eines unsigned und gibt
           das Symbol in Form eines char zurück
    @param n ein Index
*/
char operator[](const unsigned& n){
	return index_to_char[n];
}

/** @brief Fügt dem Alphabet ein Symbol hinzu und weist ihm einen Index zu.
           War das Symbol bereits enthalten, passiert nichts.
    @param c ein Symbol als char
*/
void add(char& c){
	if(!(index_to_char.size()==max_alphabet_size)){
    if(char_to_index.find(c)==char_to_index.end()){
      if(index_to_char.size() < max_alphabet_size){
        char_to_index[c]= index_to_char.size();
        index_to_char.push_back(c);
      }
    }
  }
  else{
		std::cerr << "Error: Alphabet is too big for this Instance to handle!\n";
	}
}

private:

std::map<char,unsigned> char_to_index; ///< Eine Map die Symbole auf Indexe abbildet
std::vector<char> index_to_char; ///< Ein Vektor der einen Index auf ein Symbol abbildet
unsigned max_alphabet_size; ///< Die maximale Größe des Alphabets
	
};