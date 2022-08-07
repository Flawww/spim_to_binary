#pragma once
#include <cstdint>

template<class To, size_t NumBits = 0, class From>
static To& bit_cast(const From& in) {
	static_assert(sizeof(To) * 8 >= NumBits && sizeof(From) * 8 >= NumBits, "bit_cast with too many target bits, would cause overflow");

	constexpr size_t num_bits = (NumBits > 0) ? NumBits : std::min(sizeof(From), sizeof(To)) * 8;
	constexpr size_t num_bytes = num_bits / 8;
	constexpr size_t rem_bits = num_bits % 8;
	constexpr size_t mask = (rem_bits != 0) * ~(uint64_t)0 >> (64 - rem_bits);

	To var{};
	// copy as many full bytes as possible
	memcpy(&var, &in, num_bytes);
	// copy remaining bits
	*((uint8_t*)(&var) + num_bytes) |= *((uint8_t*)(&in) + num_bytes) & mask;

	return var;
}


struct instruction {
	instruction(uint32_t hex) : hex(hex) {}

	struct r_format {
		uint32_t funct : 6;
		uint32_t shift : 5;
		uint32_t rd : 5;
		uint32_t rt : 5;
		uint32_t rs : 5;
		uint32_t opcode : 6;
	};

	struct i_format {
		uint32_t imm : 16;
		uint32_t rt : 5;
		uint32_t rs : 5;
		uint32_t opcode : 6;
	};

	struct j_format {
		uint32_t p_addr : 26; // pseudo address
		uint32_t opcode : 6;
	};

	union {
		r_format r;
		i_format i;
		j_format j;
		uint32_t hex;
	};
};

enum instructions : int {
	BEQ = 0x04,
	BNE = 0x05,
	BLEZ = 0x06,
	BGTZ = 0x07,
};