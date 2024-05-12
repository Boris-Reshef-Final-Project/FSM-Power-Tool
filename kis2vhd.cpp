#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &dest);
void MakeIODecleration(ifstream &source, ofstream &dest);
void MakeTypeState(ifstream &source, ofstream &dest);
void FSM2Process(int CfsmAmount,ifstream &source, ofstream &dest);
void Fill_state_product(ifstream &source, ofstream &dest);

struct state_product {
    string x;
    string cs;
    string ns;
    string y;
};

vector<state_product> stateProducts;

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

        FSM2Process(CfsmAmount,source, dest);

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



void FSM2Process(int CfsmAmount,ifstream &source, ofstream &dest)
{

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