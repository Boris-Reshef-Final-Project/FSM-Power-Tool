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

string Original_Reset_value;        // Original_Reset_value          -> (Fill_state_product)
int input,output,products,states;   // products is (מספר שורות) - 1;-> (MakeIODecleration)
                                    // input,output                  -> (MakeIODecleration)
                                    // states                        -> (MakeTypeState)


int  KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &dest);
void MakeIODecleration(ifstream &source, ofstream &dest);
void MakeTypeState(ifstream &source, ofstream &dest);
void FSM2Process(int j, ofstream &dest);
void Fill_state_product(ifstream &source, ofstream &dest);
bool isStringInVector(const string& str, const vector<string>& vec);
int  find_cfsm(string state);
void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm);
void Optimiser_Min_trans_prob(ifstream &source, vector<vector<string>> &cfsm);

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
    ofstream dest(ProjectFolder + "/state_machine.vhd");

    if (!source || !dest)
    {
        cerr << "Error opening files!" << endl;
        return 1;
    }

    int CfsmAmount = 2;
    //cout << "insert amount of cfsm's :" << endl;
    //cin >> CfsmAmount;

    // This is an assignment for prototyping. delete later!
    //cfsm.push_back({"st0", "st1", "st2", "st3", "st4", "st5", "st6"});
    //cfsm.push_back({"st7", "st8", "st9", "st10", "st11", "st12"});

    


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

    

    MakeTypeState(source, dest); // type state is (st0, st1, st2,..., st12);
                                 // signal st : state;
    dest << "begin" << endl << endl;
                                 
   
    
    cout << "Now starting Fill_state_product"<< endl;
    Fill_state_product(source, dest);
    cout << "Now Finished Fill_state_product"<< endl;

    int Opt;
    cout << "Choose Optimiser: 1 - Optimiser_Axe, 2 - Optimiser_Min_trans_prob"<< endl;
    cin >> Opt;
    if (Opt ==1)
    Optimiser_Axe(source, cfsm); // Located After (Fill_state_product) && Before (FSM2Process) √√√√√√√√√
    else
    Optimiser_Min_trans_prob(source, cfsm);


    int j;
    for (j = 0; j < CfsmAmount; j++)
    {
        dest << "\n\t" << "cfsm" << j << ": process(clk(" << j << "), rst) begin\n" << endl;
        dest << "\t\t" << "if(rst = '1') then"<< endl;
        dest << "\t\t\t" << "st\t<=\tst0;" << endl;
        dest << "\t\t\t" << "z <= (" << find_cfsm("st0") << " => '1', others => '0');" << endl;
        dest << "\t\t" << "elsif falling_edge(clk(" << j << ")) then" << endl;

        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

        FSM2Process(j, dest);

        dest << "\t\t" << "end if;" << endl;
        dest << "\t" <<"end process cfsm" << j << ";\n\n";
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
        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

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

                states = stoi(numberString);


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
    dest << "\t\t\t" << "case st is" << endl;
    for(int i = 0; i < cfsm[j].size(); i++) // Iterate over all the states of this specific cfsm
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
                if(!isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state is not in the current cfsm
                {
                    dest << "\t\t\t\t\t\t\t" << "z("<< j <<") <= '0';" << endl; // Disable current cfsm clock
                    int z = find_cfsm(stateProducts[k].ns);
                    dest << "\t\t\t\t\t\t\t" << "z("<< z <<") <= '1';" << endl; // Enable next cfsm clock
                }
            }
        }
        dest << "\t\t\t\t\t\t" << "when others => NULL;" << endl;
        dest << "\t\t\t\t\t" << "end case?;\n\n";
    }
    dest << "\t\t\t\t" << "when others => NULL;" << endl;
    dest << "\t\t\t" << "end case;" << endl;
}


void Fill_state_product(ifstream &source, ofstream &dest) {
    if (!source.is_open()) {
        cerr << "Error: Unable to open source file." << endl;
        return;
    }

    unordered_map<string, string> stateMap;  // To map states to state names
      // To store state_product instances
    string line;
    

    source.clear();
    source.seekg(0, ios::beg);

    // First pass to find and handle the ".r" value
    while (getline(source, line)) {
        if (line.find(".r ") == 0) {
            istringstream iss(line);
            string temp;
            iss >> temp >> Original_Reset_value;  // Read ".r" and its associated value
            stateMap[Original_Reset_value] = "st0";  // Map the reset state to "st0"
            break;  // We assume there's only one ".r" line
        }
    }

    state_product sp;

    // Rewind to the beginning of the file for the second pass
    source.clear();
    source.seekg(0, ios::beg);

    // Second pass to process the actual state lines
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

        // Map and replace cs if it does not start with 's'
        if (sp.cs != "st0" && stateMap.find(sp.cs) == stateMap.end() && sp.cs[0] != 's') {
            string stateName = "st" + to_string(stateMap.size());
            stateMap[sp.cs] = stateName;
        }

        if (sp.cs[0] != 's') {
            sp.cs = stateMap[sp.cs];
        }

        // Map and replace ns if it does not start with 's'
        if (sp.ns != "st0" && stateMap.find(sp.ns) == stateMap.end() && sp.ns[0] != 's') {
            string stateName = "st" + to_string(stateMap.size());
            stateMap[sp.ns] = stateName;
        }

        if (sp.ns[0] != 's') {
            sp.ns = stateMap[sp.ns];
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

void calculate_transition_probabilities(const vector<state_product>& stateProducts, map<pair<string, string>, double>& transitionProbs) {
    // Initialize the transition counts
    map<pair<string, string>, int> transitionCounts;
    map<string, int> stateCounts;

    // Count the transitions and occurrences of each state
    for (const auto& sp : stateProducts) {
        transitionCounts[{sp.cs, sp.ns}]++;
        stateCounts[sp.cs]++;
    }

    // Calculate transition probabilities
    for (const auto& count : transitionCounts) {
        string cs = count.first.first;
        string ns = count.first.second;
        transitionProbs[{cs, ns}] = static_cast<double>(count.second) / stateCounts[cs];
    }
}


void Optimiser_Min_trans_prob(ifstream &source, vector<vector<string>> &cfsm) {
    if (!source.is_open()) {
        cerr << "Error: Unable to open source file." << endl;
        return;
    }

    map<pair<string, string>, double> transitionProbs;
    calculate_transition_probabilities(stateProducts, transitionProbs);

    vector<string> firstHalf;
    vector<string> secondHalf;

    // Naive partitioning strategy (This can be improved)
    for (int i = 0; i < states; ++i) {
        if (i < states / 2) {
            firstHalf.push_back("st" + to_string(i));
        } else {
            secondHalf.push_back("st" + to_string(i));
        }
    }

    bool improvement = true;
    while (improvement) {
        improvement = false;
        for (const auto& state : firstHalf) {
            double currentP12 = 0.0, currentP21 = 0.0;
            double newP12 = 0.0, newP21 = 0.0;

            // Calculate current probabilities
            for (const auto& ns : secondHalf) {
                currentP12 += transitionProbs[{state, ns}];
                currentP21 += transitionProbs[{ns, state}];
            }

            // Simulate moving the state to the second half
            for (const auto& ns : firstHalf) {
                if (ns != state) {
                    newP12 += transitionProbs[{state, ns}];
                    newP21 += transitionProbs[{ns, state}];
                }
            }
            for (const auto& ns : secondHalf) {
                if (ns != state) {
                    newP12 += transitionProbs[{state, ns}];
                    newP21 += transitionProbs[{ns, state}];
                }
            }

            // Check if moving the state reduces the transition probability
            if (newP12 + newP21 < currentP12 + currentP21) {
                firstHalf.erase(remove(firstHalf.begin(), firstHalf.end(), state), firstHalf.end());
                secondHalf.push_back(state);
                improvement = true;
                break;
            }
        }
    }

    cfsm.push_back(firstHalf);
    cfsm.push_back(secondHalf);
    
}