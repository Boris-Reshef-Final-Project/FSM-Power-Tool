#include <fstream>
#include <iostream>
#include <string>

using namespace std;

std::ifstream source("E:/Final Project/Project Files/bbsse.kis");    
std::ofstream dest("E:/Final Project/Project Files/dest.txt");    






int KissFiles2Vhd();
void MakeInputVector();
void MakeTypeState (); 
void FSM2Process();

int main(){

// Get destination and source files from user



//Main Parser function - Convert Kiss to Vhd
KissFiles2Vhd() ;



    return 0;
}


int KissFiles2Vhd() 
{   
    std::string SourcePath, DestPath;
	
	//C:/Final Project/Parser Prototype 1/Parser/Parser/btat.txt

  
	if (!source || !dest) {

		std::cerr << "Error opening files!" << std::endl;
		return 1;   
	}

	std::string word; 

//####################################################################################################################
         //                     writing all the librarys to destination file.
  //##################################################################################################################   

dest << "library ieee;\nuse ieee.std_logic_1164.all;\nuse ieee.std_logic_arith.all;\nuse ieee.std_logic_unsigned.all;\n\nentity state_machine is\nport(\nclk\t\t\t:in std_logic;\nrst\t\t\t:in std_logic;\n";
/*
if (source >> word ) {
		dest << word << std::endl;
	}
*/

MakeInputVector();

dest << ");\nend entity state_machine;\n\narchitecture arc_state_machine of state_machine is\n"; 

source.clear();
source.seekg(0, ios::beg);

MakeTypeState ();// type state is (st0, st1, st2,..., st12);
                 // signal st : state;

//####################################################################################################################
//                     FSM To Process. -- Sub function to the main Kiss2Vhd Function.
//##################################################################################################################   

FSM2Process();

//####################################################################################################################
//                     Closing Files.
//##################################################################################################################   

source.close();
dest.close();

return 0; 


     
}

void MakeInputVector(){
if (source.is_open() && dest.is_open()) {
        string line;
        while (getline(source, line)) {
            size_t i_pos = line.find(".i ");
            size_t o_pos = line.find(".o ");
            
            // Handle ".i" case
            if (i_pos != string::npos) {
                size_t startIndex = i_pos + 3; // Skip ".i "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString);
                dest << "inputs\t\t\t: in\tstd_logic_vector(" << number << " downto 0);" << endl;
            }
            
            // Handle ".o" case
            if (o_pos != string::npos) {
                size_t startIndex = o_pos + 3; // Skip ".o "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString);
                dest << "outputs\t\t\t: out\tstd_logic_vector(" << number << " downto 0);" << endl;
            }
        }
        // Close files
      
}
}

void MakeTypeState (){


 if (source.is_open() && dest.is_open()) {
        string line;
        while (getline(source, line)) {
            size_t s_pos = line.find(".s ");
            
            // Handle ".s" case
            if (s_pos != string::npos) {
                size_t startIndex = s_pos + 3; // Skip ".s "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString);
                
                // Write type declaration
                dest << "type state is (";
                for (int i = 0; i < number; ++i) {
                    dest << "st" << i;
                    if (i < number - 1) {
                        dest << ", ";
                    }
                }
                dest << ");" << endl;
                
                // Write signal declaration
                dest << "signal st : state;" << endl;
            }
        }
 }



}


void FSM2Process(){







    
}


