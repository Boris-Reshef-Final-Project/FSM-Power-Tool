#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

int  KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &dest);
void MakeIODecleration(ifstream &source, ofstream &dest);
void MakeTypeState(ifstream &source, ofstream &dest);
void FSM2Process(int j, ofstream &dest);
void Fill_state_product(ifstream &source, ofstream &dest);
bool isStringInVector(const string& str, const vector<string>& vec);
int find_cfsm(string state);

struct state_product {
    string x;
    string cs;
    string ns;
    string y;
};

vector<state_product> stateProducts;
vector<vector<string>> cfsm; // Vector to store the states of each cfsm


// Create a Global state_product instance
        state_product sp;

int main()
{
    string ProjectFolder;
    string SourceFile;
    // Get destination and source files from user
    cout << "\nEnter the path of the source file: " << endl;
    getline(cin, ProjectFolder);
    cout << "\nEnter file name: " << endl;
    getline(cin, SourceFile);

    // Open source and dest files
    ifstream source(ProjectFolder + "/" + SourceFile);
    ofstream dest(ProjectFolder + "/dest.vhd");

    if (!source || !dest)
    {
        cerr << "Error opening files!" << endl;
        return 1;
    }

    int CfsmAmount = 2;
    cout << "insert amount of cfsm's :" << endl;
    cin >> CfsmAmount;

    // This is an assignment for prototyping. delete later!
    cfsm.push_back({"st0", "st1", "st2", "st3", "st4", "st5", "st6"});
    cfsm.push_back({"st7", "st8", "st9", "st10", "st11", "st12"});

    KissFiles2Vhd(CfsmAmount, source, dest); // Preform the parsing process

    // Closing files.
    source.close();
    dest.close();

    return 0;
}



int KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &dest) // Main Parser function - Convert Kiss to Vhd
{
    // ####################################################################################################################
    //                      writing all the librarys to destination file.
    // ####################################################################################################################

    dest << "library ieee;" << endl;
    dest << "use ieee.std_logic_1164.all;" << endl;
    dest << "use ieee.std_logic_arith.all;" << endl;
    dest << "use ieee.std_logic_unsigned.all;" << endl;
    dest << "\nentity state_machine is" << endl;
    dest << "port(" << endl;
    dest << "\t" << "rst\t\t: in\tstd_logic;" << endl;
    dest << "\t" << "clk\t\t: in\tstd_logic_vector(" << CfsmAmount - 1 << " downto 0);" << endl;
    MakeIODecleration(source, dest);
    dest << ");" << endl;
    dest << "end entity state_machine;" << endl;
    dest << "\narchitecture arc_state_machine of state_machine is" << endl;

    source.clear();
    source.seekg(0, ios::beg); // Return to the beginning of the file

    MakeTypeState(source, dest); // type state is (st0, st1, st2,..., st12);
                                 // signal st : state;
    dest << "begin" << endl << endl;
                                 
    source.clear();
    source.seekg(0, ios::beg); // return to the beginning of the file
    
    cout << "Now starting Fill_state_product"<< endl;
    Fill_state_product(source, dest);
    cout << "Now Finished Fill_state_product"<< endl;

    int j;
    for (j = 0; j < CfsmAmount; j++)
    {
        dest << "\n\t" << "cfsm" << j << ": process(clk(" << j << "), rst)" << endl;
        dest << "\t\t" << "begin\n\n";
        dest << "\t" << "if(rst = '1') then"<< endl;
        dest << "\t\t" << "st\t<=\tst0;" << endl;
        dest << "\t" << "elsif falling_edge(clk(" << j << ")) then" << endl;

        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

        FSM2Process(j, dest);

        dest << "\t\t " << "end case;" << endl;
        dest << "\t" << "end if;" << endl;
        dest << "end process cfsm" << j << ";\n\n";
    }

    return 0;
}



void MakeIODecleration(ifstream &source, ofstream &dest) // 
{
    if (source.is_open() && dest.is_open())
    {
        string line;
        while (getline(source, line))
        {
            size_t i_pos = line.find(".i ");
            size_t o_pos = line.find(".o ");

            // Handle ".i" case
            if (i_pos != string::npos)
            {
                size_t startIndex = i_pos + 3; // Skip ".i "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString) - 1;
                dest << "\t" << "x\t\t: in\tstd_logic_vector(" << number << " downto 0);" << endl;
            }

            // Handle ".o" case
            if (o_pos != string::npos)
            {
                size_t startIndex = o_pos + 3; // Skip ".o "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString) - 1;
                dest << "\t" << "y\t\t: out\tstd_logic_vector(" << number << " downto 0);" << endl;
            }
        }
        // Close files
    }
}



void MakeTypeState(ifstream &source, ofstream &dest) // Write the values of the enum type state
{

    if (source.is_open() && dest.is_open())
    {
        string line;
        while (getline(source, line))
        {
            size_t s_pos = line.find(".s ");

            // Handle ".s" case
            if (s_pos != string::npos)
            {
                size_t startIndex = s_pos + 3; // Skip ".s "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString);

                // Write type declaration
                dest << "\t" << "type state is (";
                for (int i = 0; i < number; ++i)
                {
                    dest << "st" << i;
                    if (i < number - 1)
                    {
                        dest << ", ";
                    }
                }
                dest << ");" << endl;

                // Write signal declaration
                dest << "\tsignal st : state;" << endl;
            }
        }
    }
}



void FSM2Process(int j, ofstream &dest)
{
    dest << "\t\t" << "case st is" << endl;
    for(int i = 0; i < cfsm[j].size(); i++) // Iterate over all the states of this specific cfsm
    {
        dest << "\t\t\t" << "when " << cfsm[j][i] << " =>\n\n";
        dest << "\t\t\t\t" << "case x is" << endl;
        for (int k = 0; k < stateProducts.size(); k++) // Iterate over all the state_products
        {
            if (stateProducts[k].cs == cfsm[j][i]) // If the current state_product is in the current state
            {
                dest << "\t\t\t\t\t" << "when \"" << stateProducts[k].x << "\" =>" << endl;
                dest << "\t\t\t\t\t\t" << "st <= " << stateProducts[k].ns << ";" << endl;
                dest << "\t\t\t\t\t\t" << "y <= \"" << stateProducts[k].y << "\";" << endl;
                if(!isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state is not in the current cfsm
                {
                    dest << "\t\t\t\t\t\t" << "z["<< j <<"] <= '0';\n"; // Disable current cfsm clock
                    int z = find_cfsm(stateProducts[k].ns);
                    dest << "\t\t\t\t\t\t" << "z["<< z <<"] <= '0';\n"; // Enable next cfsm clock
                }
            }
        }
        dest << "\t\t\t\t\t" << "when others => NULL;" << endl;
        dest << "\t\t\t\t" << "end case;\n\n";
    }
    dest << "when others => NULL;" << endl;
    dest << "\t" << "end case;" << endl;
}



void Fill_state_product(ifstream &source, ofstream &dest) {
    if (!source.is_open()) {
        cerr << "Error: Unable to open source file." << endl;
        return;
    }

    string line;
    while (getline(source, line)) {
        // Skip lines that start with a period or are empty
        if (line.empty() || line[0] == '.')
            continue;

        
        
        // Extract fields from the line
        istringstream iss(line);
        if (!(iss >> sp.x >> sp.cs >> sp.ns >> sp.y)) {
            cerr << "Error: Failed to parse line: " << line << endl;
            continue;
        }

        // Add the state_product instance to the vector
        stateProducts.push_back(sp);

        // Print the contents of the state_product instance
        cout << "x: " << sp.x << ", cs: " << sp.cs << ", ns: " << sp.ns << ",\ty: " << sp.y << endl;
       
    }
}


// function to check if a string is in a vector of strings
bool isStringInVector(const string& str, const vector<string>& vec) 
{
    return find(vec.begin(), vec.end(), str) != vec.end();
}



// function to find which cfsm the state belongs to
int find_cfsm(string state)
{
    for(int i = 0; i < cfsm.size(); i++)
    {
        if(isStringInVector(state, cfsm[i]))
        {
            return i;
        }
    }
    cerr << "Error: state not found in any cfsm" << endl;
    return -1;
}