#include <iostream>
#include <numeric>
#include <chrono>
#include <cstring>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <set>
#include <map>
#include <bitset>
#include <random>
#include <algorithm>
#include <functional>
#include <sstream>
#include <iomanip>

typedef unsigned char byte;

#include "gen_mkw.h"
#include "dis_query_process.h"
#include "dis_query_generation.h"


/////////////////////////////////////////////////////////
// Node structure for the binary tree
struct Node {
    int value = -1;
    bool is_leaf = false;
    bool is_present = false;
    string keyword;
    int start = -1;
    int end = -1;
    Node* left = NULL;
    Node* right = NULL;
    Node* parent = NULL; 
};

Node* create_tree(int l, int r, Node* parent = NULL) {
    if (l > r) return NULL;

    Node* root = new Node();
    root->start = l;
    root->end = r;
    root->parent = parent; // Assign the parent

    if (l == r) {
        root->is_leaf = true;
        root->value = l;
        root->keyword = "kw_" + to_string(l);
        return root;
    }
    
    string temp = "kw_" + to_string(l) + "_" + to_string(r);
    root->keyword = temp;
    int mid = (l+r)/2;

    root->left = create_tree(l, mid, root); // Pass current node as parent
    root->right = create_tree(mid+1, r, root); // Pass current node as parent

    return root;
}


void find_best_range_cover_helper(Node* node, int start, int end, vector<Node*>& cover) {
    if (!node || start > end || node->start > end || node->end < start) return;
    if (node->start >= start && node->end <= end) {
        cover.push_back(node);
        return;
    }
    find_best_range_cover_helper(node->left, start, end, cover);
    find_best_range_cover_helper(node->right, start, end, cover);
}

vector<Node*> find_best_range_cover(Node* root, int start, int end) {
    vector<Node*> cover;
    find_best_range_cover_helper(root, start, end, cover);
    return cover;
}

void print_tree(Node* root){
    queue<Node*> q;
    q.push(root);
    while(!q.empty()){
        int sz = q.size();
        for(int i = 0; i < sz; i++){
            Node* t = q.front();
            q.pop();
            cout << "("<<t->value << " " << t->keyword <<" "<<t->start<<" "<<t->end<< " "<<t->is_present<<"), ";
            if(t->left) q.push(t->left);
            if(t->right) q.push(t->right);
        }
        cout << endl;
    }
}

vector<string> perform_range_search(vector<Node*> bestRangeCover){
    vector<string> ans;
    for(auto node: bestRangeCover){
        queue<Node*> q;
        q.push(node);
        while(!q.empty()){
            Node* t = q.front();
            q.pop();
            if(t->is_leaf){
                if(t->is_present)
                    ans.push_back(t->keyword);
                continue;
            }
            q.push(t->left);
            q.push(t->right);
        }
    }
    return ans;
}

void update_is_present_flag(Node* node, const unordered_set<unsigned long>& ids) {
    if (!node) return;
    if (node->is_leaf) {
        if (ids.find(node->value) != ids.end()) {
            node->is_present = true;
        }
        return;
    }
    update_is_present_flag(node->left, ids);
    update_is_present_flag(node->right, ids);
}

int read_data(const string& filename, Node*& root, unsigned long& minId, unsigned long& maxId, map<int, string>& db6kMap) {
    ifstream file(filename); // Use the filename passed as a parameter
    string line;
    unordered_set<unsigned long> ids;

    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        return -1;
    }

    while (getline(file, line)) {
        istringstream iss(line);
        string keyword, idStr;
        unsigned long id;
        if (!getline(iss, idStr, ',') || !getline(iss, keyword, ',')) {
            cerr << "Error parsing line: " << line << endl;
            continue; // Skip to the next line
        }

        id = stoul(idStr, nullptr, 16);
        db6kMap[id] = keyword;
        ids.insert(id); // Store the ID in the set

        if (id < minId) minId = id;
        if (id > maxId) maxId = id;
    }

    file.close();
    cout<<endl;
    cout<<minId<<"---"<<maxId<<endl;
    root = create_tree(minId, maxId);

    // After the tree is created, update the is_present flag
    update_is_present_flag(root, ids);

    return 0;
}

void collect_and_write_ancestors(Node* node, ofstream& file,map<int, string>&db6kMap) {
    if (node == NULL) return;

    // Check if it's a leaf node
    if (node->is_leaf && node->is_present) {
        vector<string> keywords;
        Node* current = node;
        // Collect keywords from this node to root
        while (current != NULL) {
            keywords.push_back(current->keyword);
            current = current->parent;
        }

        if (db6kMap.find(node->value) != db6kMap.end()) {
            // Prepend the corresponding entry from db6k.dat
            file << db6kMap[node->value] << ",";
        }

        // Write to CSV: reverse the keywords to start from root to leaf
        file << node->value; // Leaf node value
        for (auto it = keywords.begin(); it != keywords.end(); ++it) { // Note the use of rbegin and rend for reverse iteration
            file << "," << *it;
        }
        file << "\n";
    }

    // Recurse for both subtrees
    collect_and_write_ancestors(node->left, file,db6kMap);
    collect_and_write_ancestors(node->right, file,db6kMap);
}


void outputNodeDetailsToTxt(Node* node, const string& filename,map<int, string>& db6kMap) {
    ofstream file(filename);
    collect_and_write_ancestors(node, file,db6kMap);
    file.close();
}
void print_map(map<int, string>&db6kMap){
    for(auto i:db6kMap){
        cout<<i.first<<" "<<i.second<<endl;
    }
}

void splitNodeDetailsToTxt(const string& inputFilePath, const string& outputFilePath) {
    // Open the input file
    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        cerr << "Failed to open " << inputFilePath << endl;
        return;
    }

    // Prepare the output file
    ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        cerr << "Failed to create " << outputFilePath << endl;
        inputFile.close();
        return;
    }

    string line;
    // Read each line from the input file
    while (getline(inputFile, line)) {
        istringstream lineStream(line);
        string token;
        vector<string> tokens;

        // Split the line by commas
        while (getline(lineStream, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 3) continue; // Skip invalid lines

        // Write each token except the first two as a new line in the output file with the first two tokens
        for (size_t i = 2; i < tokens.size(); ++i) {
            outputFile << tokens[0] << "," << tokens[1] << "," << tokens[i] << endl;
        }
    }

    // Cleanup
    inputFile.close();
    outputFile.close();
}

void finalDatabaseGeneration(const string &inputFilePath, const string &outputFilePath) {
    ifstream inputFile(inputFilePath);
    ofstream outputFile(outputFilePath);
    string line;
    map<string, vector<string>> kwMap;

    while (getline(inputFile, line)) {
        istringstream lineStream(line);
        string column1, column2, kw;
        getline(lineStream, column1, ',');
        getline(lineStream, column2, ',');
        getline(lineStream, kw);

        kwMap[kw].push_back(column1);
    }

    // Copy map to a vector of pairs
    vector<pair<string, vector<string>>> kvVector(kwMap.begin(), kwMap.end());

    // Sort the vector by the size of the vector in each pair
    sort(kvVector.begin(), kvVector.end(), [](const auto &a, const auto &b) {
        return a.second.size() < b.second.size();
    });

    // Write the sorted details to the outputFile
    for (const auto &pair : kvVector) {
        outputFile << pair.first; // kw_x
        for (const auto &col1Value : pair.second) {
            outputFile << "," << col1Value; // column1 values
        }
        outputFile << endl;
    }

    inputFile.close();
    outputFile.close();
}

int countEntries(const string& row) {
    stringstream ss(row);
    string token;
    int count = 0;
    while (getline(ss, token, ',')) {
        count++;
    }
    return count;
}

void findBRC(Node* root, const std::string& inputFileName,const std::string& outputFileName){
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName);

    if (!inputFile.is_open()) {
        cerr << "Failed to open " << inputFileName << endl;
        return;
    }
    if (!outputFile.is_open()) {
        cerr << "Failed to open " << outputFileName << endl;
        return;
    }

    vector<Node*> bestRangeCover;
    vector<string> bestRangeCoverString;
    vector<vector<string>> ans;
    string line;

    while (getline(inputFile, line)) {
        stringstream ss(line);
        string xStr, yStr;
        getline(ss, xStr, ','); // Get x value up to the comma
        getline(ss, yStr); // Get y value after the comma

        int start = stoi(xStr); // Convert x from string to integer
        int end = stoi(yStr); // Convert y from string to integer

        bestRangeCover.clear();
        // bestRangeCoverString.clear();
        bestRangeCover = find_best_range_cover(root,start,end);
        outputFile << xStr << "," << yStr << ",";
        for (size_t i = 0; i < bestRangeCover.size(); ++i) {
            outputFile << bestRangeCover[i]->keyword;
            if (i < bestRangeCover.size() - 1) {
                outputFile << ","; // Separate keywords with commas
            }
        }
        outputFile << endl; // New line for each range cover
       
	    
    }
    outputFile.close();
    inputFile.close();

    ifstream infile(outputFileName); 
    vector<string> lines;

    if (infile) {
        string line;
        // Read all lines from the file
        while (getline(infile, line)) {
            lines.push_back(line);
        }
        infile.close();

        // Sort the lines based on the number of entries per row
        sort(lines.begin(), lines.end(), [](const string& a, const string& b) {
            return countEntries(a) < countEntries(b);
        });

        // Write the sorted lines back to the file
        ofstream outfile(outputFileName);
        if (outfile) {
            for (const auto& line : lines) {
                outfile << line << endl;
            }
            outfile.close();
            // cout << "Sorting completed and written back to database.txt" << endl;
        } else {
            cerr << "Error: Unable to write to the file.\n";
        }
    } else {
        cerr << "Error: Unable to open the file.\n";
    }

}

int main()
{
	// std::string raw_db_file = "../../databases/db6k.dat";
	std::string raw_db_file = "../../databases/tree_details.dat";
	std::string raw_db_file_for_tree = "../../databases/db6k.dat";
	std::string meta_db_file = "../../databases/meta_db6k.dat";
    std::string range_input_file = "../../databases/range_input.txt";
    std::string range_output_file = "../../databases/range_output.txt";

	std:: vector<unsigned int> frequency;
	
	std::vector<std::string> db_kw;
	std::vector<unsigned int> bin_boundaries;
	std::vector<std::set<unsigned int>> raw_db;

	std::vector<std::string> db_mkw;
	std::vector<std::string> db_mkw_hash;
	
	std::vector<std::string>c_mkw;
	std::vector<unsigned int> ra;
	std::vector<unsigned int> s_vec;
	std::vector<unsigned int> bin;
	std::vector<vector<float>> size1;
	std::vector<std::set<unsigned int>> meta_db;
	std::vector<vector<std::string>> mkws_hash;
	std:: vector<vector<unsigned int>> bin_vector;
	std:: vector<unsigned int> freq_vec;

	vector<float> f;
	vector<string> fin;
	vector<set<string>> ham;
	stringstream ss;
	
	vector<string> result;

	int bucket_size = 5;
	int count = 0;

	auto start1 = high_resolution_clock::now();
	/** Reading data fot Tree creation from Inverted Index file **/
	unsigned long minId = numeric_limits<unsigned long>::max();
    unsigned long maxId = 0;

    Node* root = nullptr;
    map<int, string> db6kMap;

	cout<<"[*] Creating Tree for Range Search\n\n";

	if (read_data(raw_db_file_for_tree, root, minId, maxId, db6kMap) != 0) {
        cerr << "Failed to read data" << endl;
        return -1;
    }
	
	outputNodeDetailsToTxt(root, "../../databases/intermediate_tree_details1.txt",db6kMap);
    splitNodeDetailsToTxt("../../databases/intermediate_tree_details1.txt", "../../databases/intermediate_tree_details2.txt");
    finalDatabaseGeneration("../../databases/intermediate_tree_details2.txt", "../../databases/tree_details.dat");
	
    // ans = perform_range_search(bestRangeCover);
    // for(auto i:ans){
    //     cout<<i<<" ";
    // }

    findBRC(root,range_input_file,range_output_file);

    cout<<"[*] tree Creation Done\n\n";
	
	/////////////////////////////////////////////////////
	
    // print_tree(root);
	/** Reading data from Enron Inverted Index file **/
	cout<<"[*] Reading Enron Inverted Index\n\n";

	DB_ReadFromFileToDatabase(raw_db_file, db_kw, raw_db);

	cout<<"[*] Rawdb generated\n\n";

	// cout<<">>  Rawdb size = "<<raw_db.size()<<"\n";

	for(auto v:raw_db){
		auto row_size = v.size();
		count += row_size;
		// cout<<">>  Rawdb size = "<<v.size()<<"\n";
		for(auto vv:v){
			// cout<<vv<<", ";
		}
		// std::cout << std::endl;
	}

	// for(int i =0; i <=raw_db.size(); i++)
	// {
	// 	count += raw_db[i].size();

	// }
	cout<< ">>  Total unique (keyword-id) pairs = "<< count <<"\n";
	
	/** MetaDB generation function  **/
	cout << "[*] Generating Metakeywords --\n";
	DB_GenMKW(meta_db, db_mkw,bin_boundaries, raw_db,bucket_size,db_mkw_hash,frequency);
	
	cout<<"[*] Metakeywords and MetaDB generated\n";

	auto stop1 = high_resolution_clock::now();
    auto duration1 = duration_cast<microseconds>(stop1 - start1);
	cout << "[*] Time taken for metadb generation: "<< duration1.count() << " microseconds" << endl;

	DB_WriteFromDatabaseToFile(meta_db_file, db_mkw_hash, meta_db, KW_TYPE::HEX);

	// vector<string> hw_vec;
	// vector<vector<string>> hw_mkw;

	
	/**    Generating Query vectors of different hamming weights   **/   
	// for(int i=2;i<6;i+=1)
	// {
	// 	set<string> p;
	// 	for(int counter=0;counter<10;counter++)
	// 	{
	// 		ss.clear();
	// 		p.insert(gen(i,N_KW));

	// 	}
	// 	ham.push_back(p);
	// }
	
	Query_processing(ham);

	/**  Query processing  **/
	auto start2 = high_resolution_clock::now();
	
	
	for (int i=0;i<ham.size();i++)
	{
		bin_vector.clear();
		int c = 0;
		f.clear();
	 	for(auto x:ham.at(i))
	   	  { 
			c++;
		  	//hw_vec.push_back(x);
		  	mkws_hash.clear();
		  	s_vec.clear(); 

		  	DB_ConvertQueryToMetaKeyword(mkws_hash, bin_boundaries, x, bucket_size, bin_vector, s_vec, db_mkw_hash, meta_db, frequency, freq_vec);

			ra.clear();
			resultact(x,raw_db,ra);

			//double press=(double)ra.size()/(1.0*s_vec.size());
			double prec = precision(ra, s_vec);

			//cout<<"\n>>  Result_Actual = "<< ra.size();
			//cout<<"\n>>  Result_Superset = "<<s_vec.size();
			//cout<<"\n>>  Precision = "<<prec<<"\n";
			
		}
	//	hw_mkw.push_back(hw_vec);
	//	hw_vec.clear();
	
	}

	auto stop2 = high_resolution_clock::now();
	auto duration2 = duration_cast<microseconds>(stop2 - start2);
        

/*	cout<<"Writing to file started\n";
        std::ofstream outputfile;
        outputfile.open(test_vector,std::ios_base::out);
        for (int i=0;i<hw_mkw.size();i++)
        {
               // outputfile<<"\n.............HW="<<(i+2)<<".........................\n";
               // outputfile<<"\nR_Act"<<"\t"<<"R_Sup"<<"\t"<<"Precision\n\n";

                int p=0;
                for(auto yy:hw_mkw[i])
                {
                        //p++;
                        //c = c+yy;     
                        outputfile << yy << ",";
                        //if(p%2==0)
                       

                }
		outputfile<<"\n";
               // outputfile<<avg[i]<<"\n";


        }
        outputfile.close();
*/


	cout << "[*] Time taken for processing query: "<< duration2.count() << " microseconds\n" << endl;


	  
	meta_db.clear();
	raw_db.clear();
	mkws_hash.clear();
	size1.clear();


	return 0;
}

