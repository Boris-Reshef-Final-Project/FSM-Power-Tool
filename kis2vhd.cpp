
#include <fstream>
#include <iostream>
#include <string>



int main() 
{   
    std::string SourcePath, DestPath;
	
	//C:/Final Project/Parser Prototype 1/Parser/Parser/btat.txt

	std::ifstream source("E:/Final Project/Project Files/bbsse.kis");    
	std::ofstream dest("E:/Final Project/Project Files/dest.txt");    
	if (!source || !dest) {

		std::cerr << "Error opening files!" << std::endl;
		return 1;   
	}

	std::string word; 



/*
	if (source >> word && source >> word) {
		dest << word << std::endl;
	}

	//std::getline(source, word);

	if (source >> word && source >> word) {
		dest << word << std::endl;
	}
	*/

	dest << std::endl << "library ieee;\nuse ieee.std_logic_1164.all;\nuse ieee.std_logic_arith.all;\nuse ieee.std_logic_unsigned.all;\n\nentity state_machine is\nport(";
if (source >> word ) {
		dest << word << std::endl;
	}

	return 0; 


    
}



    
