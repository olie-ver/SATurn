#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        lits = std::vector<bool>();
        (*lits).reserve(numVars);
        return solve(*lits);
    }

    bool SATSolver::solve(std::vector<bool>& assignment) {
        //once we've assigned all of our variables, evaluate the cnf equation
        if (assignment.size() == numVars) {
            // for (bool b : assignment) {
            //     std::cout << b;
            // }
            // std::cout << '\n';
            bool solved = true;
            for (size_t i = 0; i < clauses.size(); i++) {
                bool true_clause = false;
                //evaluate whether or not the current clause evaluates to true
                for (size_t j = 0; j < clauses[i].size(); j++) {
                    int val = clauses[i][j];
                    int idx = std::abs(val) - 1;
                    if (val < 0) {
                        true_clause |= !assignment[idx];
                    } else {
                        true_clause |= assignment[idx];
                    }
                }
                //check if the whole equation evaluates to true
                solved &= true_clause;
            }
            return solved;
        } else {
            //otherwise, assign this variable true, then recurse
            assignment.push_back(true);
            //if we solve(assignment) returns true, we've found a solution, so return true
            if (solve(assignment)) {
                return true;
            } else {
                //otherwise, switch the current variable's assignment to be opposite of what it was
                assignment.pop_back();
                assignment.push_back(false);
                if (solve(assignment)) {
                    return true;
                } else {
                    assignment.pop_back();
                    return false;
                }
            }
        }
    }
}