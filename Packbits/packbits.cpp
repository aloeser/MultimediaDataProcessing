#include <iostream>
#include <string>
#include <fstream>
#include <array>

// todo: 128 bit, decomposition, optimization

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&num), size);
}

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&num), size);
}

void packbits_compress(std::ifstream& is, std::ofstream& os) {
    using namespace std;

    // state machine
    int state = 0;
    int count = 0;
    uint8_t curr, prev;
    array<uint8_t, 128> buff;
    // states:
    // 1: only 1 char read, what to do now?
    // 2: run state
    // 3: copy state
    // 4: done reading seq
    // 5: done reading file

    // Setup state machine (initial state)
    raw_read(is, curr);
    prev = curr;
    state = 1;
    count++;

    while (raw_read(is, curr)) {
        switch (state) {
            case 1:
                state = prev == curr ? 2 : 3;
                if (state == 3) { // we are going to copy
                    buff[0] = prev;
                }
                prev = curr;
                count++;
                break;
            case 2:
                if (prev != curr) { // end of run condition
                    raw_write(os, (uint8_t)(257 - count));
                    raw_write(os, prev);
                    count = 0;
                    state = 1;
                }
                prev = curr;
                count++;
                break;
            case 3:
                if (curr == prev) { // end of copy condition, a run started
                    raw_write(os, (uint8_t)(count - 2));
                    for (int i = 0; i < count - 1; ++i) {
                        raw_write(os, buff[i]);
                    }
                    count = 2;
                    state = 2;
                }
                else {
                    buff[count - 1] = prev;
                    prev = curr;
                    count++;
                }
                break;
        }
    }

    if (state != 2) {
        buff[count - 1] = prev;
        count++;
        raw_write(os, (uint8_t)(count - 2));
        for (int i = 0; i < count - 1; ++i) {
            raw_write(os, buff[i]);
        }
    }
    else {
        raw_write(os, (uint8_t)(257 - count));
        raw_write(os, prev);
    }
    raw_write(os, (uint8_t)128);
}

void packbits_decompress(std::ifstream& is, std::ofstream& os) {
}

void packbits(std::ifstream& is, std::ofstream& os, const std::string& action) {
    using namespace std;
    switch (action[0]) {
        case 'c':
            packbits_compress(is, os);
            break;
        case 'd':
            packbits_decompress(is, os);
            break;
        default:
            cout << "Invalid action: " << action << endl;
            exit(EXIT_FAILURE);
            break;
    }
}

int main(int argc, char** argv) {
    using namespace std;
    if (argc != 4) {
        cout << "Usage: packbits [c|d] <input file> <output file>\n";
        exit(EXIT_FAILURE);
    }
    string action = argv[1];
    string in_file = argv[2];
    string out_file = argv[3];
    ifstream is(in_file, ios::binary);
    if (!is) {
        cout << "Unable to open file " << in_file << endl;
        exit(EXIT_FAILURE);
    }
    ofstream os(out_file, ios::binary);
    if (!os) {
        cout << "Unable to open file " << out_file << endl;
        exit(EXIT_FAILURE);
    }
    packbits(is, os, action);
}