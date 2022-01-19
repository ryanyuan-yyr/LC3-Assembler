#include "Assembler.h"

std::function<bit_vec(const std::string&)> ryl::Assembler::str_to_opcode = [](const std::string& str_opcode) {
	if (opcode.find(str_opcode) == opcode.end())
		throw(std::invalid_argument("Cannot find corresponding opcode " + str_opcode));
	auto res = bit_vec(opcode[str_opcode]);
	return res.resize(4);
};

std::function<bit_vec(const std::string&)> ryl::Assembler::str_to_reg =
[](const std::string& str_reg) {
	if (str_reg.size() != 2 or
		str_reg[0] != 'R' or
		str_reg[1] < '0' or str_reg[1] > '7')
		throw(std::invalid_argument("Invalid argument as a register.  " + str_reg));
	return bit_vec(str_reg[1] - '0', 3, true);
};

std::function<bit_vec(const std::string&, size_t, bool)> ryl::Assembler::str_to_imm =
[](const std::string& str_imm, size_t bit_len, bool is_unsigned) {
	int base;
	switch (str_imm[0])
	{
	case '#':
		base = 10;
		break;
	case 'x':
		base = 16;
		break;
	case 'o':
		base = 8;
		break;
	case 'b':
		base = 2;
		break;
	default:
		throw (std::invalid_argument("Cannot determine the base of immediate " + str_imm));
		break;
	}
	bit_vec res;

	// May throw
	res = bit_vec(std::stol(str_imm.substr(1, str_imm.size() - 1), 0, base), bit_len, is_unsigned);
	return res;
};

std::function <std::pair<bit_vec, bool >(const std::string&, size_t) > ryl::Assembler::str_to_reg_or_imm =
[](const std::string& str, size_t bit_len) {
	if (str[0] == 'R')
		return  std::make_pair(str_to_reg(str).resize(bit_len), false);
	else
		return  std::make_pair(str_to_imm(str, bit_len, false), true);
};

std::function<bit_vec(const std::string&)> ryl::Assembler::imm_reg_overload = [](const std::string& str) {
	auto res_pair = str_to_reg_or_imm(str, 5);
	res_pair.first.push_back(res_pair.second);
	return res_pair.first; };

std::map<std::string, bit_vec> ryl::Assembler::opcode = {
	{"ADD", bit_vec(0x1, 4, true)},
	{"AND", bit_vec(0x5, 4, true)},
	{"BR", bit_vec(0x0, 4, true)},
	{"JMP", bit_vec(0xC, 4, true)},
	{"JSR", bit_vec(0x4, 4, true)},
	{"JSRR", bit_vec(0x4, 4, true)},
	{"LD", bit_vec(0x2, 4, true)},
	{"LDI", bit_vec(0xA, 4, true)},
	{"LDR", bit_vec(0x6, 4, true)},
	{"LEA", bit_vec(0xE, 4, true)},
	{"NOT", bit_vec(0x9, 4, true)},
	{"RET", bit_vec(0xC, 4, true)},
	{"RTI", bit_vec(0x8, 4, true)},
	{"ST", bit_vec(0x3, 4, true)},
	{"STI", bit_vec(0xB, 4, true)},
	{"STR", bit_vec(0x7, 4, true)},
	{"TRAP", bit_vec(0xF, 4, true)},
	{"LC3", bit_vec(0xD, 4, true)}
};

std::istream& ryl::get_word(std::istream& is, std::string& des)
{
	bool is_reading_word = false;
	bool is_reading_string = false;
	std::string res;
	char c;
	while (c = is.get(), is)
	{
		if (is_reading_string) {
			res.push_back(c);
			if (c == '"')
				break;
			continue;
		}
		if (c == '"' && !is_reading_word) {
			is_reading_string = true;
			is_reading_word = true;
			res.push_back(c);
			continue;
		}
		if (c == ';') {
			if (!is_reading_word)
				is.setstate(std::ios_base::eofbit | std::ios_base::failbit);
			else {
				is.putback(c);
				break;
			}
		}
		else if (isspace(c) or c == ',')
		{
			if (is_reading_word)
				break;
		}
		else
		{
			if (!is_reading_word)
				is_reading_word = true;
			res.push_back(c);
		}
	}

	if (is_reading_word && is.fail())
		is.clear();

	des = std::move(res);
	return is;
}

std::pair<bool, std::string> ryl::nzpSet(const std::string& str) {
	bool isCC = true;
	std::string condition_code = "000";
	for (size_t i = 0; i < str.size(); i++)
	{
		switch (str[i])
		{
		case 'n':
			condition_code[0] = '1';
			break;
		case 'z':
			condition_code[1] = '1';
			break;
		case 'p':
			condition_code[2] = '1';
			break;
		default:
			isCC = false;
			break;
		}
		if (!isCC)
			break;
	}
	return { isCC, condition_code };
}