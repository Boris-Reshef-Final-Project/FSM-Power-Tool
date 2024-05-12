#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

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
    cfsm.push_back({"st0", "st1"});
    cfsm.push_back({"st2", "st3"});

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

    dest << "library ieee;\nuse ieee.std_logic_1164.all;\nuse ieee.std_logic_arith.all;\nuse ieee.std_logic_unsigned.all;\n\nentity state_machine is\nport(\n\trst\t\t: in\tstd_logic;\n\tclk\t\t: in\tstd_logic_vector(" << CfsmAmount - 1 << " downto 0);\n";
    MakeIODecleration(source, dest);

    dest << ");\nend entity state_machine;\n\narchitecture arc_state_machine of state_machine is\n";

    source.clear();
    source.seekg(0, ios::beg); // Return to the beginning of the file

    MakeTypeState(source, dest); // type state is (st0, st1, st2,..., st12);
                                 // signal st : state;
                                 
    source.clear();
    source.seekg(0, ios::beg);
    
    cout << "Now starting Fill_state_product"<< endl;
    Fill_state_product(source, dest);
    cout << "Now Finished Fill_state_product"<< endl;

    int j;
    for (j = 0; j < CfsmAmount; j++)
    {
        dest << "cfsm" << j << ": process(clk(" << j << "), rst)\nbegin\n\nif(rst = '1') then\nst\t<=\tst0;\nelsif falling_edge(clk(" << j << ")) then\ncase st is\n";

        // ####################################################################################################################
        //                      FSM To Process. -- Sub function to the main Kiss2Vhd Function.
        // ####################################################################################################################

        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

        FSM2Process(j, dest);

        dest << "end case;\nend if;\nend process cfsm" << j << ";\n\n";
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
                dest << "\tx\t\t: in\tstd_logic_vector(" << number << " downto 0);" << endl;
            }

            // Handle ".o" case
            if (o_pos != string::npos)
            {
                size_t startIndex = o_pos + 3; // Skip ".o "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString) - 1;
                dest << "\ty\t\t: out\tstd_logic_vector(" << number << " downto 0);" << endl;
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
                dest << "type state is (";
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
                dest << "signal st : state;" << endl;
            }
        }
    }
}



void FSM2Process(int j, ofstream &dest)
{
    for(int i = 0; i < cfsm[j].size(); i++) // Iterate over all the states of this specific cfsm
    {
        dest << "when " << cfsm[j][i] << " =>\n\n";
        dest << "\tcase x is\n";
        for (int k = 0; k < stateProducts.size(); k++) // Iterate over all the state_products
        {
            if (stateProducts[k].cs == cfsm[j][i]) // If the current state_product is in the current state
            {
                dest << "\t\twhen \"" << stateProducts[k].x << "\" =>\n";
                dest << "\t\t\tst <= " << stateProducts[k].ns << ";\n";
                dest << "\t\t\ty <= \"" << stateProducts[k].y << "\";\n";
                if(!isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state is not in the current cfsm
                {
                    dest << "\t\t\tz["<< j <<"] <= '0';\n"; // Disable current cfsm clock
                    int z = find_cfsm(stateProducts[k].ns);
                    dest << "\t\t\tz["<< z <<"] <= '0';\n"; // Enable next cfsm clock
                }
            }
        }
        dest << "end case;\n\n";
    }
    dest << "when others => NULL;\n";
    dest << "end case;\n";
}



void Fill_state_product(ifstream &source, ofstream &dest) {
     if (source.is_open()) 
    {
        string line;
        while (getline(source, line)) 
        {
            // Skip lines that start with a period
            if (line[0] == '.')
                continue;
            
            // Create a new state_product instance
            state_product sp;
            sp.x = line.substr(0, 7);
            sp.cs = line.substr(8, 3);
            sp.ns = line.substr(12, 3);
            sp.y = line.substr(16);
            
            // Add the state_product instance to the vector
            stateProducts.push_back(sp);
            
            // Print the contents of the state_product instance
            cout << "x: " << sp.x << ", cs: " << sp.cs << ", ns: " << sp.ns << ", y: " << sp.y << endl;
        }
       
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