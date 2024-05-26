#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <unordered_map>
#define OutputBitReplace '0'

using namespace std;

string Original_Reset_value;         // Original_Reset_value          -> (Fill_state_product)
int input, output, products, states; // products is (מספר שורות) - 1;-> (MakeIODecleration)
                                     // input,output                  -> (MakeIODecleration)
                                     // states                        -> (MakeTypeState)
stringstream type_state;
vector<string> State_list;

int KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &dest);
void MakeIODecleration(ifstream &source, ofstream &dest);
void MakeTypeState(ifstream &source, ofstream &dest);
void FSM2Process(int j, ofstream &dest);
void Fill_state_product(ifstream &source, ofstream &dest);
bool isStringInVector(const string &str, const vector<string> &vec);
int find_cfsm(string state);
void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm);
void Optimiser_Min_trans_prob(vector<vector<string>> &cfsm);

struct state_product
{
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
    ofstream dest(ProjectFolder + "/state_machine.vhd");

    if (!source || !dest)
    {
        cerr << "Error opening files!" << endl;
        return 1;
    }

    int CfsmAmount = 2;
    // cout << "insert amount of cfsm's :" << endl;
    // cin >> CfsmAmount;

    // This is an assignment for prototyping. delete later!
    // cfsm.push_back({"st0", "st1", "st2", "st3", "st4", "st5", "st6"});
    // cfsm.push_back({"st7", "st8", "st9", "st10", "st11", "st12"});

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
    dest << "\tsignal z\t\t: std_logic_vector(" << CfsmAmount - 1 << " downto 0);" << endl;

    cout << "Now starting Fill_state_product" << endl;
    Fill_state_product(source, dest); // state_list = ("st0,st1...,stn")
    cout << "Now Finished Fill_state_product" << endl;

    MakeTypeState(source, dest); // type state is (st0, st1, st2,..., st12);
                                 // signal st : state;
    dest << "begin" << endl
         << endl;

    int Opt;
    cout << "Choose Optimiser: 1 - Optimiser_Axe, 2 - Optimiser_Min_trans_prob" << endl;
    cin >> Opt;
    if (Opt == 1)
        Optimiser_Axe(source, cfsm); // Located After (Fill_state_product) && Before (FSM2Process) √√√√√√√√√
    else
        Optimiser_Min_trans_prob(cfsm);

    int j;
    for (j = 0; j < CfsmAmount; j++)
    {
        dest << "\n\t" << "cfsm" << j << ": process(clk(" << j << "), rst) begin\n"
             << endl;
        dest << "\t\t" << "if(rst = '1') then" << endl;
        dest << "\t\t\t" << "st\t<=\tst0;" << endl;
        dest << "\t\t\t" << "z <= (" << find_cfsm("st0") << " => '1', others => '0');" << endl;
        dest << "\t\t" << "elsif falling_edge(clk(" << j << ")) then" << endl;

        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

        FSM2Process(j, dest);

        dest << "\t\t" << "end if;" << endl;
        dest << "\t" << "end process cfsm" << j << ";\n\n";
    }
    dest << "end arc_state_machine;" << endl;
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
            size_t p_pos = line.find(".p ");
            size_t s_pos = line.find(".s ");
            // Handle ".i" case
            if (i_pos != string::npos)
            {
                size_t startIndex = i_pos + 3; // Skip ".i "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString) - 1;
                input = stoi(numberString) - 1;
                dest << "\t" << "x\t\t: in\tstd_logic_vector(" << number << " downto 0);" << endl;
            }

            // Handle ".o" case
            if (o_pos != string::npos)
            {
                size_t startIndex = o_pos + 3; // Skip ".o "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString) - 1;
                output = stoi(numberString) - 1;
                dest << "\t" << "y\t\t: out\tstd_logic_vector(" << number << " downto 0)" << endl;
            }

            if (s_pos != string::npos)
            {
                size_t startIndex = s_pos + 3; // Skip ".s "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);
                int number = stoi(numberString);

                states = stoi(numberString);
            }

            if (p_pos != string::npos)
            {
                size_t startIndex = p_pos + 3; // Skip ".p "
                size_t endIndex = line.find_first_not_of("0123456789", startIndex);
                string numberString = line.substr(startIndex, endIndex - startIndex);

                products = stoi(numberString) - 1;
            }
        }
    }
}

void MakeTypeState(ifstream &source, ofstream &dest) // Write the values of the enum type state
{
    if (source.is_open() && dest.is_open())
    {

       
       
            type_state << "\t" << "type state is (";
            for (int i = 0; i < states; ++i)
            {

                type_state << State_list[i];
                if (i < states - 1)
                {

                    type_state << ", ";
                }
            }

            type_state << ");" << endl;
            dest << type_state.str();

            // Write signal declaration
            dest << "\tsignal st : state;" << endl;
        }
    }


void FSM2Process(int j, ofstream &dest)
{
    dest << "\t\t\t" << "case st is" << endl;
    for (int i = 0; i < cfsm[j].size(); i++) // Iterate over all the states of this specific cfsm
    {
        dest << "\t\t\t\t" << "when " << cfsm[j][i] << " =>\n\n";
        dest << "\t\t\t\t\t" << "case? x is" << endl;
        for (int k = 0; k < stateProducts.size(); k++) // Iterate over all the state_products
        {
            if (stateProducts[k].cs == cfsm[j][i]) // If the current state_product is in the current state
            {
                dest << "\t\t\t\t\t\t" << "when \"" << stateProducts[k].x << "\" =>" << endl;
                dest << "\t\t\t\t\t\t\t" << "st <= " << stateProducts[k].ns << ";" << endl;

                string modified_y = stateProducts[k].y;
                replace(modified_y.begin(), modified_y.end(), '-', OutputBitReplace);

                dest << "\t\t\t\t\t\t\t" << "y <= \"" << modified_y << "\";" << endl;
                if (!isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state is not in the current cfsm
                {
                    dest << "\t\t\t\t\t\t\t" << "z(" << j << ") <= '0';" << endl; // Disable current cfsm clock
                    int z = find_cfsm(stateProducts[k].ns);
                    dest << "\t\t\t\t\t\t\t" << "z(" << z << ") <= '1';" << endl; // Enable next cfsm clock
                }
            }
        }
        dest << "\t\t\t\t\t\t" << "when others => NULL;" << endl;
        dest << "\t\t\t\t\t" << "end case?;\n\n";
    }
    dest << "\t\t\t\t" << "when others => NULL;" << endl;
    dest << "\t\t\t" << "end case;" << endl;
}

void Fill_state_product(ifstream &source, ofstream &dest)
{

    unordered_map<string, string> stateMap; // To map states to state names
                                            // To store state_product instances
    string line;
    vector<string> Test_Format_list;

    for (int i = 0; i < states; ++i)
    {
        Test_Format_list.push_back("st" + to_string(i));
    }

    source.clear();
    source.seekg(0, ios::beg);

    // First pass to find and handle the ".r" value
    while (getline(source, line))
    {
        if (line.find(".r") != string::npos)
        {
            istringstream iss(line);
            string temp;
            iss >> temp >> Original_Reset_value;    // Read ".r" and its associated value
            stateMap[Original_Reset_value] = "st0"; // Map the reset state to "st0"
            State_list.push_back("st0");
            break; // We assume there's only one ".r" line
        }
    }

    state_product sp;

    // Rewind to the beginning of the file for the second pass
    source.clear();
    source.seekg(0, ios::beg);

    // Second pass to process the actual state lines
    while (getline(source, line))
    {
        // Skip lines that start with a period or are empty
        if (line.empty() || line[0] == '.')
            continue;

        // Extract fields from the line
        istringstream iss(line); // 0000 0101 1010 1111
        if (!(iss >> sp.x >> sp.cs >> sp.ns >> sp.y))
        { //      st5
            cerr << "Error: Failed to parse line: " << line << endl;
            continue;
        }

        // Map and replace cs if it does not start with 's'
        if (stateMap.find(sp.cs) == stateMap.end() )
        {
            if (!isStringInVector(sp.cs, Test_Format_list)){
            string stateName = "st" + to_string(stateMap.size());
            stateMap[sp.cs] = stateName;
            State_list.push_back(stateName);
            sp.cs = stateMap[sp.cs];

            }
            else {
                stateMap[sp.cs]=sp.cs;
                State_list.push_back(sp.cs);
            }
           
        }
        else {
            sp.cs =  stateMap[sp.cs];
        }
        

        // if (sp.cs[0] != 's') {
      
        //}

        // Map and replace ns if it does not start with 's'
         if (stateMap.find(sp.ns) == stateMap.end() )
        {
            if (!isStringInVector(sp.ns, Test_Format_list)){
            string stateName = "st" + to_string(stateMap.size());
            stateMap[sp.ns] = stateName;
            State_list.push_back(stateName);
            sp.ns = stateMap[sp.ns];

            }
            else {
                stateMap[sp.ns]=sp.ns;
                State_list.push_back(sp.ns);
            }
           
        }
        else {
            sp.ns =  stateMap[sp.ns];
        }

        // Add the state_product instance to the vector
        stateProducts.push_back(sp);

        // Print the contents of the state_product instance
        cout << "x: " << sp.x << ", cs: " << sp.cs << ", ns: " << sp.ns << ",\ty: " << sp.y << endl;
    }
}

// function to check if a string is in a vector of strings
bool isStringInVector(const string &str, const vector<string> &vec)
{
    return find(vec.begin(), vec.end(), str) != vec.end();
}

// function to find which cfsm the state belongs to
int find_cfsm(string state)
{
    for (int i = 0; i < cfsm.size(); i++)
    {
        if (isStringInVector(state, cfsm[i]))
        {
            return i;
        }
    }
    cerr << "Error: state not found in any cfsm" << endl;
    return -1;
}

void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm)
{
    if (source.is_open())
    {
        source.clear();
        source.seekg(0, ios::beg);

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

                vector<string> firstHalf;
                vector<string> secondHalf;

                for (int i = 0; i < number; ++i)
                {
                    if (i < number / 2)
                    {
                        firstHalf.push_back("st" + to_string(i));
                    }
                    else
                    {
                        secondHalf.push_back("st" + to_string(i));
                    }
                }

                cfsm.push_back(firstHalf);
                cfsm.push_back(secondHalf);
            }
        }
    }
    source.clear();
    source.seekg(0, ios::beg);
}

void calculate_transition_probabilities_V1(const vector<state_product> &stateProducts, map<pair<string, string>, double> &transitionProbs)
{
}

void Optimiser_Min_trans_prob(vector<vector<string>> &cfsm)
{
}