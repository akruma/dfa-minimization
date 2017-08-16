////////////////////////////////////////////////////////////////////////////////
// DFA.hpp
// Klasse für das Hopcroft Projekt
// Marco Akrutat, 05.05.2016
// Compiler: MSVC++ 14
////////////////////////////////////////////////////////////////////////////////

#ifndef __DFA_HPP__
#define __DFA_HPP__

#include <unordered_set>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "alphabet.hpp"

/// @brief Klasse zur Repräsentation von deterministischen Endlichen Automaten
class DFA
{

  //forward Declaration der Klasse für die Minimierung als Freund der DFAs
  friend class Hopcroft;

  public:

  ///@brief Der Konstruktor der Klasse
  DFA(){
    init();
  }
  
  /** @brief Ein Konstruktor der eine Datei in FSM2's at&t-Format einliest
      @param in ein ifstream einer Datei in FSM2's at&t-Format
  */
  DFA(std::ifstream& in){
    init();
    unsigned n,m;
    char c;
    bool firstline = true;
    std::string line;
    std::string token;
    std::vector<std::string> tokens;
    while(std::getline(in,line)){
      line+='\t';
      //Tokenisierung
      for(int i = 0;i!=line.size();++i){
        if(line[i] != '\t'){
          token+=line[i];
        }
        else{
          tokens.push_back(token);
          token.clear();
        }
      }
      //Wenn es auf der Zeile nur ein Token gibt...
      if(tokens.size()==1){
        //handelt es sich um einen Endzustand
        final_states.insert(atoi(tokens[0].c_str()));
        enlarge(atoi(tokens[0].c_str()));
        signatures[atoi(tokens[0].c_str())] += 63;
      }
      else{
        //Der erste Zustand der ersten Zeile ist der Startzustand
        if(firstline){
          firstline=false;
          start_state=atoi(tokens[0].c_str());
          enlarge(atoi(tokens[0].c_str()));
        }
        //Einfügen des Alphabetsymbols, Vergrößern der Container, Hinzufügen des Übergangs
        //Aktualisierung der Signatur
        alphabet.add(tokens[2][0]);
        enlarge(atoi(tokens[1].c_str()));
        enlarge(atoi(tokens[0].c_str()));
        transitions[atoi(tokens[0].c_str())][tokens[2][0]] = atoi(tokens[1].c_str());
        transition_count++;
        signatures[atoi(tokens[0].c_str())] += alphabet[tokens[2][0]];
        }
      tokens.clear();
    }
  }
  
  /** @brief Fügt dem Automaten ein Wort hinzu
      @param str Das Wort in Form eines std::string
  */
  void add_word(std::string str){
    unsigned current_state=start_state;
    //Schleife über das Wort
	  for(int i=0; i<str.length();i++){
      //Wenn es keinen Übergang mit dem derzeitigen Zeichen gibt...
      if(transitions[current_state].find(str[i])==transitions[current_state].end()){
        //Hinzufügen des Übergangs und Erweiterung der Signatur
        alphabet.add(str[i]);
        transitions[current_state][str[i]] = state_count;
        signatures[current_state] += alphabet[str[i]];
        //Hinzufügen des neuen Zustands
        //TODO: Eine Funktion schreiben, die immer wenn die Kapazität
        //des Vektors erreicht ist, eine bestimmte Menge Platz reserviert
        //um ständige Reallokation zu verhindern
        transitions.push_back(std::map<char,unsigned>());
        signatures.push_back(Signature());
        current_state = state_count;
        ++state_count;
        ++transition_count;
      }
      //Wenn es einen Übergang mit dem derzeitigen Zeichen gibt...
      else{
        current_state = transitions[current_state][str[i]];  
      }
    }
    final_states.insert(current_state);
    signatures[current_state] += 63;
  }
  
  /** @brief Schreibt den Automaten im dot-Format in eine Datei
      @param filename Der Name der Datei die den Automaten enthalten soll
  */
  void draw(std::string filename){
    std::ofstream o(filename.c_str());
    //Kopfzeile
    o << "digraph finite_state_machine {\n\trankdir=LR;\n\tsize=\"100\"\n\t";
    //Enzustände
    for(auto it=final_states.begin();it!=final_states.end();++it){
      if(*it==start_state) o << "node [shape = doubleoctagon, label=\"" << *it << "\", fontsize=12] " << *it << ";\n\t";
      else o << "node [shape = doublecircle, label=\"" << *it << "\", fontsize=12] " << *it << ";\n\t";
    }
    //Nicht-Endzustände
    for(int i=0;i!=state_count;++i){
      if(final_states.find(i)==final_states.end()){
        if(i==start_state) o << "node [shape = octagon, label=\"" << i << "\", fontsize=12] " << i << ";\n\t";
        else o << "node [shape = circle, label=\"" << i << "\", fontsize=12] " << i << ";\n\t";
      }
    }
    //Übergänge
    for(int i = 0;i!=state_count;++i){
      for(auto it=transitions[i].begin();it!=transitions[i].end();++it){
        o << i << "  -> " << it->second << "  [ label = \"" << it->first << "\" ];\n\t";
      }
    }
    //Fußzeile
    o << "\n}";	
  }
  
  ///@brief <<Operator um einige Parameter des Automaten auszugeben
  friend std::ostream& operator<<(std::ostream& o, const DFA& l)
  {
    l.print(o);
    return o;
  }
  
  private:
  
  ///	@brief private Funktion zur Vergrößerung des Automaten während dem Einlesen einer Datei
  void enlarge(unsigned n){
    if(n >= state_count){
      state_count = n+1;
      signatures.resize(state_count,Signature());
      transitions.resize(state_count,std::map<char,unsigned>());
    }
  }
  
  /** @brief Eine Signatur, die widerspiegelt für welche Zeichen ein Zustand Übergänge besitzt
             und ob er ein Endzustand ist
  */
  struct Signature{
    Signature(){
      sig = 0;
    }
    //fügt der Signatur das dem Index entsprechende Zeichen hinzu
    void operator+=(unsigned i){
      if(i < 64){
        sig |= (1 << i);
      }
      else{
        std::cerr << "Error: index is too big.\n";
      }
    }
    
    bool operator<(Signature& rhs){
      return sig < rhs.sig;
    }
    
    bool operator!=(Signature& rhs){
      return sig != rhs.sig;
    }
    
    bool operator==(Signature& rhs){
      return sig == rhs.sig;
    }
    
    //Ein Long (stellt hier quasi einen Boolvektor dar)
    long sig;
  };
  
  /// @brief Initialisierungsfunktion
  void init(){
    start_state = 0;
    transition_count = 0;
    state_count = 1;
    signatures.push_back(Signature());
    transitions.push_back(std::map<char,unsigned>());
  }
  
  /// @brief private Hilfsfunktion zur Ausgabe des Automaten auf einem ostream
  void print(std::ostream& o) const
  {
    o << "acceptor, "
      << state_count << " states, "
      << transition_count << " transitions, "
      << final_states.size() << " final states";
  }
  
  //Eine Klasse zur Indexierung der Alphabetszeichen
  Alphabet alphabet; ///< Alphabet zur Indexierung
  
  //Die Signaturen der Zustände
  std::vector<Signature> signatures; ///< Die Signaturen aller Zustände
  
  //Die  Übergänge der Zustände
  std::vector<std::map<char,unsigned>> transitions; ///< Die Übergänge aller Zustände als Maps von Char auf Zielzustände
  
  unsigned start_state; ///< Der Startzustand
  unsigned transition_count; ///< Die Anzahl an Übergängen
  unsigned state_count; ///< Die Anzahl an Zuständen
  
  //Eine Menge der Enzustände um schnell herauszufinden
  //ob ein Zustand ein Endzustand ist. 
  //TODO/Redundant: Man könnte gucken ob in der Signatur
  //das Bit für die Endzustandsmarkierung(63) gesetzt ist.
  std::unordered_set<unsigned> final_states; ///< Eine Menge der Endzustände

};

#include "hopcroft.hpp"

#endif