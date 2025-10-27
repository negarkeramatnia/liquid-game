#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <iomanip>
#include <limits>
#include <chrono>
using namespace std;

struct Layer {
    char color;
    double amount;
};

using container = stack<Layer>; //is a stack of Layers
using State = vector<container>; //is a vector holding all 7 containers

struct Move {
    State state;
    double cost;
};

struct DijkstraNode {
    double g_cost;
    State state;
    vector<State> path;
};

priority_queue<DijkstraNode> frontier;

double MAX_CAPACITY = 0.0;

bool operator<(const DijkstraNode& lhs, const DijkstraNode& rhs) {
    return lhs.g_cost > rhs.g_cost;
}

State read_file(const string& filename) {
    State initial_state(7);
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open file '" << filename << "'" << endl;
        return initial_state;
    }

    string line;

    if (getline(file, line)) {
        MAX_CAPACITY = stod(line); //convert the first line string to float
    }

    int container_index = 0;
    while (getline(file, line) && container_index < 6) {
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
        cout << "container " << (i + 1) << ": (Top to Bottom)" << endl;

        if (state[i].empty()) {
            cout << "  [Empty]" << endl;
            continue;
        }

        container temp_stack = state[i];

        // We need to reverse the stack to print bottom-to-top or just print in popped order (top-to-bottom)
        while (!temp_stack.empty()) {
            Layer layer = temp_stack.top();
            temp_stack.pop();
            cout << "  [" << layer.color << " | " << layer.amount << "]" << endl;
        }
        cout << endl;
    }
    cout << "--------------------------" << endl;
}

string state_to_string(const State& state) {
    stringstream ss;
    ss << fixed << setprecision(1);
    for (int i = 0; i < state.size(); i++) {
        container temp = state[i];
        stack<Layer> reverse_stack;
        while (!temp.empty()) {
            reverse_stack.push(temp.top());
            temp.pop();
        }
        while (!reverse_stack.empty()) {
            Layer l = reverse_stack.top();
            reverse_stack.pop();
            ss << l.color << l.amount << "-";
        }
        ss << "|"; // Separator for containers
    }
    return ss.str();
}

bool is_goal(const State& state) {
    int solved_containers = 0;
    for (int i = 0; i < state.size(); i++) {
        if (state[i].empty()) {
            solved_containers++;
            continue;
        }
        char target_color = state[i].top().color;
        bool is_monochromatic = true;
        container temp = state[i];
        while (!temp.empty()) {
            if (temp.top().color != target_color) {
                is_monochromatic = false;
                break;
            }
            temp.pop();
        }
        if (is_monochromatic) {
            solved_containers++;
        }
    }
    return (solved_containers == 7);
}

void print_state_line(State& state) {
    cout << fixed << setprecision(1);
    for (int i = 0; i < state.size(); i++) {
        cout << "C" << (i + 1) << ":";
        if (state[i].empty()) {
            cout << "[Empty] ";
            continue;
        }
        container temp = state[i];
        string s = "";
        while (!temp.empty()) {
            Layer l = temp.top();
            temp.pop();
            // Shorten print: R|5.0
            s = "[" + string(1, l.color) + "|" + to_string(l.amount).substr(0, 3) + "]" + s;
        }
        cout << s << " ";
    }
    cout << endl;
}

double get_container_sum(const container& c) {
    container temp = c;
    double sum = 0.0;
    while (!temp.empty()) {
        sum += temp.top().amount;
        temp.pop();
    }
    return sum;
}

vector<Move> get_successors(const State& current_state) {
    vector<Move> successors;
    for (int i = 0; i < 7; i++) { // Source 'i'
        if (current_state[i].empty()) continue;
        Layer top_layer = current_state[i].top();
        double move_cost = top_layer.amount;

        for (int j = 0; j < 7; j++) { // Destination 'j'
            if (i == j) continue;

            double dest_fill = get_container_sum(current_state[j]);
            if (dest_fill + move_cost <= MAX_CAPACITY) {
                State new_state = current_state;
                new_state[i].pop();
                new_state[j].push(top_layer);

                Move new_move;
                new_move.state = new_state;
                new_move.cost = move_cost;
                successors.push_back(new_move);
            }
        }
    }
    return successors;
}

void solve_dijkstra(State initial_state) {
    map<string, double> visited_g_costs;

    DijkstraNode start_node;
    start_node.g_cost = 0.0;
    start_node.state = initial_state;
    start_node.path.push_back(initial_state);

    frontier.push(start_node);
    visited_g_costs[state_to_string(initial_state)] = 0.0;

    cout << "Solving with Dijkstra's (Unconscious Method)..." << endl;
    int states_explored = 0;

    while (!frontier.empty()) {
        DijkstraNode current = frontier.top();
        frontier.pop();
        states_explored++;

        string current_key = state_to_string(current.state);

        //skips useless paths
        if (current.g_cost > visited_g_costs[current_key]) continue;

        if (is_goal(current.state)) {
            cout << "\n--- DIJKSTRA'S SOLUTION FOUND! ---" << endl;
            cout << "Total Fluid Displacement: " << current.g_cost << endl;
            cout << "Total Steps: " << current.path.size() - 1 << endl;
            cout << "States Explored: " << states_explored << "\n" << endl;

            for (int i = 0; i < current.path.size(); i++) {
                print_state_line(current.path[i]);
            }
            return;
        }

        vector<Move> successors = get_successors(current.state);
        for (int i = 0; i < successors.size(); i++) {
            State next_state = successors[i].state;
            double move_cost = successors[i].cost;

            double new_g_cost = current.g_cost + move_cost;
            string next_key = state_to_string(next_state);

            if (visited_g_costs.find(next_key) == visited_g_costs.end() ||
                new_g_cost < visited_g_costs[next_key]) {
                visited_g_costs[next_key] = new_g_cost;
                DijkstraNode next_node;
                next_node.g_cost = new_g_cost;
                next_node.state = next_state;
                next_node.path = current.path;
                next_node.path.push_back(next_state);
                frontier.push(next_node);
            }
        }
    }
    cout << "--- No Solution Found ---" << endl;
}

void solve_a_star(State initial_state) {
    cout << "A* search is not implemented yet." << endl;
}

int main() {
    State initial = read_file("D:\\Main\\ai-projects\\test1.txt");
    cout << "--- Initial State ---" << endl;
    print_state(initial);
    cout << "Which method do you want to use?" << endl;
    cout << "1. Unconscious Method (Dijkstra's Algorithm)" << endl;
    cout << "2. Conscious Method (A* Search)" << endl;
    cout << "Enter 1 or 2: ";

    int choice;
    while (true) {
        cout << "Enter 1 or 2: ";
        cin >> choice;

        if (choice == 1) {
            system("cls");
            auto startTime = chrono::high_resolution_clock::now();
            solve_dijkstra(initial);
            auto endTime = chrono::high_resolution_clock::now();
            double duration = chrono::duration<double>(endTime - startTime).count();
            cout << "Execution time: " << duration << " seconds\n";
            break;
        }
        else if (choice == 2) {
            system("cls");
            auto startTime = chrono::high_resolution_clock::now();
            solve_a_star(initial);
            auto endTime = chrono::high_resolution_clock::now();
            double duration = chrono::duration<double>(endTime - startTime).count();
            cout << "Execution time: " << duration << " seconds\n";
            break;
        }
        else {
            cout << "Invalid choice. Please enter 1 or 2." << endl;
        }
    }
}