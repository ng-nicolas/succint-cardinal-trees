#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>

#define MIN_DEGREE 950
#define VOCABULARY_SIZE 5000
#define MARGIN_DEGREE 100
#define MIN_HEIGHT 2
#define MAX_HEIGHT 4
using namespace std;

/*
struct specs {
	int MIN_DEGREE = 300000;
	int MARGIN_DEGREE = 100;
	int MIN_HEIGHT = 2;
	int VOCABULARY_SIZE = 10000000;
	int MAX_HEIGHT = 3;
}
*/

string multiply_str(string a, size_t n) {
	string b = "";
	for (size_t i=0; i<n; i++) b += a;
	return b;
}

bool check_bp(string bp) {
	size_t open = 0;
	size_t close = 0;
	for (size_t i=0; i<bp.size(); i++) {
		if (bp[i] == '(') open++;
		else if (bp[i] == ')') close++;
		if (open < close) return false;
	}
	cout << "open: " << open << endl;
	cout << "close: " << close << endl;
	return open == close;
}

bool belong(vector<uint32_t>* v, uint32_t s) {
	for (auto i=v->begin(); i<v->end(); i++) {
		if ((*i) == s) return true;
	}
	return false;
}


void generate_symbols(vector<uint32_t>* v, size_t N, uint32_t& last) {
	srand(time(NULL));
	if (last+N > VOCABULARY_SIZE+MIN_DEGREE/4) last = 1;
	for (size_t i=0; i<N; i++) {
		v->push_back(++last);
	}
}

typedef struct {
	uint32_t count_nodes = 0;
	uint32_t max_arity = 0;
	uint32_t average_arity = 0;
	uint32_t max_height = 0;
	uint32_t average_height = 0;
	uint32_t count_leaf = 0;
	uint32_t count_nodes_no_leaf = 0;
	uint32_t current_height = 0;
	uint32_t open = 0;
	uint32_t close = 0;
} tree_info_t;

void random_tree_to_file(size_t current_height, ofstream& out_file, ofstream& letts_file, tree_info_t* tree_info, uint32_t tree_info_current_height, uint32_t& symbol) {
	current_height++;
	if (current_height == 1) {
		cout << "print first (" << endl;
		uint64_t aux = 0;
		letts_file.write((char*)&aux, sizeof(uint64_t));
		letts_file.write((char*)&aux, sizeof(uint32_t));
		out_file << "(";
		tree_info->open++;
	}
	
	if (current_height  >= MIN_HEIGHT) {
		int num  = rand() % 100 + 1;
		if (num <= 99) current_height+=MAX_HEIGHT;
	}
	
	tree_info_current_height++;
	string bp_tmp = "";

	if (current_height >= MAX_HEIGHT) {
		if (tree_info->close > tree_info->open) cout << "[ERROR] Parentheses check is wrong! There are more close parentheses than open parentheses." << endl;
		out_file << ")";
		tree_info->close++;
		tree_info->count_nodes = tree_info->count_nodes + 1;
		tree_info->count_leaf = tree_info->count_leaf + 1;
		tree_info->average_height += tree_info_current_height;
		if (tree_info->max_height < tree_info_current_height) tree_info->max_height = tree_info_current_height;
	} else {
		size_t degree = (rand() % MARGIN_DEGREE) + MIN_DEGREE;
		if (degree > tree_info->max_arity) tree_info->max_arity = degree;
		tree_info->average_arity += degree;
		tree_info->count_nodes_no_leaf++;
		tree_info->count_nodes = tree_info->count_nodes + 1;

		bp_tmp = multiply_str("(", degree) + ")";
		tree_info->open += degree;
		tree_info->close++;
		if (tree_info->close > tree_info->open) cout << "[ERROR] 02 Parentheses check is wrong! There are more close parentheses than open parentheses." << endl;
		out_file << bp_tmp;

		// Generate and write symbols.
		vector<uint32_t> symbols;
		generate_symbols(&symbols, degree, symbol);
		for (auto i=symbols.begin(); i<symbols.end(); i++) letts_file.write((char*)&(*i), sizeof(uint32_t));

		for (size_t i=0; i<degree; i++) {
			random_tree_to_file(current_height, out_file, letts_file, tree_info, tree_info_current_height, symbol);
		}
	}
}


void check_files(string bp, string letts) {
	cout << "Check Integrity..." << endl;
	ifstream bp_f, letts_f;
	cout << "name file: " << letts << endl;

	letts_f.open(letts);
	uint64_t total_nodes, total_symbols;
	uint32_t aux;
	letts_f.read((char*)&total_nodes, sizeof(uint64_t));
	cout << "total_nodes read: " << total_nodes << endl;
	total_symbols = total_nodes - 1;

	cout << "01" << endl;
	letts_f.read((char*)&aux, sizeof(uint32_t));
	cout << "02" << endl;
	uint32_t* array = (uint32_t*)malloc(sizeof(uint32_t)*total_symbols);
	cout << "03" << endl;
	letts_f.read((char*)array, sizeof(uint32_t)*total_symbols);
	cout << "04" << endl;
	for (size_t i=0; i<10; i++) cout << array[i] << "|";
	cout << endl;

	cout << "05" << endl;
	letts_f.close();
	bp_f.open(bp);
	char* p = (char*)malloc(sizeof(char)*total_nodes*2);
	bp_f.read((char*)p, total_nodes*2);
	if (check_bp(p) == true) cout << "balanced sequence." << endl;
	cout << "Check Integrity... OK";

}

int main(int argc, const char *argv[])
{
	
	srand(time(NULL));
	// option 1.
	ofstream out_file, out_symbols;
	out_file.open("random_tree.ascii", ios::out);
	out_symbols.open("random_tree.letts", ios::out | ios::binary);
	tree_info_t tree_info;
	uint32_t symbol = 1;
	random_tree_to_file(0, out_file, out_symbols, &tree_info, 1, symbol);

	uint64_t count_nodes = tree_info.count_nodes;
	out_symbols.seekp(0);
	out_symbols.write((char*)&count_nodes, sizeof(uint64_t));
	cout << "count_nodes: " << count_nodes << endl;
	cout << "ascii total bytes: " << count_nodes * 2 << endl;
	cout << "letts total bytes: " << 8 + count_nodes * 4 << endl;
	out_file.close();
	out_symbols.close();

	check_files("random_tree.ascii", "random_tree.letts");
	cout << "Tree info" << endl;
	cout << "max arity: " << tree_info.max_arity << endl;
	cout << "average arity: " << (double) tree_info.average_arity / tree_info.count_nodes_no_leaf << endl;
	cout << "max height: " << tree_info.max_height << endl;
	cout << "average height: " << (double) tree_info.average_height / tree_info.count_leaf << endl;

	cout << tree_info.max_arity << "|" << (double) tree_info.average_arity / tree_info.count_nodes_no_leaf
			<< "|" << tree_info.max_height << "|" << (double) tree_info.average_height / tree_info.count_leaf << endl;
	return 0;
}
