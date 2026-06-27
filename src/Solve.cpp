#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        lits = std::vector<bool>();
        (*lits).reserve(numVars);
        //immediately learn what we can about the equation
        learn();

        // propagate();

        return solve(*lits);
    }

    bool SATSolver::solve(std::vector<bool>& assignment) {
        //once we've assigned all of our variables, evaluate the cnf equation
        if (assignment.size() == numVars) {
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
            //size + 1 = index of current literal to be assigned
            int cur_lit = assignment.size() + 1;
            if (learned.contains(cur_lit)) { //needs to remove the current assignment
                //if we learned cur_lit => cur_lit = true
                assignment.push_back(true);

                if (solve(assignment)) {
                    return true;
                } else {
                    assignment.pop_back();
                    return false;
                }

                // return solve(assignment); //move onto the next variable
            } else if (learned.contains(-cur_lit)) {
                //if we learned -cur_lit => cur_lit = false
                assignment.push_back(false); //move onto the next variable

                if (solve(assignment)) {
                    return true;
                } else {
                    assignment.pop_back();
                    return false;
                }
            } else {
                //assume the current literal is true
                learned.insert(cur_lit);
                assignment.push_back(true);

                //learn and propagate
                const std::vector<int>& lit_assignments = learn();
                if (solve(assignment)) { //solve more
                    return true;
                } else {
                    //remove and switch the value of the current literal assignment
                    assignment.pop_back();
                    assignment.push_back(false);

                    //erase the learning of cur_lit as true
                    learned.erase(cur_lit);

                    //remove everything we learned from cur_lit
                    for (size_t i = 0; i < lit_assignments.size(); i++) {
                        learned.erase(lit_assignments[i]);
                    }

                    //now assume the current literal is false
                    learned.insert(-cur_lit);

                    //relearn everything under the assumption that cur_lit = false
                    const std::vector<int>& new_lit_assignments = learn();
                    // const std::vector<size_t>& new_indices = propagate();
                    if (solve(assignment)) { //now solve again
                        return true;
                    } else {
                        //if that fails, remove cur_lit's assignment
                        learned.erase(-cur_lit);

                        assignment.pop_back();

                        //unlearn everything we learned from cur_lit
                        for (size_t i = 0; i < new_lit_assignments.size(); i++) {
                            learned.erase(new_lit_assignments[i]);
                        }

                        return false;
                    }
                }
            }
        }
    }
}