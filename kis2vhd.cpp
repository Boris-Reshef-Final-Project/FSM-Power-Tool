/**-------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------
Project:        FSM-Power-Tool
Authors:        Boris Karasov, Reshef Schachter
Date:           2023-2024
Institution:    Afeka College of Engineering
Description:    This is the main file for our bachelor degree in Electrical Engineering final project.
Notes:          This is meant to work with a VHDL2008 compiler ONLY!
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
#include <filesystem>
#include <cerrno>
#include <cstring>

#define OutputBitReplace '0'

using namespace std;

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*===============================*/
/*      Global Declarations      */
/*===============================*/

struct state_product {string x; string cs; string ns; string y;};
vector<state_product> stateProducts;
vector<vector<string>> cfsm; // Vector to store the states of each cfsm
state_product sp; // Create a Global state_product instance
string ProjectFolder;
string SourceName;
string NewLocation;
string templateFolder    = "tb-template";   // Name of template folder
string destinationFolder = "optimised";     // Name of destination folder
stringstream type_state;                    // String stream to store the type state line
vector<string> State_list;                  // Vector to store the state names
string Original_Reset_state_code;           // The name (code) of the original reset state in the Kiss file
int input, output, products, states;        // Number of inputs, outputs, products, and states as declared in the Kiss file preamble

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*===============================*/
/*      Function Prototypes      */
/*===============================*/

int  KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &destin);
void MakeIODecleration(ifstream &source, ofstream &destin);
void MakeTypeState(ifstream &source, ofstream &destin);
void FSM2Process(int j, ofstream &destin);
void Fill_state_product(ifstream &source, ofstream &destin);
bool isStringInVector(const string &str, const vector<string> &vec);
int  find_cfsm(string state);
void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm);
void Optimiser_Min_trans_prob(vector<vector<string>> &cfsm);
void create_tb(int num_clocks);
void ReplaceSymbolsInNewFile(ifstream& srcfile, ofstream& dstfile, const vector<string>& symbols, const vector<string>& replacements);
void Return2Beginning(fstream &file);
void CreateSubFolders();

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*=====================*/
/*      Functions      */
/*=====================*/

int main()
{
    // Get destination and source files from user
    cout << "\nEnter the path of the source file: " << endl;
    getline(cin, ProjectFolder);
    cout << "\nEnter file name: " << endl;
    getline(cin, SourceName);

    // Open source and destin files
    CreateSubFolders();
    ifstream source(ProjectFolder + SourceName + ".kis");
    NewLocation = ProjectFolder + "/" + destinationFolder + "/" + SourceName;
    ofstream destin(NewLocation + "/" + SourceName + ".vhd");

    if (!source)
    {
        cerr << "Error opening source file!" << endl;
        return 1;
    }
    if (!destin)
    {
        cerr << "Error opening destination file!" << endl;
        return 1;
    }

    int CfsmAmount = 2; // Default value
    // cout << "insert amount of cfsm's :" << endl; // These lines are for if we decide to allow more than 2 cfsm's
    // cin >> CfsmAmount;

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
    destin << "\nentity state_machine is" << endl;
    destin << "port(" << endl;
    destin << "\t" << "rst\t\t: in\tstd_logic;" << endl;
    destin << "\t" << "clk\t\t: in\tstd_logic_vector(" << CfsmAmount - 1 << " downto 0);" << endl;

    MakeIODecleration(source, destin);

    
    destin << "\t" << "z\t\t: out\tstd_logic_vector(" << CfsmAmount - 1 << " downto 0)" << endl;
    destin << ");" << endl;
    destin << "end entity state_machine;" << endl;
    destin << "\narchitecture arc_state_machine of state_machine is" << endl;

    //cout << "Now starting Fill_state_product" << endl;
    Fill_state_product(source, destin); // Create state_list and assign values to stateProducts
    //cout << "Now Finished Fill_state_product" << endl;

    MakeTypeState(source, destin); // Create the lines for the state type and signal st
    destin << "begin" << endl << endl;

    string Opt;
    cout << "Available Optimisers:\n\t1 - Optimiser_Axe\n\t2 - Optimiser_Min_trans_prob\nChoose an optimizer: ";
    getline(cin, Opt);
    cout << endl;
    if (Opt == "1")
        Optimiser_Axe(source, cfsm); // Located After (Fill_state_product) && Before (FSM2Process) √√√√√√√√√
    else
        Optimiser_Min_trans_prob(cfsm);

    for (j = 0; j < CfsmAmount; j++)
    {
        destin << "\n\t" << "cfsm" << j << ": process(clk(" << j << "), rst) begin\n" << endl;
        destin << "\t\t" << "if(rst = '1') then" << endl;
        destin << "\t\t\t" << "st\t<=\tst0;" << endl;
        destin << "\t\t\t" << "z <= (" << find_cfsm("st0") << " => '1', others => '0');" << endl;
        destin << "\t\t" << "elsif falling_edge(clk(" << j << ")) then" << endl;

        source.clear();
        source.seekg(0, ios::beg); // Return to the beginning of the file

        FSM2Process(j, destin); // Create the vhdl process for the current cfsm

        destin << "\t\t" << "end if;" << endl;
        destin << "\t" << "end process cfsm" << j << ";\n\n";
    }

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
            products = stoi(numberString) - 1;
        }
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void MakeTypeState(ifstream &source, ofstream &destin) // Write the values of the enum type state
{
        type_state << "type state is (";
        for (int i = 0; i < states; ++i)
        {
            type_state << State_list[i];
            if (i < states - 1)
                type_state << ", ";
        }
        type_state << ");" << endl;
        destin << "\t" << type_state.str();
        destin << "\tsignal st : state;" << endl;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void FSM2Process(int j, ofstream &destin)
{
    destin << "\t\t\t" << "case st is" << endl;
    for (int i = 0; i < cfsm[j].size(); i++) // Iterate over all the states of this specific cfsm
    {
        destin << "\t\t\t\t" << "when " << cfsm[j][i] << " =>\n\n";
        destin << "\t\t\t\t\t" << "case? x is" << endl;
        for (int k = 0; k < stateProducts.size(); k++) // Iterate over all the state_products
        {
            if (stateProducts[k].cs == cfsm[j][i]) // If the current state_product is in the current state
            {
                destin << "\t\t\t\t\t\t" << "when \"" << stateProducts[k].x << "\" =>" << endl;
                destin << "\t\t\t\t\t\t\t" << "st <= " << stateProducts[k].ns << ";" << endl;

                string modified_y = stateProducts[k].y;
                replace(modified_y.begin(), modified_y.end(), '-', OutputBitReplace);

                destin << "\t\t\t\t\t\t\t" << "y <= \"" << modified_y << "\";" << endl;
                if (!isStringInVector(stateProducts[k].ns, cfsm[j])) // If the next state is not in the current cfsm
                {
                    destin << "\t\t\t\t\t\t\t" << "z(" << j << ") <= '0';" << endl; // Disable current cfsm clock
                    int z = find_cfsm(stateProducts[k].ns);
                    destin << "\t\t\t\t\t\t\t" << "z(" << z << ") <= '1';" << endl; // Enable next cfsm clock
                }
            }
        }
        destin << "\t\t\t\t\t\t" << "when others => NULL;" << endl;
        destin << "\t\t\t\t\t" << "end case?;\n\n";
    }
    destin << "\t\t\t\t" << "when others => NULL;" << endl;
    destin << "\t\t\t" << "end case;" << endl;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Fill_state_product(ifstream &source, ofstream &destin)
{
    unordered_map<string, string> stateMap; // To map states to state names
    string line;
    vector<string> Test_Format_list; // List of states in the format "st0", "st1", etc.

    for (int i = 0; i < states; ++i) // Generate format list for states
        Test_Format_list.push_back("st" + to_string(i));

    source.clear();
    source.seekg(0, ios::beg);

    // First pass to find and handle the ".r" value
    while (getline(source, line))
    {
        if (line.find(".r") != string::npos)
        {
            istringstream iss(line);
            string temp;
            iss >> temp >> Original_Reset_state_code;    // Read ".r" and its associated value
            stateMap[Original_Reset_state_code] = "st0"; // Map the reset state to "st0"
            State_list.push_back("st0");
            break; // We assume there's only one ".r" line
        }
    }

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
        istringstream iss(line);
        if (!(iss >> sp.x >> sp.cs >> sp.ns >> sp.y))
        {
            cerr << "Error: Failed to parse line: " << line << endl;
            break;
        }

        // Map and replace cs if it does not match the Format list
        if (stateMap.find(sp.cs) == stateMap.end()) {
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
        
        // Map and replace ns if it does not match the Format list
         if (stateMap.find(sp.ns) == stateMap.end())
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

void calculate_transition_probabilities_V1(const vector<state_product> &stateProducts, map<pair<string, string>, double> &transitionProbs)
{
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Optimiser_Min_trans_prob(vector<vector<string>> &cfsm)
{
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void create_tb(int num_clocks/*args*/) // Copy and use the template files to create the testbench
{
    /*  symbols: $ = source name,  @ = vcd run time, ?<char> = replace parameter with number from cpp file
        template files: vcdrun.do, tb_state_machine.vhd, tb_package_state_machine.vhd
        new files: tb_$.vhd, tb_package_$.vhd, vcdrun_$.do

        Create new files in the destination folder
        test if the files are created successfully
        Copy the content of the template files to the new files
        Replace the symbols in the new files with the appropriate values */

    // Open the template files for read
    ifstream VcdDoTemplate  (templateFolder + "/vcdrun.do");
    ifstream TbTemplate     (templateFolder + "/tb_state_machine.vhd");
    ifstream PackTemplate   (templateFolder + "/tb_package_state_machine.vhd");

    // Check if the template files were opened successfully
    if (!VcdDoTemplate || !TbTemplate || !PackTemplate)
    {
        cerr << "Error: Failed to open template files" << endl;
        return;
    }

    // Create new files in the destination folder
    ofstream TbVhd       (NewLocation + "/tb_" +         SourceName + ".vhd");
    ofstream TbPackageVhd(NewLocation + "/tb_package_" + SourceName + ".vhd");
    ofstream VcdDoTb     (NewLocation + "/vcdrun_" +     SourceName + ".do");

    // Check if the new files were created successfully
    if (!TbVhd || !TbPackageVhd || !VcdDoTb)
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
    ReplaceSymbolsInNewFile(VcdDoTemplate,  VcdDoTb,    {"$", "@"}, {SourceName, vcdRunTime});
    
    ReplaceSymbolsInNewFile(TbTemplate,     TbVhd,      {"$"}, {SourceName});
    
    ReplaceSymbolsInNewFile(PackTemplate, TbPackageVhd, {"$", "?x", "?y", "?c", "?p", "?s", "?k"},
                            {SourceName, to_string(input), to_string(output), to_string(num_clocks), clockPeriod,
                            type_state.str(), "\"" + filesystem::absolute(ProjectFolder).string() + SourceName + ".kis" + "\""});
        
    // Close the files
    VcdDoTemplate.close();
    TbTemplate.close();
    PackTemplate.close();
    TbVhd.close();
    TbPackageVhd.close();
    VcdDoTb.close();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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

void Return2Beginning(fstream &file) // Return to the beginning of the file
{
    file.clear();
    file.seekg(0, ios::beg);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void CreateSubFolders() // Create the necessary subfolders
{
    // Destination folder
    if (!filesystem::exists(destinationFolder))
        filesystem::create_directory(destinationFolder);
    // Subfolder for the specific source
    if (!filesystem::exists(destinationFolder + "/" + SourceName))
        filesystem::create_directory(destinationFolder + "/" + SourceName);
}