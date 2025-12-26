#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include <iomanip>
using namespace std;

struct Layer {
    char color;
    double amount;
};

struct State {
    double max_capacity;
    Layer tubes[7][10];
    int num_layers[7];
};

struct Node {
    State state;
    Node* parent;
    int from_tube;
    int to_tube;
    double total_displacement;//g(n)
    double final_cost;//f(n) = g(n) + h(n)
    Node* next;
};

Node* allocated_head = nullptr;
unordered_map<string, double> cost;
double epsilon = 1e-9;

struct Compare_A_star {
    bool operator()(Node* a, Node* b) {
        return a->final_cost > b->final_cost;
    }
};

struct Compare_Dijkstra {
    bool operator()(Node* a, Node* b) {
        return a->total_displacement > b->total_displacement;
    }
};

bool isEqual(double a, double b) {
    return fabs(a - b) <= epsilon;
}

double my_min(double a, double b) {
    if (a < b) return a;
    else return b;
}

double get_tube_total(State s, int tube_index) {
    double total = 0;
    for (int i = 0; i < s.num_layers[tube_index]; i++)
        total += s.tubes[tube_index][i].amount;
    return total;
}

double heuristic(State s) {
    double total_misplaced_cost = 0;
    for (int i = 0; i < 7; i++) {
        double max_amount_in_tube = 0.0;
        for (int j = 0; j < s.num_layers[i]; j++) {
            if (s.tubes[i][j].amount > max_amount_in_tube) {
                max_amount_in_tube = s.tubes[i][j].amount;
            }
        }
        total_misplaced_cost += (get_tube_total(s, i)//total liquid in one tube
            - max_amount_in_tube);//amount of the largest layer
    }
    return total_misplaced_cost;
}

string stateToString(State s) {
    stringstream ss;
    string tube_strings[7];
    for (int i = 0; i < 7; i++) {
        stringstream tube_ss;

        for (int j = 0; j < s.num_layers[i]; j++) {
            double amt = s.tubes[i][j].amount;
            if (amt <= 0.0) continue;
            tube_ss << s.tubes[i][j].color << amt << ",";
        }
        tube_strings[i] = tube_ss.str();
    }
    for (int i = 0; i < 7; i++)
        ss << tube_strings[i] << "|";
    return ss.str();
}

Node* createNode(State& state, Node* parent, int from, int to, double g_cost, double f_cost) {
    Node* newNode = new Node{ state, parent, from, to, g_cost, f_cost, nullptr };
    newNode->next = allocated_head;
    allocated_head = newNode;
    return newNode;
}

void printSolutionPath(Node* s) {
    if (s == nullptr || s->parent == nullptr) return;
    printSolutionPath(s->parent);
    int max_h = s->state.max_capacity;
    cout << "\n";
    for (int h = max_h; h >= 1; h--) {
        for (int t = 0; t < 7; t++) {
            double current_total_fill = get_tube_total(s->state, t);
            if (h > current_total_fill)
                cout << "| | ";
            else {
                double height_so_far = 0.0;
                char color_at_this_height;

                for (int j = 0; j < s->state.num_layers[t]; j++) {
                    height_so_far += s->state.tubes[t][j].amount;

                    if (h <= height_so_far) {
                        color_at_this_height = s->state.tubes[t][j].color;
                        break;
                    }
                }
                cout << "|" << color_at_this_height << "| ";
            }
        }
        cout << endl;
    }
    for (int t = 0; t < 7; t++) {
        cout << "--- ";
    } cout << endl;
    for (int t = 0; t < 7; t++) {
        cout << " " << t + 1 << "  ";
    } cout << endl;

    for (int t = 0; t < 7; t++) {
        double total = get_tube_total(s->state, t);
        cout << total << "  ";
    } cout << endl;
}

bool isGoal(State s) {
    string seen_colors = ""; //for store all the pure colors that is  found

    for (int i = 0; i < 7; i++) {
        if (s.num_layers[i] == 0) continue;

        //if the tube is pure?
        char color = s.tubes[i][0].color;
        for (int j = 1; j < s.num_layers[i]; j++) {
            if (s.tubes[i][j].color != color)
                return false;
        }

        // Loop on seen_colors to check if we've seen it before
        for (int k = 0; k < seen_colors.length(); k++) {
            if (seen_colors[k] == color)
                return false;
        }
        //new pure color
        seen_colors += color;
    } return true;
}
//bool isGoal(State s) {
//    int amount_tubes = 0;
//    for (int i = 0; i < 7; i++) {
//        if (s.num_layers[i] == 0) {
//            amount_tubes++;
//        }
//        else if (s.num_layers[i] == 1) {
//            if (isEqual(s.tubes[i][0].amount, s.max_capacity))
//                amount_tubes++;
//        }
//    } return amount_tubes == 7;
//}

void pour(State& s, int from_tube, int to_tube, double amount) {
    int fromTopIndex = s.num_layers[from_tube] - 1;
    char color = s.tubes[from_tube][fromTopIndex].color;

    s.tubes[from_tube][fromTopIndex].amount -= amount;//from the source tube's top layer
    if (isEqual(s.tubes[from_tube][fromTopIndex].amount, 0.0))//if that layer get empty
        s.num_layers[from_tube]--;

    int toTopIndex = s.num_layers[to_tube] - 1;

    if (s.num_layers[to_tube] > 0 && s.tubes[to_tube][toTopIndex].color == color)//if there colores are similer
        s.tubes[to_tube][toTopIndex].amount += amount;
    else { //if there is differente colors or the tube is empty
        int toNewIndex = s.num_layers[to_tube];
        s.tubes[to_tube][toNewIndex].color = color;
        s.tubes[to_tube][toNewIndex].amount = amount;
        s.num_layers[to_tube]++;
    }
}

State organize_state(State s) {
    State clean_state;
    clean_state.max_capacity = s.max_capacity;
    for (int i = 0; i < 7; i++) {
        clean_state.num_layers[i] = 0;
    }
    int target_index = 0;

    //copy only the full tubes
    for (int i = 0; i < 7; i++) {
        if (s.num_layers[i] > 0) {
            clean_state.tubes[target_index][0].color = s.tubes[i][0].color;
            clean_state.tubes[target_index][0].amount = s.tubes[i][0].amount;
            clean_state.num_layers[target_index] = 1;
            target_index++;
        }
    }
    return clean_state;
}

void print_state(State s) {
    int max_h = s.max_capacity;
    cout << "\n";
    for (int h = max_h; h >= 1; h--) {
        for (int t = 0; t < 7; t++) {
            double current_total_fill = get_tube_total(s, t);
            if (h > current_total_fill)
                cout << "| | ";
            else {
                double height_so_far = 0.0;
                char color_at_this_height;

                for (int j = 0; j < s.num_layers[t]; j++) {
                    height_so_far += s.tubes[t][j].amount;

                    if (h <= height_so_far) {
                        color_at_this_height = s.tubes[t][j].color;
                        break;
                    }
                }
                cout << "|" << color_at_this_height << "| ";
            }
        }
        cout << endl;
    }
    for (int t = 0; t < 7; t++) {
        cout << "--- ";
    } cout << endl;
    for (int t = 0; t < 7; t++) {
        cout << " " << t + 1 << "  ";
    } cout << endl;

    for (int t = 0; t < 7; t++) {
        double total = get_tube_total(s, t);
        if (total >= 10.0)
            cout << total << "  ";
        else
            cout << total << "  ";
    }
}

void cleanup_memory() {
    Node* curr = allocated_head;
    while (curr) {
        Node* temp = curr;
        curr = curr->next;
        delete temp;
    }
    allocated_head = nullptr;
}

void solve_dijkstra(State start) {
    priority_queue<Node*, vector<Node*>, Compare_Dijkstra> frontier;
    string start_state = stateToString(start);
    cost[start_state] = 0.0;
    Node* startNode = createNode(start, nullptr, -1, -1, 0.0, 0.0);
    frontier.push(startNode);
    Node* goalNode = nullptr;
    int nodes_explored = 0;

    while (!frontier.empty()) {
        Node* curr_node = frontier.top();//lowest g(n)
        frontier.pop();//it's now been exploreds so pop it from frontier
        nodes_explored++;

        if (isGoal(curr_node->state)) {
            goalNode = curr_node;
            break;
        }

        for (int from = 0; from < 7; from++) {//tubes as source
            if (curr_node->state.num_layers[from] == 0) continue;//skip empty tube

            int top_layer_index = curr_node->state.num_layers[from] - 1;
            double src_Amount = curr_node->state.tubes[from][top_layer_index].amount;

            for (int to = 0; to < 7; to++) {//tubes as destination
                if (from == to) continue;

                double dst_Free = curr_node->state.max_capacity - get_tube_total(curr_node->state, to);
                if (isEqual(dst_Free, 0.0) || dst_Free < 0) continue;
                double amount = my_min(src_Amount, dst_Free);
                if (isEqual(amount, 0.0) || amount <= 0) continue;

                State newState = curr_node->state;//make a copy of the current state
                pour(newState, from, to, amount);

                double new_g_cost = curr_node->total_displacement + amount;
                string newStateString = stateToString(newState);

                if (cost.find(newStateString) == cost.end() //is it duplicate?
                    || new_g_cost < cost[newStateString]) {//is it cheaper?
                    //it's a new state or a better state
                    cost[newStateString] = new_g_cost;//update map with new cheaper cost
                    double new_f_cost = new_g_cost; // h(n) = 0

                    Node* newNode = createNode(newState, curr_node, from, to, new_g_cost, new_f_cost);

                    frontier.push(newNode);
                }
            }
        }
    }

    if (goalNode) {
        cout << "frontier size = " << frontier.size() << endl;
        cout << "explored size = " << cost.size() << endl;

        cout << "\nSolution found with total displacement cost: " << goalNode->total_displacement << "\n";
        cout << "Nodes explored: " << nodes_explored << "\n";
        cout << "\n--- Solution Path ---" << endl;
        printSolutionPath(goalNode);
        cout << "\n--- Final State sorted ---" << endl;
        State organizedState = organize_state(goalNode->state);
        print_state(organizedState);
    }
    else cout << "No solution found after exploring " << nodes_explored << " nodes.\n";
    cleanup_memory();
}

void solve_a_star(State start) {
    priority_queue<Node*, vector<Node*>, Compare_A_star> frontier;
    string start_state = stateToString(start);
    cost[start_state] = 0.0;
    double h_start = heuristic(start);
    Node* startNode = createNode(start, nullptr, -1, -1, 0.0, h_start);
    frontier.push(startNode);
    Node* goalNode = nullptr;
    int nodes_explored = 0;

    while (!frontier.empty()) {
        Node* curr_node = frontier.top();
        frontier.pop();
        nodes_explored++;

        if (isGoal(curr_node->state)) {
            goalNode = curr_node;
            break;
        }

        for (int from = 0; from < 7; from++) {
            if (curr_node->state.num_layers[from] == 0) continue;

            int top_layer_index = curr_node->state.num_layers[from] - 1;
            double src_Amount = curr_node->state.tubes[from][top_layer_index].amount;

            for (int to = 0; to < 7; to++) {
                if (from == to) continue;

                double dst_Free = curr_node->state.max_capacity - get_tube_total(curr_node->state, to);
                if (isEqual(dst_Free, 0.0) || dst_Free < 0) continue;
                double amount = my_min(src_Amount, dst_Free);
                if (isEqual(amount, 0.0) || amount <= 0) continue;

                State newState = curr_node->state;
                pour(newState, from, to, amount);

                double new_g_cost = curr_node->total_displacement + amount;
                string newStateString = stateToString(newState);

                if (cost.find(newStateString) == cost.end() || new_g_cost < cost[newStateString]) {
                    cost[newStateString] = new_g_cost;
                    double new_h_cost = heuristic(newState);
                    double new_f_cost = new_g_cost + new_h_cost;

                    Node* newNode = createNode(newState, curr_node, from, to, new_g_cost, new_f_cost);

                    frontier.push(newNode);
                }
            }
        }
    }

    if (goalNode) {
        cout << "frontier size = " << frontier.size() << endl;
        cout << "explored size = " << cost.size() << endl;

        cout << "\nSolution found with total displacement cost: " << goalNode->total_displacement << "\n";
        cout << "Nodes explored: " << nodes_explored << "\n";
        cout << "\n--- Solution Path ---" << endl;
        printSolutionPath(goalNode);
        cout << "\n--- Final State sorted ---" << endl;
        State organizedState = organize_state(goalNode->state);
        print_state(organizedState);
    }
    else cout << "No solution found after exploring " << nodes_explored << " nodes.\n";
    cleanup_memory();
}

int main() {
    ifstream file("D:\\Main\\ai-projects\\test2.txt");
    if (!file) {
        cout << "Error open file";
        return 1;
    }

    State state;
    string line;
    getline(file, line);
    state.max_capacity = stod(line);

    for (int i = 0; i < 7; i++) {
        state.num_layers[i] = 0;
        string line2;
        getline(file, line2);

        stringstream tube_line(line2);
        string peace;
        while (getline(tube_line, peace, '-')) {
            stringstream ss(peace);
            string color_str;
            double amount_double;
            if (ss >> color_str >> amount_double) {
                state.tubes[i][state.num_layers[i]].color = color_str[0];
                state.tubes[i][state.num_layers[i]].amount = amount_double;
                state.num_layers[i]++;
            }
        }
    }
    file.close();

    cout << "--- Initial State ---" << endl;
    print_state(state);

    cout << "\nWhich method do you want to use?\n";
    cout << "1. Unconscious Method (Dijkstra's Algorithm)\n";
    cout << "2. Conscious Method (A* Search)\n";

    int choice;
    while (true) {
        cout << "Enter 1 or 2: ";
        cin >> choice;
        if (choice == 1) {
            cout << "\nRunning 'Unconscious Method' (Dijkstra)...\n";
            auto startTime = chrono::high_resolution_clock::now();
            solve_dijkstra(state);
            auto endTime = chrono::high_resolution_clock::now();
            double duration = chrono::duration<double>(endTime - startTime).count();
            cout << "\nExecution time: " << setprecision(4) << duration << " seconds\n";
            break;
        }
        else if (choice == 2) {
            cout << "\nRunning 'Conscious Method' (A* Search)...\n";
            auto startTime = chrono::high_resolution_clock::now();
            solve_a_star(state);
            auto endTime = chrono::high_resolution_clock::now();
            double duration = chrono::duration<double>(endTime - startTime).count();
            cout << "\nExecution time: " << setprecision(4) << duration << " seconds\n";
            break;
        }
        else cout << "Invalid choice.\n";
    }
}