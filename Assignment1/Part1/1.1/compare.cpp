////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//	Description: Compares first line of two file and checks whether both      //
//				 files have same first lines.                                 //
//	Author: Shiv Bhushan Tripathi                                             //
//	Date: 14 October 2020                                                     //
//									                                          //
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <fstream>
#include <bits/stdc++.h>

using namespace std;

int main (int argc, char *argv[]) {

	ifstream in_file1, in_file2;
	string str1, str2;
	
	// Open the files to read and compare.
	in_file1.open(argv[1], ios::in);
	in_file2.open(argv[2], ios::in);
	
	// Check whether both files are opened successfully.
	assert (in_file1.is_open());
	assert (in_file2.is_open());
	
	// Don't loop here.. just compare only the first line of both files.
	// Get the first line from both files.
	getline(in_file1, str1);
	getline(in_file2, str2);
	
	// If both strings are equal then return 0 else return a value other than
	// zero.
	if (str1.compare(str2) == 0) {
		return 0;
	} else {
		return 1;
	}		

}
