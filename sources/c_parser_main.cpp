#include <iostream>
#include <fstream>
#include "c_parser.h"


int main(int argc, char* argv[]) {

	if (argc != 3) { //TO-DO: help
		std::cout << "usage:" << std::endl
			<< "c_parser input.c output.gv" << std::endl;
		return 1;
	}
	
	std::ifstream file_in(argv[1]);
	std::ofstream file_out(argv[2]);

	if (!file_in.is_open()) {
		std::cout << "Can't open " << argv[1] << std::endl; //TO-DO: shell load
		return 1;
	}
	if (!file_out.is_open()) {
		std::cout << "Can't open " << argv[2] << std::endl; //TO-DO: shell load
		return 1;
	}

	c_parser parser(file_in);
	parser.run();
	parse_tree<c_token>& tree = parser.get_tree();
	tree.to_dot(file_out);

	file_in.close();
	file_out.close();

	return 0;
}
