
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

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

	dest << std::endl << "library ieee;\nuse ieee.std_logic_1164.all;\nuse ieee.std_logic_arith.all;\nuse ieee.std_logic_unsigned.all;\n\nentity state_machine is\nport(\nclk\t\t\t:in std_logic;\nrst\t\t\t:in std_logic;";
/*
if (source >> word ) {
		dest << word << std::endl;
	}
*/

if (source.is_open() && dest.is_open()) {
        string line;
        while (getline(source, line)) {
            // Check if the line contains ".i 7"
            size_t found = line.find(".i 7");
            if (found != string::npos) {
                // Extract the number following ".i"
                size_t startIndex = found + 3; // Skip ".i "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex); // Find first non-digit character
                string numberString = line.substr(startIndex, endIndex - startIndex); // Extract the number as a string
                int number = stoi(numberString); // Convert the string to an integer
                // Write to destination file
                dest << "\ninputs\t\t\t: in std_logic_vector(" << number << " downto 0);" << endl;
            }
        }
        // Close files
        source.close();
        dest.close();





	return 0; 


    
}
}



    
