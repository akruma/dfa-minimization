# Make-Datei f�r das Hopcroft-Projekt
# Verwendet Befehle (rm, mv) aus gnuwin32
# und ben�tigt graphviz f�r das Zeichnen der Automaten.
# Zum Testen: "make test"

# Zun�chst werden mal einige Zeichenkettenvariablen definiert
# (dies ist streng genommen nicht notwendig, erleichtert aber die Pflege der Make-Datei)
# Variablen werden weiter unten mit der $(...)-Notation wieder aufgenommen.

# Quelldateien
Test_CPP = src/test.cpp
# Include-Dateien
Hopcroft_HPP = include/DFA.hpp include/hopcroft.hpp include/alphabet.hpp
# Ausf�hrbare Programme
Test_BIN = bin/test.exe
# Doxygen-Datei
Hopcroft_DOC_INDEX = doc/html/index.html

# tfsm-Datei
TFSMFILE = data/fang.tfsm


# Verwendeter Compiler
CPPCOMPILER = cl
CPPCOMPILEROPTIONS = /Ox /EHsc

################################################################################
# Ab hier kommen die Targets. Das erste ist das Haupt-Target.
################################################################################

# Main-Target: ausf�hrbare Datei (hier mit Variablen realisiert)
$(Test_BIN) : $(Test_CPP) $(Hopcroft_HPP)
	$(CPPCOMPILER) $(CPPCOMPILEROPTIONS) $(Test_CPP)
	mv test.exe bin

# Dokumentations-Target
documentation: $(Hopcroft_DOC_INDEX)

$(Hopcroft_DOC_INDEX):
	doxygen doc/hopcroft.doxygen
# 	html-Ordner nach doc verschieben
	mv html doc

# Test-Target
# 'test' h�ngt vom ausf�hrbaren Programm ab (dieses wird also zuerst erzeugt)
test: $(Test_BIN)
	$(Test_BIN) $(TFSMFILE)
	dot -Tpdf -o test.pdf test.dot
	dot -Tpdf -o test_minimal.pdf test_minimal.dot

# clean-Target: alles aufr�umen
clean:
	rm -f $(Test_BIN) *.obj chart.html
	rm -rf doc/html
