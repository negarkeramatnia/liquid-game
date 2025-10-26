#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <iomanip>
using namespace std;

struct Layer {
    char color;
    double amount;
};

//"Container" is a stack of Layers
using Container = stack<Layer>;

//"State" is a vector holding all 7 containers
using State = vector<Container>;

double MAX_CAPACITY = 0.0;
State read_file(const string& filename) {
    State initial_state(7);

    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return initial_state;
    }

    string line;

    if (getline(file, line)) {
        MAX_CAPACITY = stod(line);//convert the first line string to float
    }

    int container_index = 0;
    while (getline(file, line) && container_index < 7) {
        stringstream ss(line);
        string chunk;

        while (getline(ss, chunk, '-')) {
            stringstream chunk_ss(chunk);
            char color;
            double amount;

            if (chunk_ss >> color >> amount) {
                Layer new_layer;
                new_layer.color = color;
                new_layer.amount = amount;

                initial_state[container_index].push(new_layer);
            }
        }
        container_index++;
    }

    file.close();
    return initial_state;
}

void print_state(const State& state) {
    cout << "--- Game State ---" << endl;
    cout << "Max Capacity: " << MAX_CAPACITY << "\n" << endl;

    cout << fixed << setprecision(1);

    for (int i = 0; i < state.size(); i++) {
        cout << "Container " << (i + 1) << ": (Top to Bottom)" << endl;

        if (state[i].empty()) {
            cout << "  [Empty]" << endl;
            continue;
        }

        Container temp_stack = state[i];

        while (!temp_stack.empty()) {
            Layer layer = temp_stack.top();
            temp_stack.pop();

            cout << "  [" << layer.color << " | " << layer.amount << "]" << endl;
        }
        cout << endl;
    }
    cout << "--------------------------" << endl;
}

int main() {
    State initial = read_file("D:\\Main\\ai-projects\\test1.txt");
    print_state(initial);
}