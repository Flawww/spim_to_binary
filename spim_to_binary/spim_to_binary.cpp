#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "helpers.h"

enum SECTIONS : int {
    INVALID_SECTION = -1,
    TEXT,
    KTEXT,
    DATA,
    KDATA,

    NUM_SECTIONS
};

constexpr const char* section_names[] = {
    ".text", ".ktext", ".data", ".kdata"
};

constexpr const char* section_headers[] = {
    "User Text Segment", "Kernel Text Segment", "User data segment", "Kernel data segment"
};

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return 0;
    }

    std::string filename = argv[1];
    std::filesystem::path filepath(filename);
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        printf("Failed to open file %s\n", filename.c_str());
        return 0;
    }

    int current_section = INVALID_SECTION;
    bool write_address = false;
    std::vector<uint32_t> bytes;

    auto try_write_file = [&]() {
        if (current_section != INVALID_SECTION) {
            std::string out_file_name = filepath.stem().string() + section_names[current_section];
            std::ofstream file_out(out_file_name, std::ios::out | std::ios::binary);
            file_out.write((char*)bytes.data(), bytes.size() * sizeof(uint32_t));
            file_out.close();

            printf("Creating file %s with a size of %X\n", out_file_name.c_str(), bytes.size());

            current_section = INVALID_SECTION;
            bytes.clear();
        }
    };

    std::string line;
    while (std::getline(infile, line)) {
        if (line.length() < 1) {
            continue; // empty line
        }
        line = line.substr(0, 55); // truncate it, we dont need all of it.

        // If the line starts with a [, AND we currently have a section, keep working on the current section.
        if (line[0] == '[' && current_section != INVALID_SECTION) {
            uint32_t val = 0;
            std::stringstream ss(line);
            if (write_address) {       
                ss.seekg(1); // write the first address of this section to the buffer
                ss >> std::hex >> val;

                if (ss.fail()) {
                    throw std::runtime_error("stringstream failed to write hex value to 'val'\n");
                }

                // write start address as the first element in the vector
                bytes.push_back(val);
                write_address = false;
                printf("%s has address %X (%i)\n", section_names[current_section], val, bytes.size());
            }

            ss.seekg(line.find(']') + 1); // skip first []

            if (current_section == TEXT || current_section == KTEXT) {
                ss >> std::hex >> val;
                if (ss.fail()) {
                    throw std::runtime_error("stringstream failed to write hex value to 'val'\n");
                }

                // For some reason spim does relative branches from "PC" instead of "PC + 4" - let's fix that by subtracting 1 instruction (4 bytes) from the imm.
                instruction inst(val);
                if (inst.i.opcode >= BEQ && inst.i.opcode <= BGTZ) {
                    inst.i.imm = bit_cast<int16_t>(inst.i.imm) - 1;
                }

                bytes.push_back(inst.hex);
            }
            else { // DATA AND KDATA
                if (ss.peek() == '.') { // padding
                    ss.seekg(line.find('[', ss.tellg()) + 1); // find start of second bracket (end addr)
                    uint32_t end_addr = 0;
                    ss >> std::hex >> end_addr;
                    bool fail = ss.fail();

                    ss.seekg(line.find(']', ss.tellg()) + 1); // past both brackets. (fill/pad value)
                    ss >> std::hex >> val;
                    fail = fail || ss.fail();

                    ss.seekg(1);
                    uint32_t start_addr = 0;
                    ss >> std::hex >> start_addr;
                    fail = fail || ss.fail();

                    if (fail) {
                        throw std::runtime_error("stringstream failed to write hex values for data pad\n");
                    }
                    // ensure the data is 4-byte aligned (it should always be, this is just a sanity check)
                    if (((end_addr - start_addr) + 1) & 0x3) {
                        throw std::runtime_error("section " + std::string(section_names[current_section]) + " is not aligned to 4 bytes\n");
                    }

                    uint32_t num_new_elems = ((end_addr - start_addr) + 1) / sizeof(uint32_t);
                    bytes.resize(bytes.size() + num_new_elems, val);

                }
                else { // normal data
                    for (int i = 0; i < 4; i++) {
                        ss >> std::hex >> val;
                        if (ss.fail()) {
                            break;
                        }
                        bytes.push_back(val);
                    }
                }
            }   
        }
        else {
            try_write_file();
            // Look for a new section
            for (int i = 0; i < NUM_SECTIONS; i++) {
                if (line.find(section_headers[i]) == 0) { // string starts with the segment header
                    write_address = true;
                    current_section = i;
                }
            }
        }

    }

    try_write_file();

    return 0;
}

