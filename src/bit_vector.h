#pragma once
#include<bitset>
#include<stdexcept>
#include<string>

struct BitRange {
	unsigned short front_bit, len;
};

template<size_t N>
class bit_vector :public std::bitset<N>
{
private:
	size_t i_size = N;
public:

	bit_vector() = default;

	bit_vector(size_t s) :i_size(s) {}

	bit_vector(int value, size_t s, bool is_unsigned) :i_size(s), std::bitset<N>(value) {
		if (i_size == 0)
			return;
		if (!is_unsigned) {
			if (-value > 1 << (s - 1) || value >= 1 << (s - 1))
				throw(std::out_of_range("Overflow: " + std::to_string(i_size) + " bits at most"));
		}
	}

	bit_vector<N> operator++()
	{
		size_t i = 0;
		while (*this[i] && i < i_size)
		{
			*this[i++] = false;
		}
		if (i < i_size)
			*this[i] = true;
		else
		{
			throw(std::out_of_range("Overflow: " + std::to_string(i_size) + " bits at most"));
		}
	}

	size_t size() const {
		return i_size;
	}

	bool operator[](size_t sub) const {
		if (sub < i_size)
			return std::bitset<N>::operator[](sub);
		else
			return false;
	}

	auto operator[](size_t sub) {
		return std::bitset<N>::operator[](sub);
	}

	bit_vector operator()(const BitRange& bitRange) const {
		bit_vector res;
		res.i_size = bitRange.len;
		for (size_t i = bitRange.front_bit; i != bitRange.front_bit + bitRange.len; i++)
		{
			res.push_back(*this[i]);
			return res;
		}
	}

	bit_vector resize(size_t des_size, bool align = false)const {
		bit_vector res;
		size_t i, j;
		if (align) {
			for (i = des_size, j = size(); j != 0 && i != 0; i--, j--)
			{
				res[i - 1] = (*this)[j - 1];
			}
		}
		else {
			for (i = 0, j = 0; j < size() && i < des_size; i++, j++)
			{
				res[i] = (*this)[j];
			}
		}
		res.i_size = des_size;
		return res;
	}

	void push_back(bool val) {
		(*this)[size()] = val;
		i_size++;
	}

	void push_front(bool val) {
		(*this) <<= 1;
		(*this)[0] = val;
		i_size++;
	}

	void partial_assign(const bit_vector& r, size_t start) {
		for (size_t i = 0; i < r.size(); i++)
		{
			(*this)[i + start] = r[i];
		}
	}
};

template<size_t N>
bool operator<(const bit_vector<N>& lhs, const bit_vector<N>& rhs)
{
	size_t i = N;
	while (i)
	{
		if (!lhs[i - 1] && rhs[i - 1])
			return true;
		else if (lhs[i - 1] && !rhs[i - 1])
			return false;
		else
			i--;
	}
	return false;
}
