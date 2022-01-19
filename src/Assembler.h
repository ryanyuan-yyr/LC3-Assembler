#pragma once
#include"bit_vector.h"
#include<map>
#include<iostream>
#include<string>
#include<vector>
#include<functional>
#include<utility>
#include<deque>
#include<sstream>
#include<utility>
#include<list>
#include<stack>
#include<set>
#include<stdexcept>

using bit_vec = bit_vector<16>;

namespace ryl
{
	std::istream& get_word(std::istream& is, std::string& des);

	class Assembler
	{
	private:
		using bin_instruction = bit_vector<16>;
		using address = bit_vector<16>;
		std::map<std::string, size_t> symbolTable;

		/**
		 * @brief All information one needs to know to converts an argument to machine language.  
		 * arg_process: A callable object. 
		 *     param: string-type argument, such as `R0`, `#0`.  
		 * bitRange: Demonstrating the range of bits of the argument in the instruction.  E.g.The BitRange of the 1st argument in ADD is from 9 to 11
		*/
		struct Arg {
			std::function<bit_vec(const std::string&)> arg_process;
			BitRange bitRange;
		};
		std::map<std::string, std::list<Arg>> InstructionFormatTable;

		size_t orig_line_count = 0, processed_line_count = 0;

		std::multimap<std::size_t, std::string> error_table;

		static std::function<bit_vec(const std::string&)> str_to_opcode;
		static std::function<bit_vec(const std::string&)> str_to_reg;
		static std::function<bit_vec(const std::string&, size_t, bool)> str_to_imm;
		static std::function<std::pair<bit_vec, bool>(const std::string&, size_t)> str_to_reg_or_imm;
		static std::function<bit_vec(const std::string&)> imm_reg_overload;

		static std::map<std::string, bit_vec> opcode;

	public:

		std::function<bit_vec(const std::string&, size_t)> label_or_offset =
			[&](const std::string& str, size_t offset_bit) {
			if (symbolTable.find(str) == symbolTable.end()) {
				try
				{
					return str_to_imm(str, offset_bit, false);
				}
				catch (const std::invalid_argument& e)
				{
					throw(std::invalid_argument("Undefined label " + str));
				}
				catch (const std::out_of_range& e)
				{
					throw(std::out_of_range("Overflow. " + std::to_string(offset_bit) + " bits only"));
				}
			}
			else {
				try
				{
					return bit_vec(symbolTable[str] - processed_line_count - 1, offset_bit, false);
				}
				catch (const std::out_of_range& e)
				{
					throw(std::out_of_range(std::string(e.what()) + ". Label is out of range. "));
				}
			}
		};

		auto get_error_table() {
			return error_table;
		}

	public:
		/**
		 * @brief Initializes InstructionFormatTable
		*/
		Assembler() {
			InstructionFormatTable["ADD"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["ADD"].push_back({ str_to_reg,{9,3} });
			InstructionFormatTable["ADD"].push_back({ str_to_reg,{6,3} });
			InstructionFormatTable["ADD"].push_back({ imm_reg_overload, { 0,6 } });

			InstructionFormatTable["AND"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["AND"].push_back({ str_to_reg,{9,3} });
			InstructionFormatTable["AND"].push_back({ str_to_reg,{6,3} });
			InstructionFormatTable["AND"].push_back({ imm_reg_overload, { 0,6 } });

			InstructionFormatTable["BR"].push_back({ str_to_opcode, {12,4} });
			InstructionFormatTable["BR"].push_back({ [](const std::string& str_condition_code) {
					bit_vec res(3);
					res[0] = str_condition_code[2] - '0';
					res[1] = str_condition_code[1] - '0';
					res[2] = str_condition_code[0] - '0';
					return res; },{9,3} });
			InstructionFormatTable["BR"].push_back({ std::bind(label_or_offset,std::placeholders::_1,9), {0,9} });

			InstructionFormatTable["JMP"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["JMP"].push_back({ [](const std::string& jmp_str) {
				return str_to_reg(jmp_str).resize(6).resize(12,true); },{0, 12} });

			InstructionFormatTable["JSR"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["JSR"].push_back({ [&](const std::string& offset) {auto res = this->label_or_offset(offset, 11); res.push_back(1); return res; },{0,12} });

			InstructionFormatTable["JSRR"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["JSRR"].push_back({ [](const std::string& jmp_str) {
				return str_to_reg(jmp_str).resize(6).resize(12,true); },{0, 12} });

			InstructionFormatTable["LD"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["LD"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["LD"].push_back({ std::bind(label_or_offset,std::placeholders::_1,9),{0,9} });

			InstructionFormatTable["LDI"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["LDI"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["LDI"].push_back({ std::bind(label_or_offset,std::placeholders::_1,9),{0,9} });

			InstructionFormatTable["LDR"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["LDR"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["LDR"].push_back({ str_to_reg, {6,3} });
			InstructionFormatTable["LDR"].push_back({ std::bind(label_or_offset,std::placeholders::_1,6),{0,6} });

			InstructionFormatTable["LEA"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["LEA"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["LEA"].push_back({ std::bind(label_or_offset,std::placeholders::_1,9),{0,9} });

			InstructionFormatTable["NOT"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["NOT"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["NOT"].push_back({ [](const std::string& reg) {
				auto res = str_to_reg(reg);
				for (size_t i = 0; i != 6; i++)
					res.push_front(true);
				return res; }, {0,9} });

			InstructionFormatTable["RET"].push_back({ [](const std::string& op) {
				auto res = str_to_opcode(op);
				res = res.resize(16, true);
				res[6] = res[7] = res[8] = true;
				return res;
				},{0,16} });

			InstructionFormatTable["RTI"].push_back({ [](const std::string& op) {
				auto res = str_to_opcode(op);
				res.resize(16, true);
				return res;
				},{0,16} });

			InstructionFormatTable["ST"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["ST"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["ST"].push_back({ std::bind(label_or_offset,std::placeholders::_1,9),{0,9} });

			InstructionFormatTable["STI"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["STI"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["STI"].push_back({ std::bind(label_or_offset,std::placeholders::_1,9),{0,9} });

			InstructionFormatTable["STR"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["STR"].push_back({ str_to_reg, {9,3} });
			InstructionFormatTable["STR"].push_back({ str_to_reg, {6,3} });
			InstructionFormatTable["STR"].push_back({ std::bind(label_or_offset,std::placeholders::_1,6),{0,6} });

			InstructionFormatTable["TRAP"].push_back({ str_to_opcode,{12,4} });
			InstructionFormatTable["TRAP"].push_back({ [](const std::string& reg) {
				auto res = str_to_imm(reg, 8, true);
				res.resize(12);
				return res; },{0,12} });

			InstructionFormatTable["HALT"].push_back({ [](const std::string& halt) {
				return bit_vec(0xF025, 16 , true); }, { 0,16 } });
		}

		auto get_symbol_table() {
			return symbolTable;
		}

		struct assembledInstruction
		{
			bin_instruction instr;
			size_t orig_line;
		};

		/**
		 * @brief To assemble the source file
		 * @param first, last: Designate a range from which we can read lines of source file. Expects a string object after dereferencing InputIt.  
		 * @param output: To where write the converted instructions
		 * @return If assembled successfully
		*/
		template<typename InputIt, typename OutputIt>
		bool assembly(InputIt first, InputIt last, OutputIt output);
	};

	std::pair<bool, std::string> nzpSet(const std::string& str);

	template<typename InputIt, typename OutputIt>
	inline bool Assembler::assembly(InputIt first, InputIt last, OutputIt output)
	{
		std::vector<std::pair<std::deque<std::string>, size_t>> processedAssemblySrc;
		bool exit_from_end = false, has_orig = false;
		for (auto iter = first; iter != last; iter++) {
			std::string& line = *iter;

			std::istringstream line_stream(line);
			std::string cur_word;
			std::deque<std::string> cur_line_container;

			while (get_word(line_stream, cur_word)) {
				cur_line_container.push_back(cur_word);
			}

			if (cur_line_container.size() == 0) {
				orig_line_count++;
				continue;
			}
			else {
				if (InstructionFormatTable.find(cur_line_container[0]) == InstructionFormatTable.end() && cur_line_container[0][0] != '.') {
					bool isBR = false;
					std::string condition_code;
					if (cur_line_container[0].substr(0, 2) == "BR") {
						tie(isBR, condition_code) = nzpSet(cur_line_container[0].substr(2));
					}
					if (!isBR) {
						// Error: Multiple address for one label
						if (symbolTable.find(cur_line_container[0]) != symbolTable.end()) {
							error_table.insert({ orig_line_count, "Multiple addresses for label " + cur_line_container[0] });
						}
						symbolTable[cur_line_container[0]] = processed_line_count;
						cur_line_container.pop_front();

						if (cur_line_container[0].substr(0, 2) == "BR") {
							tie(isBR, condition_code) = nzpSet(cur_line_container[0].substr(2));
							if (isBR) {
								cur_line_container.pop_front();
								cur_line_container.push_front(condition_code);
								cur_line_container.push_front("BR");
							}
							else
							{
								error_table.insert({ orig_line_count,"Unresolvable condition code " + cur_line_container[0] });
							}
						}
					}
					else {
						cur_line_container.pop_front();
						cur_line_container.push_front(condition_code);
						cur_line_container.push_front("BR");
					}
				}

				if (cur_line_container.size() != 0) {
					// pseudo-operate
					if (cur_line_container[0] == ".BLKW")
						processed_line_count += str_to_imm(cur_line_container[1], 16, true).to_ullong() - 1;
					else if (cur_line_container[0] == ".STRINGZ")
						processed_line_count += cur_line_container[1].size() - 2;
					else if (cur_line_container[0] == ".ORIG") {
						has_orig = true;
						if (processed_line_count != 0) {
							error_table.insert({ orig_line_count, ".ORIG should be at the 1st line of asm file" });
						}
					}
					else if (cur_line_container[0] == ".END") {
						exit_from_end = true;
						break;
					}
				}

				processedAssemblySrc.push_back(std::make_pair(cur_line_container, orig_line_count));
				processed_line_count++;
			}
			orig_line_count++;
		}

		if (!exit_from_end) {
			error_table.insert({ orig_line_count,"No .END at the end of file" });
		}

		if (!has_orig) {
			error_table.insert({ orig_line_count,"No .ORIG detected" });
		}

		processed_line_count = 0;

		for (auto& instr_lineNum_pair : processedAssemblySrc)
		{
			auto bit_lineNum_pair = std::make_pair(bit_vec(), instr_lineNum_pair.second);
			auto& instruction = instr_lineNum_pair.first;

			if (instruction.size() == 0) {
				error_table.insert({ instr_lineNum_pair.second,"Empty line" });
				processed_line_count++;
			}
			else if (instruction[0][0] != '.') {
				if (InstructionFormatTable.find(instruction[0]) == InstructionFormatTable.end()) {
					error_table.insert({ instr_lineNum_pair.second,"Unresolvable instruction " + instruction[0] });
				}
				else if (instruction.size() != InstructionFormatTable[instruction[0]].size()) {
					error_table.insert({ instr_lineNum_pair.second,"Wrong number of argument(s).  " });
				}
				else {
					try
					{
						size_t idx = 0;
						for (auto& arg : InstructionFormatTable[instruction[0]])
						{
							bit_lineNum_pair.first.partial_assign(arg.arg_process(instruction[idx]), arg.bitRange.front_bit);
							idx++;
						}
					}
					catch (const std::exception& e) {
						error_table.insert({ instr_lineNum_pair.second,e.what() });
					}
					*output++ = bit_lineNum_pair;
				}
				processed_line_count++;
			}
			else {
				if (instruction[0] == ".FILL") {
					try
					{
						bit_lineNum_pair.first = str_to_imm(instruction[1], 16, false);
					}
					catch (const std::exception& e) {
						error_table.insert({ instr_lineNum_pair.second,e.what() });
					}
					*output++ = bit_lineNum_pair;
					processed_line_count++;
				}
				else if (instruction[0] == ".BLKW") {
					if (instruction.size() != 2) {
						error_table.insert({ instr_lineNum_pair.second,"Wrong number of argument(s).  " });
					}
					else {
						unsigned long long bulk_num;
						try {
							bulk_num = str_to_imm(instruction[1], 16, true).to_ullong();
						}
						catch (const std::exception& e) {
							error_table.insert({ instr_lineNum_pair.second,e.what() });
						}
						bit_lineNum_pair.first.reset();
						for (size_t i = 0; i != bulk_num; i++)
						{
							*output++ = bit_lineNum_pair;
						}
						processed_line_count += bulk_num;
					}
				}
				else if (instruction[0] == ".STRINGZ") {
					if (instruction.size() != 2)
					{
						error_table.insert({ instr_lineNum_pair.second,"Wrong number of argument(s).  " });
					}
					else {
						for (size_t i = 1; i < instruction[1].size() - 1; i++)
						{
							bit_lineNum_pair.first = bit_vec(instruction[1][i], 16, false);
							*output++ = bit_lineNum_pair;
						}
						bit_lineNum_pair.first.reset();
						*output++ = bit_lineNum_pair;
						processed_line_count += instruction[1].size() - 1;
					}
				}
				else if (instruction[0] == ".ORIG") {
					if (instruction.size() != 2) {
						error_table.insert({ instr_lineNum_pair.second,"Wrong number of argument(s).  " });
					}
					else {
						try {
							*output++ = { str_to_imm(instruction[1], 16, true),instr_lineNum_pair.second };
						}
						catch (const std::exception& e) {
							error_table.insert({ instr_lineNum_pair.second,e.what() });
						}
					}
					processed_line_count++;
				}
				else if (instruction[0] == ".END")
					break;
				else
					error_table.insert({ instr_lineNum_pair.second,"Unresolvable pseudo-operator " + instruction[0] });
			}
		}
		return error_table.size() == 0;
	}
}
