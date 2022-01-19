#include<fstream>
#include<list>
#include<iterator>
#include<iomanip>
#include<cmath>
#include"Assembler.h"

using namespace std;
using namespace ryl;

int main(int argc, const char* argv[])
{
	if (argc != 2) {
		cout << "Invalid argument. \n Usage: lc3asm file.asm\n\n";
		return 0;
	}

	ifstream in(argv[1], ios::in);

	if (!in.is_open()) {
		cout << "Unable to open the file\n";
		return 0;
	}

	std::vector<std::string> srcFile;
	{
		string line;
		while (getline(in, line)) {
			srcFile.push_back(line);
		}
	}
	Assembler assemble;

	std::vector<std::pair<bit_vec, size_t>> res;

	auto res_iter = std::inserter(res, res.end());

	bool succAssem = assemble.assembly(srcFile.begin(), srcFile.end(), res_iter);

	if (res.size() == 0) {
		cout << "No instruction detected\n";
		return 0;
	}

	size_t orig_lineNum_w = static_cast<size_t>(log10(res[res.size() - 1].second)) + 1;
	size_t processed_lineNum_w = static_cast<size_t>(log10(res.size() - 1)) + 1;

	size_t processed_lineNum = 0;

	size_t orig_line_Num_cnt = 0;
	for (auto& line : srcFile)
	{
		std::cout
			<< std::setw(orig_lineNum_w) << right <<
			orig_line_Num_cnt++ << ": "
			<< line << "\n";
	}

	cout << "\n";

	if (succAssem)
	{
		size_t addr = res[0].first.to_ullong() - 1;

		bool isfirst = true;

		for (auto& bit_lineNum_pair : res) {
			if (!isfirst)
				cout << std::hex << setw(6) << left << showbase << uppercase <<
				addr
				<< "  ";
			else {
				cout << "        ";
				isfirst = false;
			}

			cout << std::setw(processed_lineNum_w) << std::dec << right <<
				processed_lineNum;

			std::cout << ": " << bit_lineNum_pair.first << "\t\t"
				<< std::setw(orig_lineNum_w) << right <<
				bit_lineNum_pair.second << ": "
				<< srcFile[bit_lineNum_pair.second] << "\n";
			processed_lineNum++;
			addr++;
		}

		cout << "\n";

		for (auto& strSymbol_processedLineNum_pair : assemble.get_symbol_table())
		{
			cout << strSymbol_processedLineNum_pair.first << ": " << strSymbol_processedLineNum_pair.second << "\n";
		}

		ofstream out(std::string(argv[1]) + ".bin", ios::out);
		for (auto& bit_lineNum_pair : res)
		{
			out << bit_lineNum_pair.first << "\n";
		}
	}
	else
	{
		for (auto& lineNum_error_pair : assemble.get_error_table())
		{
			cout << std::setw(orig_lineNum_w) << right <<
				lineNum_error_pair.first << " " << lineNum_error_pair.second << "\n";
		}
	}
}