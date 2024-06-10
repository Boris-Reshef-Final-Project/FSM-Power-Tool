#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <unordered_map>
#include <cmath>
#define OutputBitReplace '0'

using namespace std;

string Original_Reset_value;         // Original_Reset_value          -> (Fill_state_product)
int input, output, NP, states;       // NP is (מספר שורות) ;         -> (MakeIODecleration)
                                     // input,output                  -> (MakeIODecleration)
                                     // states                        -> (MakeTypeState)
stringstream type_state;
vector<string> State_list;

// Structure to store results
struct Result {
    vector<string> cfsm0;
    vector<string> cfsm1;
    double pt1;
    double pt2;
    double sum_pt;
};

int KissFiles2Vhd(int CfsmAmount, ifstream &source, ofstream &dest);
void MakeIODecleration(ifstream &source, ofstream &dest);
void MakeTypeState(ifstream &source, ofstream &dest);
void FSM2Process(int j, ofstream &dest);
void Fill_state_product(ifstream &source, ofstream &dest);
bool isStringInVector(const string &str, const vector<string> &vec);
int find_cfsm(string state);
void Optimiser_Axe(ifstream &source, vector<vector<string>> &cfsm);
void Optimiser_Min_trans_prob(
    const vector<Result>& min_pt1_vector,
    const vector<Result>& min_pt2_vector,
    const vector<Result>& min_sum_pt_vector,
    vector<vector<string>>& cfsm);
void calc_state_prob_V1(unordered_map<string, double>& stateProbMap, const vector<string>& State_list, int states);
void find_best_probabilities(int states, const unordered_map<string, unordered_map<string, double>>& transitionProbMap);



struct state_product
{
    string x;
    string cs;
    string ns;
    string y;
};

vector<state_product> stateProducts;

unordered_map<string, unordered_map<string, double>> map_state_prob(
    const vector<state_product>& stateProducts, 
    const unordered_map<string, double>& stateProbMap,
    const vector<double>& productProbabilities);

vector<double> calc_product_prob(const vector<state_product>& stateProducts);

vector<vector<string>> cfsm; // Vector to store the states of each cfsm

vector<vector<vector<string>>> Allcfsms_AndProbabilities; // Each element will be a vector of 2 vectors of strings and 2 doubles



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




    MakeTypeState(source, dest); // type state is (st0, st1, st2,..., st12);
                                 // signal st : state;
    dest << "begin" << endl
         << endl;

    int Opt;
    cout << "Choose Optimiser: 1 - Optimiser_Axe, 2 - Optimiser_Min_trans_prob" << endl;
    cin >> Opt;
    if (Opt == 1)
        Optimiser_Axe(source, cfsm); // Located After (Fill_state_product) && Before (FSM2Process) √√√√√√√√√
    else if (Opt ==2)
    find_best_probabilities(states, transitionProbMap);

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

                NP = stoi(numberString) ;
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
void find_best_probabilities(int states, const unordered_map<string, unordered_map<string, double>>& transitionProbMap) {
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


void calculate_transition_probabilities_V1(const vector<state_product> &stateProducts, map<pair<string, string>, double> &transitionProbs)
{
    
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


