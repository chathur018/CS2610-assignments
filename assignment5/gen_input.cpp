#include<iostream>
#include<fstream>
using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: ./a.out N output_filename" << endl;
		exit(0);
	}
	int n = atoi(argv[1]);
	string outfilename(argv[2]);
	ofstream outfile;
	outfile.open(outfilename, ios::out | ios::trunc);
	if (!outfile.is_open()) {
		cout << "Error: Cannot open file " << outfilename << " for writing" << endl;
		exit(0);
	}

	srand((unsigned) time(NULL));

	// Write the value of N
	outfile << n << endl;

	// Write the two matrices
	for(int i=0; i<(2*n); i++) {
		for(int j=0; j<n; j++) {
			outfile << rand()%10 << " ";
		}
		outfile << endl;
	}
}
