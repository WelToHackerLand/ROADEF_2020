#ifndef MCTS_SOLUTION
#define MCTS_SOLUTION

#include "../utils.cpp"
#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"
#include "NLS_local_search.cpp"

struct Node {
    int id;
    int numVisit;
    Node* parent;
    double score;
    double uct_score;
    vector<Node*> children;
    vector<int> moves;
    vector<int> untried_moves;
    vector<int> best_setting;

    Node(): id(-1), parent(nullptr){
		score = 0;
    }
    Node(Node* parent, int id, vector<int>& moves): id(id), parent(parent), untried_moves(moves), moves(moves){
		numVisit = 0;
		score = 0;
    }

    bool has_untried_moves(){
		return !untried_moves.empty();
    }

    bool isLeaf(){
		return children.empty() || has_untried_moves();
    }

    Node* select_childNode(){
		// best uct node
		if ( children.empty() ) return nullptr;

		for (auto child: children) {
			child->uct_score = (double)child->score / (double)child->numVisit + 4*sqrt(log((double)this->numVisit) / child->numVisit);
		}

		return *max_element(children.begin(), children.end(),[](Node* a, Node* b) { return a->uct_score < b->uct_score; });
    }

    Node* add_child(int move){
		Utils::remove(untried_moves, move);
		vector<int> child_moves = moves;
		Utils::remove(child_moves, move);
		Node* child = new Node(this, move, child_moves);
		children.push_back(child);
		return child;
    }

    int get_untried_move(){ // randomly return a untouched moves
		return untried_moves[ Utils::integer_random_generator(0 , untried_moves.size()) ];
    }

    void backpropagate(double reward){
		score += reward;
		numVisit++;
		if (parent == nullptr) return;
		parent->backpropagate(reward);
    }
};

struct MCTS
{
    Problem_Instance* instance;
    Node* root;
    double bestObj = 1e10;
    NLS_object best_solution;

    MCTS(Problem_Instance* instance, string outputFile, double timeLimit ): instance(instance) {
		int n = instance->numInterventions;

		// init root node
		vector<int> untried_moves;
		for (int i = 1; i <= n; ++i)
			untried_moves.push_back(i);
		root = new Node(nullptr, -1, untried_moves);

		// prepare obj1 insertion cost
		instance->prepare_obj1_cost();
    }

    double evaluate(bool feasible, NLS_object& solution){
		double score = solution.get_OBJ(*instance);
		if (!feasible) return 0;
		else if (score < bestObj) return 1;
		else if (score <= 1.3*bestObj) return (1.3*bestObj/score - 1) * (1.3*bestObj/score - 1);
		return 0;
    }

    void process() {
		int iter = 0;
		while ( iter < 100000 ){
			Node* node = root;
			bool feasible = true;
			NLS_object solution;
			solution.Initialize(*instance);

			// 1. SELECT. start at root, select a path through the tree to a LEAF node using UCT on all fully expanded nodes
			while (!node->isLeaf()){
				node = node->select_childNode();
				/// >>> make a move as node is selected
				feasible = solution.Randomized_Insert(*instance, node->id);
			}

			// 2. EXPAND. by adding A SINGLE CHILD (if not terminal or not fully expanded/has any untried move)
			if (node->has_untried_moves()){
				int move = node->get_untried_move();
				node = node->add_child(move);
				feasible = solution.Randomized_Insert(*instance, node->id);
			}

			// 3. SIMULATE (if not terminal)
			if (feasible && !node->moves.empty()){
				Utils::shuffle(node->moves);
				for (auto move : node->moves) {
					feasible = solution.Randomized_Insert(*instance, move);
					if (!feasible) break;
				}
			}

			// 5. EVALUATE & BACKPROPAGATION as reaching final state
			double reward = evaluate(feasible, solution);
			node->backpropagate(reward);

			if (solution.get_OBJ(*instance) < bestObj && feasible){
				bestObj = solution.get_OBJ(*instance);
				cerr << bestObj << "\n";
				best_solution = solution;
			}

			++iter;
		}
    }
};

#endif // MCTS_SOLUTION