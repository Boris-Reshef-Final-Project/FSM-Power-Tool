/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
File:           kis2vhd.cpp
Authors:        Boris Karasov, Reshef Schachter
Date:           2023-2024
Institution:    Afeka College of Engineering
Description:    This is our final project for our Bachelor's degree in Electrical Engineering.
Notes:          The project is meant to work with a VHDL2008 compiler and a C++17 compiler.
File Description: This file is the main file of the project. It is resposible for parsing KIS to VHDL and optimizing the FSM.
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------**/

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <unordered_map>
#include <cmath>
#include <filesystem>
#include <cerrno>
#include <cstring>
#include <Windows.h>
#include <libloaderapi.h>
#include <functional>
#include <chrono>

#define OutputBitReplace '0'

using namespace std;

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*===============================*/
/*      Global Declarations      */
/*===============================*/

// Structure to store results
struct Result {
    vector<string> cfsm0;
    vector<string> cfsm1;
    double pt1;
    double pt2;
    double sum_pt;
    };

struct state_product
{
    string x;
    string cs;
    string ns;
    string y;
};

vector<state_product> stateProducts;
vector<vector<string>> cfsm; // Vector to store the states of each cfsm
state_product sp; // Create a Global state_product instance
string ProjectFolder;
string SourceName;
string NewLocation,NewLocation2;
string templateFolder    = "tb-template";   // Name of template folder
string destinationFolder = "optimised";     // Name of destination folder
string destinationFolder2 = "not-optimised";
stringstream type_state;                    // String stream to store the type state line
vector<string> State_list;                  // Vector to store the state names
vector<string> testarray;                   // Vector to test array for the TB
string Original_Reset_state_code;           // The name (code) of the original reset state in the Kiss file
int input, output, products, states;        // Number of inputs, outputs, products, and states as declared in the Kiss file preamble

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*===============================*/
/*      Function Prototypes      */
/*===============================*/

int  KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &destin);
void MakeIODecleration(ifstream &source, ofstream &destin);
void MakeTypeState(ifstream &source, ofstream &destin, const vector<vector<string>> &cfsm);
void MakeTypeState2(ifstream &source, ofstream &dest);
void FSM2Process(int j, ofstream &destin, int CfsmAmount);
void Fill_state_product(ifstream &source, ofstream &destin);
bool isStringInVector(const string &str, const vector<string> &vec);
int  find_cfsm(string state);
void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm);
void calculate_transition_probabilities_V1(const vector<state_product> &stateProducts, map<pair<string, string>, double> &transitionProbs);
void create_tb(int num_clocks);
void ReplaceSymbolsInNewFile(ifstream& srcfile, ofstream& dstfile, const vector<string>& symbols, const vector<string>& replacements);
void ReplaceSymbolsInNewFile(ifstream& srcfile, ofstream& dstfile, const vector<string>& symbols, const vector<string>& replacements, const string& triggerSymbol);
void Return2Beginning(ifstream &file);
vector<double> calc_product_prob(const vector<state_product>& stateProducts);
void calc_state_prob_V1(unordered_map<string, double>& stateProbMap, const vector<string>& State_list, int states);
unordered_map<string, unordered_map<string, double>> map_state_prob(
    const vector<state_product>& stateProducts, 
    const unordered_map<string, double>& stateProbMap,
    const vector<double>& productProbabilities);
bool compare_pt1(const Result& a, const Result& b);
bool compare_pt2(const Result& a, const Result& b);
bool compare_sum_pt(const Result& a, const Result& b);
template <typename Compare>
void update_top_results(vector<Result>& results, const Result& new_result, Compare comp);
void find_best_probabilities_recursive(
    const vector<string>& state_list,
    vector<string>& current_comb,
    vector<string>& remaining_elements,
    size_t index,
    const unordered_map<string, unordered_map<string, double>>& transitionProbMap,
    vector<Result>& min_pt1_vector,
    vector<Result>& min_pt2_vector,
    vector<Result>& min_sum_pt_vector);
void calculate_basic_probability(
    const vector<string>& state_list,
    const unordered_map<string, unordered_map<string, double>>& transitionProbMap,
    vector<Result>& min_pt1_vector,
    vector<Result>& min_pt2_vector,
    vector<Result>& min_sum_pt_vector);
void find_best_probabilities(int states, const unordered_map<string, unordered_map<string, double>>& transitionProbMap,int CfsmAmount);
void SetWorkingDirectory();
void Optimiser_Min_trans_prob(
    const vector<Result>& min_pt1_vector,
    const vector<Result>& min_pt2_vector,
    const vector<Result>& min_sum_pt_vector,
    vector<vector<string>>& cfsm);
void CreateSubFolders();


map<int, string> primitive_polynomials = {
    {2,  "1,0"        },
    {3,  "2,1"        },
    {4,  "3,2"        },
    {5,  "4,3,2,1"    },
    {6,  "5,4,2,1"    },
    {7,  "6,5,4,3"    },
    {8,  "7,5,4,3"    },
    {9,  "8,7,5,4"    },
    {10, "9,8,6,5"    },
    {11, "10,9,8,6"   },
    {12, "11,10,7,5"  },
    {13, "12,11,9,8"  },
    {14, "13,12,10,8" },
    {15, "14,13,12,10"},
    {16, "15,13,12,10"},
    {17, "16,13,12,10"},
    {18, "17,16,15,12"},
    {19, "18,17,16,13"},
    {20, "19,18,15,13"},
    {21, "20,19,18,15"},
    {22, "21,18,17,16"},
    {23, "22,21,19,17"},
    {24, "23,22,20,19"},
    {25, "24,23,22,21"}
};



int main()
{
    SetWorkingDirectory();
    // Get destination and source files from user
    cout << "\nEnter the path of the source file: " << endl;
    getline(cin, ProjectFolder);
    std::cout << "\nEnter file name: " << std::endl;
    getline(cin, SourceName);

    // Open source and destin files
    CreateSubFolders();
    ifstream source(ProjectFolder + SourceName + ".kis");
    NewLocation = ProjectFolder + "\\" + destinationFolder + "\\" + SourceName;
    ofstream destin(NewLocation + "\\" + SourceName + ".vhd");
    

    if (!source)
    {
        cerr << "Error opening source file! " << "Reason: " << strerror(errno) << endl;
        return 1;
    }
    if (!destin)
    {
        cerr << "Error opening destination file! " << "Reason: " << strerror(errno) << endl;
        return 1;
    }

    int CfsmAmount = 2; // Default value
   
    KissFiles2Vhd(CfsmAmount, source, destin); // Preform the parsing process

    create_tb(CfsmAmount); // Create the testbench files


    destin.close();

    CfsmAmount = 1;
    
    NewLocation2 = ProjectFolder + "\\" + destinationFolder2 + "\\" + SourceName;
    destin.open(NewLocation2 + "\\" + SourceName + ".vhd");

    source.clear();
    source.seekg(0, ios::beg); // Return to the beginning of the file

    KissFiles2Vhd(CfsmAmount, source, destin); // Preform the parsing process

    create_tb(CfsmAmount); // Create the testbench files

    // Closing files.
    source.close();
    destin.close();

    cout << endl << endl << "Program completed successfully!" << endl;

    return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

int KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &destin) // Main Parser function - Convert Kiss to Vhd
{
    int j;

    destin << "library ieee;" << endl;
    destin << "use ieee.std_logic_1164.all;" << endl;
    destin << "use ieee.std_logic_arith.all;" << endl;
    destin << "use ieee.std_logic_unsigned.all;" << endl;
    destin << "\nentity " << SourceName << " is" << endl;
    destin << "port(" << endl;
    destin << "\t" << "rst\t\t: in\tstd_logic;" << endl;
    destin << "\t" << "clk\t\t: in\tstd_logic_vector(" << CfsmAmount - 1 << " downto 0);" << endl;

    MakeIODecleration(source, destin);

    
    destin << "\t" << "clken\t: out\tstd_logic_vector(" << CfsmAmount - 1 << " downto 0)" << endl;
    destin << ");" << endl;
    destin << "end entity " << SourceName << ";" << endl;
    destin << "\narchitecture arc_state_machine of " << SourceName << " is" << endl;

    //cout << "Now starting Fill_state_product" << endl;
    Fill_state_product(source, destin); // Create state_list and assign values to stateProducts
    //cout << "Now Finished Fill_state_product" << endl;

    
    vector<double> productProbabilities = calc_product_prob(stateProducts);
    unordered_map<string, double> stateProbMap;
    

    calc_state_prob_V1(stateProbMap, State_list, states);

    
    auto transitionProbMap = map_state_prob(stateProducts, stateProbMap, productProbabilities);
    

    // Print the transition probability map
    for (const auto& si : stateProbMap) {
        for (const auto& sj : stateProbMap) {
            cout << si.first << " -> " << sj.first << ": " << transitionProbMap[si.first][sj.first] << endl;
        }
    }



/*============================================================*/

/*                   For The Optimiser                        */

/*============================================================*/




    
    if (CfsmAmount != 1){
    string Opt;
    cout << "Available Optimisers:\n\t1 - Optimiser_Axe\n\t2 - Optimiser_Min_trans_prob\nChoose an optimizer: ";
    getline(cin, Opt);
    cout << endl;
    if (Opt == "1")
        Optimiser_Axe(source, cfsm); // Located After (Fill_state_product) && Before (FSM2Process) √√√√√√√√√
    else if (Opt == "2"){
        auto start = chrono::high_resolution_clock::now();
        find_best_probabilities(states, transitionProbMap,CfsmAmount);
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;
        // Print the duration in seconds
        cout << "Time taken: " << duration.count() << " seconds" << endl;
    }
    }
    else if (CfsmAmount == 1){

        vector<string> firstHalf;
        cfsm.clear();
    for (int i = 0; i < states; ++i) {
        firstHalf.push_back(State_list[i]);
    }  
    cfsm.push_back(firstHalf);  
    }

    if (CfsmAmount == 2){
    MakeTypeState(source, destin, cfsm);} // Create the lines for the state type and signal st
    else if (CfsmAmount == 1){
    MakeTypeState2(source, destin);    
    }


    destin << "begin" << endl << endl;
    if (CfsmAmount == 1)
    destin << "\tclken(0) <= '1';" << endl;

    for (j = 0; j < CfsmAmount; j++)
    {
        destin << "\n\t" << "cfsm" << j << ": process(clk(" << j << "), rst) begin\n"
               << endl;
        destin << "\t\t" << "if(rst = '1') then" << endl;

        switch (j)
        {
        case 0:
            destin << "\t\t\t" << "s" << j << "\t<=\tst0;" << endl;
            break;

        default:
            destin << "\t\t\t" << "s" << j << "\t<=\tst" << j << "_wait;" << endl;
            break;
        }

        destin << "\t\t" << "elsif rising_edge(clk(" << j << ")) then" << endl;
        if (CfsmAmount != 1){
        if (j >= 0 && j < cfsm.size())
        {
            for (const string &state : cfsm[1 - j])
            {
                int stateNumber = stoi(state.substr(2)); // Extract the number from the state string
                destin << "\t\tz(" << stateNumber << ") := '0';" << endl;
            }
        }}

        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

        FSM2Process(j, destin,CfsmAmount); // Create the vhdl process for the current cfsm

        destin << "\t\t" << "end if;" << endl;
        if (CfsmAmount != 1){
        if (j >= 0 && j < cfsm.size())
        {
            int otherCfsmIndex = (j == 0) ? 1 : 0; // Determine the other CFSM index
            destin << "\tclken(" << otherCfsmIndex << ") <= ";
            for (size_t i = 0; i < cfsm[otherCfsmIndex].size(); ++i)
            {
                int stateNumber = std::stoi(cfsm[otherCfsmIndex][i].substr(2)); // Extract the number from the state string
                destin << "z(" << stateNumber << ")";
                if (i < cfsm[otherCfsmIndex].size() - 1)
                {
                    destin << " or ";
                }
                else
                {
                    destin << ";" << std::endl;
                }
            }
        }}

        destin << "\t" << "end process cfsm" << j << ";\n\n";
    }
    if (CfsmAmount != 1)
    destin << "\ty <= y1 or y2;" << endl << endl;
    else
    destin << "\ty <= y1;" << endl << endl;
    
    destin << "end arc_state_machine;" << endl;
    return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void MakeIODecleration(ifstream &source, ofstream &destin)
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
            destin << "\t" << "x\t\t: in\tstd_logic_vector(" << number << " downto 0);" << endl;
        }

        // Handle ".o" case
        if (o_pos != string::npos)
        {
            size_t startIndex = o_pos + 3; // Skip ".o "
            size_t endIndex = line.find_first_not_of("0123456789", startIndex);
            string numberString = line.substr(startIndex, endIndex - startIndex);
            int number = stoi(numberString) - 1;
            output = stoi(numberString) - 1;
            destin << "\t" << "y\t\t: out\tstd_logic_vector(" << number << " downto 0);" << endl;
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
            products = stoi(numberString) - 1; // When merging, maybe need to remove the -1
        }
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void MakeTypeState(ifstream &source, ofstream &destin, const vector<vector<string>> &cfsm) {
    ostringstream type_state;
    int CfsmAmount = cfsm.size();
    int states=0;
    
    for (const auto &v : cfsm) {
        states += v.size();
    }

    // First part: type state_0
    type_state << "\ntype state_0 is (";
    for (int i = 0; i < cfsm[CfsmAmount - 2].size(); ++i) {
        type_state << cfsm[CfsmAmount - 2][i];
        if (i < cfsm[CfsmAmount - 2].size() - 1)
            type_state << ", ";
    }
    type_state << ", st0_wait);" << endl;


    // Second part: type state_1
    type_state << "type state_1 is (";
    for (int i = 0; i < cfsm[CfsmAmount - 1].size(); ++i) {
        type_state << cfsm[CfsmAmount - 1][i];
        if (i < cfsm[CfsmAmount - 1].size() - 1)
            type_state << ", ";
    }
    type_state << ", st1_wait);" << endl;

    // Adding signals
    type_state << "signal s0 : state_0;" << endl;
    type_state << "signal s1 : state_1;" << endl;

    // Adding shared variables
    for (int i = 1; i <= CfsmAmount; ++i) {
        type_state << "shared variable y" << i << ": std_logic_vector(y'range);" << endl;
    }
    type_state << "shared variable z: std_logic_vector(" << states - 1 << " downto 0) := (others => '0');\n" << endl;

    // Writing to the destination
    destin << "\t" << type_state.str();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void MakeTypeState2(ifstream &source, ofstream &dest) // Write the values of the enum type state
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
            dest << "\tsignal s0 : state;" << endl;
            dest << "\tshared variable y1: std_logic_vector(y'range);" << endl;


        }
    }

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void FSM2Process(int j, ofstream &destin,int CfsmAmount)
{
    destin << "\t\t\t" << "case s" << j << " is" << endl;
    for (int i = 0; i < cfsm[j].size(); i++) // Iterate over all the states of this specific cfsm
    {
        destin << "\t\t\t\t" << "when " << cfsm[j][i] << " =>\n\n";
        destin << "\t\t\t\t\t" << "case? x is" << endl;
        for (int k = 0; k < stateProducts.size(); k++) // Iterate over all the state_products
        {
            if (stateProducts[k].cs == cfsm[j][i]) // If the current state_product is in the current state
            {
                destin << "\t\t\t\t\t\t" << "when \"" << stateProducts[k].x << "\" =>" << endl;

                if (isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state ***is*** in the current cfsm
                destin << "\t\t\t\t\t\t\t" << "s" << j << " <= " << stateProducts[k].ns << ";" << endl;

                string modified_y = stateProducts[k].y;
                replace(modified_y.begin(), modified_y.end(), '-', OutputBitReplace);

                destin << "\t\t\t\t\t\t\t" << "y" << j+1 << " := \"" << modified_y << "\";" << endl;
                if (!isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state is not in the current cfsm
                {
                    int nextStateNumber = std::stoi(stateProducts[k].ns.substr(2));

                    destin << "\t\t\t\t\t\t\t" << "s" << j << " <= st"<<j<<"_wait;" << endl;
                    
                   

                    destin << "\t\t\t\t\t\t\t" << "z(" << nextStateNumber << ") := '1';" << endl; // z(ns) = 1

                }
            }
        }
        destin << "\t\t\t\t\t\t" << "when others => NULL;" << endl;
        destin << "\t\t\t\t\t" << "end case?;\n\n";

    }
    if (CfsmAmount != 1){
    destin << "\t\t\t\t" << "when st"<<j<<"_wait=>" << endl;
    
    if (j >= 0 && j < cfsm.size()) {
    destin << "\t\t\t\t\tif ";
    for (size_t i = 0; i < cfsm[j].size(); ++i) {
        int stateNumber = std::stoi(cfsm[j][i].substr(2)); // Extract the number from the state string
        if (i == 0) {
            destin << "(z(" << stateNumber << ")='1') then" << std::endl;
        } else {
            destin << "\t\t\t\t\telsif (z(" << stateNumber << ")='1') then" << std::endl;
        }
        destin << "\t\t\t\t\t\ts" << j << " <= " << cfsm[j][i] << ";" << std::endl;
    }
    destin << "\t\t\t\t\telse" << std::endl;
    destin << "\t\t\t\t\t\ts" << j << " <= st" << j << "_wait;" << std::endl;
    destin << "\t\t\t\t\tend if;" << std::endl;
}}

    destin << "\t\t\t\t" << "when others => NULL;\n" << endl;
    destin << "\t\t\t" << "end case;" << endl;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Fill_state_product(ifstream &source, ofstream &destin)
{
    unordered_map<string, string> stateMap; // To map states to state names
    string line;

    for (int i = 0; i < states; ++i) // Generate format list for states
        State_list.push_back("st" + to_string(i));

    Return2Beginning(source);

    // First pass to find and handle the ".r" value
    while (getline(source, line))
    {
        if (line.find(".r") != string::npos)
        {
            istringstream iss(line);
            string temp;
            iss >> temp >> Original_Reset_state_code;    // Read ".r" and its associated value
            stateMap[Original_Reset_state_code] = "st0"; // Map the reset state to "st0"
            break; // We assume there's only one ".r" line
        }
    }

    // Rewind to the beginning of the file for the second pass
    Return2Beginning(source);
    
    // Second pass to process the actual state lines
    while (getline(source, line))
    {
        // Skip lines that start with a period or are empty
        if (line.empty() || line[0] == '.')
            continue;

        // Extract fields from the line
        istringstream iss(line);
        if (!(iss >> sp.x >> sp.cs >> sp.ns >> sp.y))
        {
            cerr << "Error: Failed to parse line: " << line << endl;
            break;
        }

        // Map and replace cs if it does not match the Format list
        if (stateMap.find(sp.cs) == stateMap.end()) {
            if (!isStringInVector(sp.cs, State_list)){
            string stateName = "st" + to_string(stateMap.size());
            stateMap[sp.cs] = stateName;
            State_list.push_back(stateName);
            sp.cs = stateMap[sp.cs];
            }
            else 
                stateMap[sp.cs]=sp.cs;
        }
        else 
            sp.cs =  stateMap[sp.cs];
        
        // Map and replace ns if it does not match the Format list
         if (stateMap.find(sp.ns) == stateMap.end())
        {
            if (!isStringInVector(sp.ns, State_list)){
            string stateName = "st" + to_string(stateMap.size());
            stateMap[sp.ns] = stateName;
            sp.ns = stateMap[sp.ns];

            }
            else 
                stateMap[sp.ns]=sp.ns;
        }
        else 
            sp.ns =  stateMap[sp.ns];
        // Add the state_product instance to the vector
        stateProducts.push_back(sp);
        
        // Add contents to testarray
        static int k = 0;
        testarray.push_back(("\t" + to_string(k++) + "\t=> (x => \"" + sp.x + "\",\tCS => " + sp.cs +
                            ",\tNS => " + sp.ns + ",\ty => \"" + sp.y + "\")"));
        
        // Print the contents of the state_product instance
        cout << "x: " << sp.x << ", cs: " << sp.cs << ", ns: " << sp.ns << ",\ty: " << sp.y << endl;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

bool isStringInVector(const string &str, const vector<string> &vec) // function to check if a string is in a vector of strings
{
    return find(vec.begin(), vec.end(), str) != vec.end();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

int find_cfsm(string state) // function to find which cfsm the state belongs to
{
    for (int i = 0; i < cfsm.size(); i++)
        if (isStringInVector(state, cfsm[i]))
            return i;      
    cerr << "Error: state not found in any cfsm" << endl;
    return -1;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm)
{
    vector<string> firstHalf, secondHalf;
    string line;
    for (int i = 0; i < states; ++i)
    {
        if (i < states / 2)
            firstHalf.push_back(State_list[i]);
        else
            secondHalf.push_back(State_list[i]);
    }
    cfsm.push_back(firstHalf);
    cfsm.push_back(secondHalf);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void create_tb(int num_clocks) // Copy and use the template files to create the testbench
{
    /*  symbols: $ = source name,  @ = vcd run time, ?<char> = replace parameter with number from cpp file
        template files: vcdrun.do, tb_state_machine.vhd, tb_package_state_machine.vhd, top_template.vhd
        new files: tb_$.vhd, tb_package_$.vhd, vcdrun_$.do, top_$.vhd

        Create new files in the destination folder
        test if the files are created successfully
        Copy the content of the template files to the new files
        Replace the symbols in the new files with the appropriate values */

    // copy-paste the PLL file:
    ifstream PLLFile(templateFolder + "\\PLL_altpll.vhd");
    ofstream PLLCopy(NewLocation + "\\PLL_altpll.vhd");

    // Check if the PLL_altpll.vhd file was opened successfully
    if (!PLLFile || !PLLCopy)
    {
        cerr << "Error: Failed to open or create PLL_altpll.vhd file" << endl;
        return;
    }

    // Copy the content of the file
    PLLCopy << PLLFile.rdbuf();

    // Open the template files for read
    ifstream VcdDoTemplate   (templateFolder + "\\vcdrun.do");
    ifstream TbTemplate      (templateFolder + "\\tb_state_machine.vhd");
    ifstream PackTemplate    (templateFolder + "\\tb_package_state_machine.vhd");
    ifstream TopTemplate     (templateFolder + "\\top_template.vhd");
    ifstream TopPackTemplate (templateFolder + "\\top_pack_template.vhd");

    // Check if the template files were opened successfully
    if (!VcdDoTemplate || !TbTemplate || !PackTemplate || !TopTemplate || !TopPackTemplate)
    {
        cerr << "Error: Failed to open template files" << endl;
        return;
    }

    // Create new files in the destination folder
    ofstream TbVhd       (NewLocation + "\\tb_" +         SourceName + ".vhd");
    ofstream TbPackageVhd(NewLocation + "\\tb_package_" + SourceName + ".vhd");
    ofstream VcdDoTb     (NewLocation + "\\vcdrun_" +     SourceName + ".do");
    ofstream top_vhd     (NewLocation + "\\top_" +        SourceName + ".vhd");
    ofstream top_pack    (NewLocation + "\\top_pack_" +   SourceName + ".vhd");

    // Check if the new files were created successfully
    if (!TbVhd || !TbPackageVhd || !VcdDoTb || !top_vhd || !top_pack)
    {
        cerr << "Error: Failed to create new test-bench files. Reason: " << strerror(errno) << endl;
        return;
    }

    // Recieve user input for the vcdrun.do simulation run time and clock period
    cout << "Enter the vcd run time (with time units): ";
    string vcdRunTime;
    getline(cin, vcdRunTime);
    cout << "Enter the clock period (with time units): ";
    string clockPeriod;
    getline(cin, clockPeriod);

    // Replace the symbols in the new files with the appropriate values
    ReplaceSymbolsInNewFile(VcdDoTemplate, VcdDoTb, {"$", "@"}, {SourceName, vcdRunTime});
    
    ReplaceSymbolsInNewFile(TbTemplate, TbVhd, {"$"}, {SourceName});
    
    ReplaceSymbolsInNewFile(PackTemplate, TbPackageVhd, {"$", "?x", "?y", "?c", "?t", "?s", "?p", "?q"},
                            {SourceName, to_string(input), to_string(output), to_string(num_clocks), clockPeriod, type_state.str(), to_string(testarray.size()-1), "\0"},
                            "?q");
    
    ReplaceSymbolsInNewFile(TopTemplate, top_vhd, {"$", "?x", "?y", "?c"}, {SourceName, to_string(input), to_string(output), to_string(num_clocks - 1)});

    ReplaceSymbolsInNewFile(TopPackTemplate, top_pack, {"$", "?g"}, {SourceName, primitive_polynomials[input+1]});
        
    // Close the files
    VcdDoTemplate.close();
    TbTemplate.close();
    PackTemplate.close();
    TopTemplate.close();
    TopPackTemplate.close();
    PLLFile.close();
    TbVhd.close();
    TbPackageVhd.close();
    VcdDoTb.close();
    top_vhd.close();
    top_pack.close();
    PLLCopy.close();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* Overload number 1*/
void ReplaceSymbolsInNewFile(ifstream& srcfile, ofstream& dstfile, const vector<string>& symbols, const vector<string>& replacements)
{ // Replaces "symbols" with "replacements" in the file
    string line;
    size_t pos;
    while (getline(srcfile, line))
    {
        for (size_t i = 0; i < symbols.size(); i++)
        {
            pos = line.find(symbols[i]);
            if (pos != string::npos)
                line.replace(pos, symbols[i].length(), replacements[i]);
        }
        dstfile << line << endl;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* Overload number 2*/
void ReplaceSymbolsInNewFile(ifstream& srcfile, ofstream& dstfile, const vector<string>& symbols, const vector<string>& replacements, const string& triggerSymbol)
{ 
    string line;
    size_t pos;
    while (getline(srcfile, line))
    {
        for (size_t i = 0; i < symbols.size(); i++)
        {
            pos = line.find(symbols[i]);
            if (pos != string::npos)
            {
                if (symbols[i] == triggerSymbol)
                    {
                        for(int j = 0; j < testarray.size(); j++)
                        {
                            dstfile << testarray[j];
                            if(j < testarray.size()-1)
                                dstfile << "," << endl;
                        }
                        line = "";
                    }
                else
                    line.replace(pos, symbols[i].length(), replacements[i]);
            }
        }
        dstfile << line << endl;
    }
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Return2Beginning(ifstream &file) // Return to the beginning of the file
{
    file.clear();
    file.seekg(0, ios::beg);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/


vector<double> calc_product_prob(const vector<state_product>& stateProducts) {
    vector<double> probabilities;

    for (const auto& product : stateProducts) {
        int totalBits = product.x.size();
        int dontCareBits = count(product.x.begin(), product.x.end(), '-');
        int careBits = totalBits - dontCareBits;
        double probability = pow(2, static_cast<double>(-careBits));
        probabilities.push_back(probability);
    }

    return probabilities;
}


void calc_state_prob_V1(unordered_map<string, double>& stateProbMap, const vector<string>& State_list, int states) {
    double probability = 1.0 / states;

    for (const string& state : State_list) {
        stateProbMap[state] = probability;
    }
}

unordered_map<string, unordered_map<string, double>> map_state_prob(
    const vector<state_product>& stateProducts, 
    const unordered_map<string, double>& stateProbMap,
    const vector<double>& productProbabilities) {
    
    unordered_map<string, unordered_map<string, double>> transitionProbMap;
    unordered_map<string, double> stateProb;

    for (const auto& sp : stateProducts) {
        stateProb[sp.cs] = stateProbMap.at(sp.cs);
    }

    for (const auto& si : stateProbMap) {
        for (const auto& sj : stateProbMap) {
            double sumProductProb = 0.0;

            for (size_t k = 0; k < stateProducts.size(); ++k) {
                if (stateProducts[k].cs == si.first && stateProducts[k].ns == sj.first) {
                    sumProductProb += productProbabilities[k];
                }
            }

            transitionProbMap[si.first][sj.first] = stateProb.at(si.first) * sumProductProb;
        }
    }

    return transitionProbMap;
}


bool compare_pt1(const Result& a, const Result& b) {
    return a.pt1 < b.pt1;
}

bool compare_pt2(const Result& a, const Result& b) {
    return a.pt2 < b.pt2;
}

bool compare_sum_pt(const Result& a, const Result& b) {
    return a.sum_pt < b.sum_pt;
}

// Function to update the top 5 results
template <typename Compare>
void update_top_results(vector<Result>& results, const Result& new_result, Compare comp) {
    if (results.size() < 5) {
        results.push_back(new_result);
    } else {
        if (comp(new_result, results.back())) {
            results.back() = new_result;
        }
    }
    sort(results.begin(), results.end(), comp);
}

// Recursive function to generate combinations and find best probabilities
void find_best_probabilities_recursive(
    const vector<string>& state_list,
    vector<string>& current_comb,
    vector<string>& remaining_elements,
    size_t index,
    const unordered_map<string, unordered_map<string, double>>& transitionProbMap,
    vector<Result>& min_pt1_vector,
    vector<Result>& min_pt2_vector,
    vector<Result>& min_sum_pt_vector) {

    if (current_comb.size() == state_list.size() / 2) {
        remaining_elements.clear();
        for (const auto& state : state_list) {
            if (find(current_comb.begin(), current_comb.end(), state) == current_comb.end()) {
                remaining_elements.push_back(state);
            }
        }

        double pt1 = 0.0;
        double pt2 = 0.0;

        for (const auto& cs : current_comb) {
            for (const auto& ns : remaining_elements) {
                pt1 += transitionProbMap.at(cs).at(ns);
                pt2 += transitionProbMap.at(ns).at(cs);
            }
        }

        double sum_pt = pt1 + pt2;

        Result result = {current_comb, remaining_elements, pt1, pt2, sum_pt};

        update_top_results(min_pt1_vector, result, compare_pt1);
        update_top_results(min_pt2_vector, result, compare_pt2);
        update_top_results(min_sum_pt_vector, result, compare_sum_pt);

        return;
    }

    if (index >= state_list.size()) {
        return;
    }

    // Include current index in the combination
    current_comb.push_back(state_list[index]);
    find_best_probabilities_recursive(state_list, current_comb, remaining_elements, index + 1, transitionProbMap, min_pt1_vector, min_pt2_vector, min_sum_pt_vector);
    current_comb.pop_back();

    // Exclude current index from the combination
    find_best_probabilities_recursive(state_list, current_comb, remaining_elements, index + 1, transitionProbMap, min_pt1_vector, min_pt2_vector, min_sum_pt_vector);
}

void calculate_basic_probability(
    const vector<string>& state_list,
    const unordered_map<string, unordered_map<string, double>>& transitionProbMap,
    vector<Result>& min_pt1_vector,
    vector<Result>& min_pt2_vector,
    vector<Result>& min_sum_pt_vector) {

    vector<string> cfsm0(state_list.begin(), state_list.begin() + state_list.size() / 2);
    vector<string> cfsm1(state_list.begin() + state_list.size() / 2, state_list.end());

    double pt1 = 0.0;
    double pt2 = 0.0;

    for (const auto& cs : cfsm0) {
        for (const auto& ns : cfsm1) {
            pt1 += transitionProbMap.at(cs).at(ns);
            pt2 += transitionProbMap.at(ns).at(cs);
        }
    }

    double sum_pt = pt1 + pt2;

    Result basic_result = {cfsm0, cfsm1, pt1, pt2, sum_pt};

  // Print the basic probability
    cout << "Basic probability:" << endl;
    cout << "{cfsm0={";
    for (const auto& s : basic_result.cfsm0) cout << s << ",";
    cout << "}, cfsm1={";
    for (const auto& s : basic_result.cfsm1) cout << s << ",";
    cout << "}, pt1=" << basic_result.pt1 << ", pt2=" << basic_result.pt2 << ", sum_pt=" << basic_result.sum_pt << "}" << endl;

    update_top_results(min_pt1_vector, basic_result, compare_pt1);
    update_top_results(min_pt2_vector, basic_result, compare_pt2);
    update_top_results(min_sum_pt_vector, basic_result, compare_sum_pt);
}

// Function to find the best probabilities using the recursive helper
void find_best_probabilities(int states, const unordered_map<string, unordered_map<string, double>>& transitionProbMap,int CfsmAmount) {
    vector<string> state_list;
    for (int i = 0; i < states; ++i) {
        state_list.push_back("st" + to_string(i));
    }
    

    vector<string> current_comb;
    vector<string> remaining_elements;

    vector<Result> min_pt1_vector;
    vector<Result> min_pt2_vector;
    vector<Result> min_sum_pt_vector;

    calculate_basic_probability(state_list, transitionProbMap, min_pt1_vector, min_pt2_vector, min_sum_pt_vector);

    find_best_probabilities_recursive(state_list, current_comb, remaining_elements, 0, transitionProbMap, min_pt1_vector, min_pt2_vector, min_sum_pt_vector);

    // Print the best results
    cout << "Best results for min_pt1:" << endl;
    for (const auto& res : min_pt1_vector) {
        cout << "{cfsm0={";
        for (const auto& s : res.cfsm0) cout << s << ",";
        cout << "}, cfsm1={";
        for (const auto& s : res.cfsm1) cout << s << ",";
        cout << "}, pt1=" << res.pt1 << ", pt2=" << res.pt2 << ", sum_pt=" << res.sum_pt << "}" << endl;
    }

    cout << "Best results for min_pt2:" << endl;
    for (const auto& res : min_pt2_vector) {
        cout << "{cfsm0={";
        for (const auto& s : res.cfsm0) cout << s << ",";
        cout << "}, cfsm1={";
        for (const auto& s : res.cfsm1) cout << s << ",";
        cout << "}, pt1=" << res.pt1 << ", pt2=" << res.pt2 << ", sum_pt=" << res.sum_pt << "}" << endl;
    }

    cout << "Best results for min_sum_pt:" << endl;
    for (const auto& res : min_sum_pt_vector) {
        cout << "{cfsm0={";
        for (const auto& s : res.cfsm0) cout << s << ",";
        cout << "}, cfsm1={";
        for (const auto& s : res.cfsm1) cout << s << ",";
        cout << "}, pt1=" << res.pt1 << ", pt2=" << res.pt2 << ", sum_pt=" << res.sum_pt << "}" << endl;
    }
    
    Optimiser_Min_trans_prob(min_pt1_vector, min_pt2_vector, min_sum_pt_vector, cfsm);
    

}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void SetWorkingDirectory() // Set the working directory to the exe location
{
    using namespace std;
    // Get the full path of the executable
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of("\\/");
    string exePath = string(buffer).substr(0, pos);
    // Set the working directory to the executable location
    filesystem::current_path(exePath);
}


void Optimiser_Min_trans_prob(
    const vector<Result>& min_pt1_vector,
    const vector<Result>& min_pt2_vector,
    const vector<Result>& min_sum_pt_vector,
    vector<vector<string>>& cfsm) {

    const vector<Result>* vectors[] = {&min_pt1_vector, &min_pt2_vector, &min_sum_pt_vector};
    double min_score = numeric_limits<double>::max();
    const Result* best_result = nullptr;

    for (const auto& vec : vectors) {
        for (const auto& res : *vec) {
            double score = res.pt1 * 10 + res.pt2 * 10 + res.sum_pt;
            if (score < min_score) {
                min_score = score;
                best_result = &res;
            }
        }
    }

    if (best_result) {
        cfsm.push_back(best_result->cfsm0);
        cfsm.push_back(best_result->cfsm1);
    }

     // Print the chosen CFSM
    cout << "Chosen CFSM:" << endl;
    for (const auto& group : cfsm) {
        cout << "{ ";
        for (const auto& state : group) {
            cout << state << " ";
        }
        cout << "}" << endl;
    }
}

void CreateSubFolders() // Create the necessary subfolders
{
    // Destination folder
    if (!filesystem::exists(destinationFolder))
        filesystem::create_directory(destinationFolder);
    // Subfolder for the specific source
    if (!filesystem::exists(destinationFolder + "\\" + SourceName))
        filesystem::create_directory(destinationFolder + "\\" + SourceName);

    if (!filesystem::exists(destinationFolder2))
        filesystem::create_directory(destinationFolder2);
    // Subfolder for the specific source
    if (!filesystem::exists(destinationFolder2 + "\\" + SourceName))
        filesystem::create_directory(destinationFolder2 + "\\" + SourceName);



}