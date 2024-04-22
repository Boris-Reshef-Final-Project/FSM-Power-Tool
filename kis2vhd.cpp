#include <fstream>
#include <iostream>
#include <string>

using namespace std;





int KissFiles2Vhd(int i,std::ifstream &source,std::ofstream &dest);
void MakeInputVector(std::ifstream &source,std::ofstream &dest);
void MakeTypeState (std::ifstream &source,std::ofstream &dest); 
void FSM2Process(std::ifstream &source,std::ofstream &dest);
int GetNumFromUser();


int main(){

// Get destination and source files from user

// Open source and dest files
std::ifstream source("E:/Final Project/Project Files/bbsse.kis");    
std::ofstream dest("E:/Final Project/Project Files/dest.vhd");    


	if (!source || !dest) {

		std::cerr << "Error opening files!" << std::endl;
		return 1;   
	}

    int i=2;
    i=GetNumFromUser();



    //Main Parser function - Convert Kiss to Vhd
    KissFiles2Vhd(i,source,dest) ;



    return 0;
}


int KissFiles2Vhd(int i,std::ifstream &source,std::ofstream &dest) 
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

dest << "library ieee;\nuse ieee.std_logic_1164.all;\nuse ieee.std_logic_arith.all;\nuse ieee.std_logic_unsigned.all;\n\nentity state_machine is\nport(\nclk\t\t\t\t:in std_logic_vector("<<i-1<<" downto 0);\nrst\t\t\t\t:in std_logic;\n";
/*
if (source >> word ) {
		dest << word << std::endl;
	}
*/

MakeInputVector(source,dest);

dest << ");\nend entity state_machine;\n\narchitecture arc_state_machine of state_machine is\n"; 

source.clear();
source.seekg(0, ios::beg);

MakeTypeState (source,dest);// type state is (st0, st1, st2,..., st12);
                 // signal st : state;

int j;
for (j=0;j<i;j++) {
dest << "cfsm"<<j<<": process(clk("<<j<<"), rst)\nbegin\n\nif(rst = '1') then\nst\t<=\tst0;\nelsif falling_edge(clk("<<j<<")) then\ncase st is\n";



//####################################################################################################################
//                     FSM To Process. -- Sub function to the main Kiss2Vhd Function.
//##################################################################################################################   

source.clear();
source.seekg(0, ios::beg);

FSM2Process(source,dest);


dest<< "end case;\nend if;\nend process cfsm"<<j<<";\n\n";

}
//####################################################################################################################
//                     Closing Files.
//##################################################################################################################   

source.close();
dest.close();

return 0; 


     
}

void MakeInputVector(std::ifstream &source,std::ofstream &dest){
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
                int number = stoi(numberString)-1;
                dest << "inputs\t\t\t: in\tstd_logic_vector(" << number << " downto 0);" << endl;
            }
            
            // Handle ".o" case
            if (o_pos != string::npos) {
                size_t startIndex = o_pos + 3; // Skip ".o "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString)-1;
                dest << "outputs\t\t\t: out\tstd_logic_vector(" << number << " downto 0);" << endl;
            }
        }
        // Close files
      
}
}

void MakeTypeState (std::ifstream &source,std::ofstream &dest){


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


void FSM2Process(std::ifstream &source,std::ofstream &dest){







    
}

int GetNumFromUser(){
int i;
 cout << "insert amount of cfsm's"<< endl;
cin>>i;
return i;

}
