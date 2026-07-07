#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        lits = std::vector<bool>();
        (*lits).reserve(numVars);
        //immediately learn what we can about the equation
        const std::optional<std::vector<int>>& unit_prop = learn();

        if (unit_prop.has_value()) {
            const std::vector<int>& learned_lits = *unit_prop;
            for (size_t i = 0; i < learned_lits.size(); i++) {
                learned.insert(learned_lits[i]);
            }
        } else {
            return false;
        }

        //eliminate all pure literals, if any
        const std::vector<int>& pure = pure_lit_eliminate();
        for (size_t i = 0; i < pure.size(); i++) {
            learned.insert(pure[i]);
        }

        //find which clauses are satisfied
        mark_satisfied();

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
                // const std::vector<int>& lit_assignments = learn();

                const std::optional<std::vector<int>>& unit_prop = learn();

                //cur_lit = true led to contradiction
                if (!unit_prop.has_value()) {
                    //erase cur_lit's effect on learned and assignment
                    learned.erase(cur_lit);
                    assignment.pop_back();
                    //add in -cur_lit and continue
                    learned.insert(-cur_lit);
                    assignment.push_back(false);

                    const std::optional<std::vector<int>>& new_unit_prop = learn();
                    
                    //-cur_lit also led to a contradiction => erase -cur_lit's effect
                    // and return false
                    if (!new_unit_prop.has_value()) {
                        learned.erase(-cur_lit);
                        assignment.pop_back();
                        return false;
                    } 
                    //-cur_lit did not lead to a contradiction
                    else {
                        //learn what -cur_lit implies
                        const std::vector<int>& learned_lits = *new_unit_prop;
                        for (size_t i = 0; i < learned_lits.size(); i++) {
                            learned.insert(learned_lits[i]);
                        }

                        //pure literal elimination afterwards
                        const std::vector<int>& pure = pure_lit_eliminate();
                        for (size_t i = 0; i < pure.size(); i++) {
                            learned.insert(pure[i]);
                        }

                        //find which clauses are satisfied and mark them as satisfied
                        const std::vector<size_t>& indices = mark_satisfied();

                        //if -cur_lit led to a satisfiable assignment, return true
                        if (solve(assignment)) {
                            return true;
                        } 
                        //if -cur_lit did not lead to a solution, unlearn everything in regards to it
                        else {
                            learned.erase(-cur_lit);
                            assignment.pop_back();

                            for (size_t i = 0; i < learned_lits.size(); i++) {
                                learned.erase(learned_lits[i]);
                            }

                            for (size_t i = 0; i < pure.size(); i++) {
                                learned.erase(pure[i]);
                            }

                            for (size_t i = 0; i < indices.size(); i++) {
                                satisfied[indices[i]] = false;
                            }

                            return false;
                        }
                    }

                } 
                //cur_lit did not lead to a contradiction, learn it
                else {
                    const std::vector<int>& learned_lits = *unit_prop;
                    for (size_t i = 0; i < learned_lits.size(); i++) {
                        learned.insert(learned_lits[i]);
                    }

                    //pure literal elimination afterwards
                    const std::vector<int>& pure = pure_lit_eliminate();
                    for (size_t i = 0; i < pure.size(); i++) {
                        learned.insert(pure[i]);
                    }

                    //find which clauses are satisfied and mark them as satisfied
                    const std::vector<size_t>& indices = mark_satisfied();

                    if (solve(assignment)) {
                        return true;
                    } else { //cur_lit did not lead to a solution
                        //remove and switch the value of the current literal assignment
                        assignment.pop_back();
                        assignment.push_back(false);

                        //erase the learning of cur_lit as true
                        learned.erase(cur_lit);

                        //remove everything we learned from cur_lit
                        for (size_t i = 0; i < learned_lits.size(); i++) {
                            learned.erase(learned_lits[i]);
                        }

                        for (size_t i = 0; i < pure.size(); i++) {
                            learned.erase(pure[i]);
                        }

                        //un-mark the newly satisfied clauses as unsatisfied
                        for (size_t i = 0; i < indices.size(); i++) {
                            satisfied[indices[i]] = false;
                        }

                        //now assume the current literal is false
                        learned.insert(-cur_lit);

                        //relearn everything under the assumption that cur_lit = false
                        const std::optional<std::vector<int>>& new_unit_prop = learn();

                        //-cur_lit led to a contradiction
                        if (!new_unit_prop.has_value()) {
                            learned.erase(-cur_lit);
                            assignment.pop_back();

                            return false;
                        } else {
                            const std::vector<int>& new_learned_lits = *new_unit_prop;
                            for (size_t i = 0; i < new_learned_lits.size(); i++) {
                                learned.insert(new_learned_lits[i]);
                            }

                            const std::vector<int>& new_pure = pure_lit_eliminate();
                            for (size_t i = 0; i < new_pure.size(); i++) {
                                learned.insert(new_pure[i]);
                            }

                            //recheck every clause that is now satisfied
                            const std::vector<size_t>& new_indices = mark_satisfied();

                            if (solve(assignment)) { //now solve again
                                return true;
                            } else {
                                //if that fails, remove cur_lit's assignment
                                learned.erase(-cur_lit);

                                assignment.pop_back();

                                //unlearn everything we learned from cur_lit
                                for (size_t i = 0; i < new_learned_lits.size(); i++) {
                                    learned.erase(new_learned_lits[i]);
                                }

                                for (size_t i = 0; i < new_pure.size(); i++) {
                                    learned.erase(new_pure[i]);
                                }

                                //un-mark the newly satisfied clauses as unsatisfied
                                for (size_t i = 0; i < new_indices.size(); i++) {
                                    satisfied[new_indices[i]] = false;
                                }

                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
}