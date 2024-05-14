#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std;

struct Node {
    int value = -1;
    bool is_leaf = false;
    bool is_present = false;
    std::string keyword;
    int start = -1;
    int end = -1;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* parent = nullptr;
};

Node* ensureNode(Node*& current, int start, int end);
void setNodeProperties(Node* node, int value, const std::string& keyword, bool isLeaf, bool isPresent);
std::pair<int, int> parseRangeFromKeyword(const std::string& keyword);
void print_tree(Node* root);

Node* recreateTreeFromOutput(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return nullptr;
    }

    Node* root = nullptr;
    std::string line;
    while (getline(file, line)) {
        std::istringstream iss(line);
        std::string token, keyword;
        std::vector<std::string> path;
        int leafValue;

        // Skip the first column (ID)
        getline(iss, token, ',');

        // Read the leaf node value
        getline(iss, token, ',');
        leafValue = std::stoi(token);

        // Read the rest of the path
        while (getline(iss, token, ',')) {
            keyword = token;
        }

        auto [start, end] = parseRangeFromKeyword(keyword);
        Node* node = ensureNode(root, start, end);
        setNodeProperties(node, leafValue, keyword, true, true);
    }

    file.close();
    return root;
}

Node* ensureNode(Node*& current, int start, int end) {
    if (current == nullptr) {
        current = new Node();
        current->start = start;
        current->end = end;
        current->keyword = "kw_" + std::to_string(start) + (start == end ? "" : "_" + std::to_string(end));
        return current;
    }

    if (start == current->start && end == current->end) {
        return current;
    }

    if (end <= (current->start + current->end) / 2) {
        return ensureNode(current->left, start, end);
    } else {
        return ensureNode(current->right, start, end);
    }
}

void setNodeProperties(Node* node, int value, const std::string& keyword, bool isLeaf, bool isPresent) {
    node->value = value;
    node->is_leaf = isLeaf;
    node->is_present = isPresent;
    node->keyword = keyword;
}

std::pair<int, int> parseRangeFromKeyword(const std::string& keyword) {
    auto pos = keyword.find('_');
    auto range = keyword.substr(pos + 1);
    pos = range.find('_');
    int start = std::stoi(range.substr(0, pos));
    int end = pos != std::string::npos ? std::stoi(range.substr(pos + 1)) : start;
    return {start, end};
}
// Your existing print_tree function for verification
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


int main() {
    string filename = "intermediate_tree_details1.txt"; // Replace with your filename
    Node* root = recreateTreeFromOutput(filename);
    print_tree(root);
    // Remember to add code to delete the tree to avoid memory leaks
    return 0;
}
