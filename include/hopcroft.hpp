////////////////////////////////////////////////////////////////////////////////
// hopcroft.hpp
// Klasse für das Hopcroft Projekt
// Marco Akrutat, 05.05.2016
// Compiler: MSVC++ 14
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>
#include <ppl.h>

/// @brief Klasse zur Minimierung von deterministischen endlichen Automaten in Form der DFA-Klasse
class Hopcroft{
  
  public:
  
  /** @brief Operator () zur Minimierung eines Automaten
      @param dfa Der zu minimierende Automat
  */
  void operator()(DFA& dfa){
    init(dfa);
    first_step(dfa);
    refine(dfa);
    construct(dfa);
  }
  
  private:
  
  ///	@brief Eine Container Klasse für die Minimierung
  struct HopcroftState{
    
    HopcroftState(unsigned s = 0, std::vector<unsigned*> t = std::vector<unsigned*>()){
      state = s;
      transitions = t;
    }
    
    /**	@brief Der "gleich"-Operator prüft ob die Zustände der beiden HopcroftStates
							 mit den gleichen Symbolen zu den gleichen Partitionen führen
		*/
    bool operator==(HopcroftState& rhs){
      if(transitions.size()!=rhs.transitions.size()) return false;
      else{
        for(auto it = transitions.begin(), it2 = rhs.transitions.begin();
            it != transitions.end(); ++it, ++it2){
          if(**it != **it2) return false;
        }
        return true;
      }
    }
    
    /**	@brief Ein "kleiner"-Operator, ähnlich dem "gleich"-Operator
							 Zum Sortieren der Zustände nach ihren Übergängen
		*/
    bool operator<(HopcroftState& rhs){
      if(transitions.size()!=rhs.transitions.size()) return transitions.size() < rhs.transitions.size();
      else{
        for(auto it = transitions.begin(), it2 = rhs.transitions.begin();
            it != transitions.end(); ++it, ++it2){
          if(**it != **it2) return **it < **it2;
        }
        return false;
      }
    }
    
    unsigned state; ///< Die Nummer des Zustandes den der  HopcroftState widerspiegelt
    std::vector<unsigned*> transitions; ///< Die Übergänge als Pointer auf die Partitionen in die sie führen
  };
  
  ///	@brief Funktionsobjekt zum Sortieren der HopcroftStates während der Initialisierung
  struct Init_compare{
    //Das Funktionsobjekt benötigt einen Pointer auf den Automaten
    //um die Signaturen der Zustände zu lesen und einen auf die
    //Hopcroft-Instanz um die Partitionen zu lesen
    Init_compare(DFA* d,Hopcroft* h){
      dfa = d;
      hop = h;
    }
    bool operator()(HopcroftState* one, HopcroftState* two) const{
      //Wenn die Partitionen unterschiedlich sind...
      if(hop->partitions[one->state] != hop->partitions[two->state]) 
        return hop->partitions[one->state] < hop->partitions[two->state];
      else{
        //Wenn die Signaturen unterschiedlich sind...
        if(dfa->signatures[one->state] != dfa->signatures[two->state])
          return dfa->signatures[one->state] < dfa->signatures[two->state];
        //anderfalls vergleiche die Übergänge
        else return *one < *two;
      }
    }
    DFA* dfa;
    Hopcroft* hop;
  };
  
  /**	@brief Wie Init_compare, lässt die Signaturen jedoch außer Acht,
						 da im Refine-Step alle Zustände einer Partition bereits mit
						 Sicherheit die Übergänge mit den gleichen Symbolen besitzen
	*/
  struct Refine_compare{
    Refine_compare(Hopcroft* h){
      hop = h;
    }
    bool operator()(HopcroftState* one, HopcroftState* two) const{
      //Wenn die Partitionen ungleich sind...
      if(hop->partitions[one->state] != hop->partitions[two->state]) 
        return hop->partitions[one->state] < hop->partitions[two->state];
      //andernfalls vergleiche die Übergänge...
      else return *one < *two;
    }
    Hopcroft* hop;
  };
  
	///	@brief init initialisiert die Datenstrukturen für die Minimierung
  void init(DFA& dfa){
    //Die Vektoren erhalten gleich die richtige Größe um Reallokation vorzubeugen
    states.resize(dfa.transitions.size(),nullptr);
    partitions.resize(dfa.transitions.size(),0);
    next_partitions.resize(dfa.transitions.size(),0);

    //Schleife über alle Zustände
    int end = dfa.transitions.size();
    concurrency::parallel_for(0,end,[&](int i){
      //Die Übergänge des gerade zu bearbeitenden Zustandes
      std::vector<unsigned*> current;
      //Schleife über die jeweiligen Übergänge
      for(auto it = dfa.transitions[i].begin();it!=dfa.transitions[i].end();++it){
        //Fügt dem Vektor die Adresse des Zustandes hinzu in den die Übergänge führen
        current.push_back(&partitions[it->second]);
      }
      //Erstellt den dazugehörigen HopcroftState
      states[i] = new HopcroftState(i,current);
      //anschließend wird current wieder frei gemacht
      current.clear();
      //und die Anfangspartition eingetragen
      //0 für Nicht-Endzustände, 1 für Endzustände
      if(dfa.final_states.find(i) == dfa.final_states.end()) partitions[i] = 0;
      else partitions[i] = 1;
    });

    //Anschließend werden die Zustände sortiert nach:
    //Partitionen > Signaturen > Übergänge
    //Funktionsobjekt: Init_compare
    concurrency::parallel_sort(states.begin(),states.end(),Init_compare(&dfa, this));
    
  }
  
	/**	@brief Hier wird die erste Iteration des klassischen Hopcroft ausgeführt. 
						 Bei dieser Implementierung gehört dieser Schritt genaugenommen zur Initialisierung.
	*/
  void first_step(DFA& dfa){
    
    //Der Partitionszähler wird auf 0 gesetzt. Es sind zwar
    //bereits die Partitionen null und eins gesetzt. Diese werden
    //jedoch nur als Informationen für die tatsächlich Initial-
    //partitionierung benutzt.
    partition_count = 0;
    //Wir merken uns Signatur, Partition und Übergänge des Vorgängers
    //Nachkommentar: Redundant, da wir Übergänge mit dem Operator des
    //HopcroftStates vergleichen und uns diesen merken, aus dem man
    //Signatur und Partition ableiten kann. Speicherersparnis wäre
    //aber absolut minimal, deshalb und wegen der Lesbarkeit habe ich
    //es so gelassen
    DFA::Signature current_sig = dfa.signatures[states[0]->state];
    unsigned current_part = partitions[states[0]->state];
    HopcroftState* current_trans = states[0];
    //Schleife über alle HopcroftStates

    for(int i = 0;i!=states.size();++i){
      if(current_part == partitions[states[i]->state]){
        if(current_sig == dfa.signatures[states[i]->state]){
          if(*current_trans == *(states[i])){
            //Wenn alles übereinstimmt, weise dem Zustand die Partition
            //des Vorgängers zu
            next_partitions[states[i]->state] = partition_count;
          }
          else{
            //in den anderen Fällen...
            partition_count++;
            next_partitions[states[i]->state] = partition_count;
            current_trans = states[i];
          }
        }
        else{
          //öffne eine neue Partition...
          partition_count++;
          next_partitions[states[i]->state] = partition_count;
          current_sig = dfa.signatures[states[i]->state];
          current_trans = states[i];
        }
      }
      else{
        //und merke die Attribute des Vorgängers die sich unterscheiden
        partition_count++;
        next_partitions[states[i]->state] = partition_count;
        current_part = partitions[states[i]->state];
        current_sig = dfa.signatures[states[i]->state];
        current_trans = states[i];
      }
    }
    
    //zuletzt weisen wir die neu ermittelten Partitionen zu    
    partitions = next_partitions;
    
  }
  
	///	@brief Hier werden die Partitionen immer weiter verfeinert, bis keine Änderung mehr geschieht
  void refine(DFA& dfa){
    
    unsigned partition_count_before = partition_count;
    do{
      
      
      //Als erstes Sortieren wir die Zustände, um anschließend
      //neue Partitionen zuzuweisen
      //Im Gegensatz zu Init_compare müssen keine Signaturen verglichen werden
      concurrency::parallel_sort(states.begin(),states.end(),Refine_compare(this));

      partition_count_before = partition_count;
      partition_count = 0;
      unsigned current_part = partitions[states[0]->state];
      HopcroftState* current_trans = states[0];
      
      for(int i = 0;i!=states.size();++i){
        if(current_part == partitions[states[i]->state]){
          if(*current_trans == *(states[i])){
            next_partitions[states[i]->state] = partition_count;
          }
          else{
            partition_count++;
            next_partitions[states[i]->state] = partition_count;
            current_trans = states[i];
          }
        }
        else{
          partition_count++;
          next_partitions[states[i]->state] = partition_count;
          current_trans = states[i];
          current_part = partitions[states[i]->state];
        }
      }

      partitions = next_partitions;
      
    }while(partition_count != partition_count_before);
    
  }
  
	///	@brief Konstruktion des minimalen DFA und Weitergabe an die DFA-Instanz
  void construct(DFA& dfa){
    //Die Container für den neuen minimalen DFA
    unsigned new_start_state;
    unsigned new_state_count = partition_count+1;
    unsigned new_transition_count = 0;
    
    std::vector<DFA::Signature> new_signatures;
    new_signatures.resize(new_state_count,DFA::Signature());
    
    std::vector<std::map<char,unsigned>> new_transitions;
    new_transitions.resize(new_state_count,std::map<char,unsigned>());
    
    std::unordered_set<unsigned> new_final_states;
    
    for(unsigned i = 0;i!=partitions.size();++i){
      if(i == dfa.start_state){
        new_start_state = partitions[i];
      }
      if(dfa.final_states.find(i)!=dfa.final_states.end()){
        new_final_states.insert(partitions[i]);
        new_signatures[partitions[i]] += 63;
      }
      for(auto it = dfa.transitions[i].begin();it!=dfa.transitions[i].end();++it){
        if(new_transitions[partitions[i]].find(it->first)==new_transitions[partitions[i]].end()){
          new_transitions[partitions[i]][it->first] = partitions[it->second];
          new_transition_count++;
          new_signatures[partitions[i]] += dfa.alphabet[it->first];
        }
      }
    }
    
    //Zuweisung des neuen minimalen DFA
    dfa.signatures = new_signatures;
    dfa.transitions = new_transitions;
    dfa.state_count = new_state_count;
    dfa.transition_count = new_transition_count;
    dfa.final_states = new_final_states;
    dfa.start_state = new_start_state;
  }
  
  //Um beim späteren Sortieren nicht zu große Datenmengen bewegen
  //zu müssen habe ich mich hier für Pointer entschieden
  std::vector<HopcroftState*> states; ///< Die Zustände als HopcroftStates.
  
  std::vector<unsigned> partitions;///< Die aktuelle Partitionen in denen sich die jeweiligen Zustände befinden

  std::vector<unsigned> next_partitions; ///< Die Partitionen der Zustände nach der Interation
  
  unsigned partition_count; ///< Zähler der Partitionen
  
};