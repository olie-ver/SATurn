#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        try_solved = true;
        lits.reserve(numVars);

        //immediately learn what we can about the equation
        const std::optional<learn_pair>& unit_prop = learn();

        if (!unit_prop.has_value()) {
            return false;
        }

        pure_lit_eliminate();

        //find which clauses are satisfied
        const std::optional<std::vector<size_t>>& satisfied = mark_satisfied();

        //if mark_satisfied() returns nothing, that means that we've solved every clause
        if (!satisfied.has_value()) {
            return true;
        }

        return solve(lits);
    }

    bool SATSolver::solve(std::vector<bool>& assignment) {
        //once we've assigned all of our variables, evaluate the cnf equation
        if (assignment.size() == numVars) {
            bool solved = true;
            for (size_t i = 0; i < clauses.size(); i++) {
                if (satisfied[i]) {
                    continue;
                }

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
                const std::optional<learn_pair>& unit_prop = learn();

                //cur_lit = true led to contradiction
                if (!unit_prop.has_value()) { //no need to undo anything because learn() handles it for us in this case
                    //erase cur_lit's effect on learned and assignment
                    learned.erase(cur_lit);
                    assignment.pop_back();

                    //add in -cur_lit and continue
                    learned.insert(-cur_lit);
                    assignment.push_back(false);

                    const std::optional<learn_pair>& new_unit_prop = learn();
                    
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
                        const learn_pair& pure = pure_lit_eliminate();

                        //find which clauses are satisfied and mark them as satisfied
                        const std::optional<std::vector<size_t>>& indices = mark_satisfied();

                        //if all clauses are satisfied
                        if (!indices.has_value()) {
                            return true;
                        }

                        //if -cur_lit led to a satisfiable assignment, return true
                        if (solve(assignment)) {
                            return true;
                        } 
                        //if -cur_lit did not lead to a solution, unlearn everything in regards to it
                        else {
                            learned.erase(-cur_lit);
                            assignment.pop_back();

                            const auto& [units, unit_indices] = *new_unit_prop;
                            for (size_t i = 0; i < units.size(); i++) {
                                learned.erase(units[i]);
                            }

                            for (size_t i = 0; i < unit_indices.size(); i++) {
                                satisfied[unit_indices[i]] = false;
                            }

                            const auto& [pure_lits, pure_indices] = pure;

                            for (size_t i = 0; i < pure_lits.size(); i++) {
                                learned.erase(pure_lits[i]);
                            }

                            for (size_t i = 0; i < pure_indices.size(); i++) {
                                satisfied[pure_indices[i]] = false;
                            }

                            const std::vector<size_t>& satisfied_clauses = *indices;

                            for (size_t i = 0; i < satisfied_clauses.size(); i++) {
                                satisfied[satisfied_clauses[i]] = false;
                            }

                            return false;
                        }
                    }

                } 
                //cur_lit did not lead to a contradiction, learn it
                else {
                    //pure literal elimination afterwards
                    const learn_pair& pure = pure_lit_eliminate();

                    //find which clauses are satisfied and mark them as satisfied
                    const std::optional<std::vector<size_t>>& indices = mark_satisfied();

                    if (!indices.has_value()) {
                        return true;
                    }

                    if (solve(assignment)) {
                        return true;
                    } else { //cur_lit did not lead to a solution
                        //remove and switch the value of the current literal assignment
                        assignment.pop_back();
                        assignment.push_back(false);

                        //erase the learning of cur_lit as true
                        learned.erase(cur_lit);

                        //now assume the current literal is false
                        learned.insert(-cur_lit);

                        const auto& [unit, unit_indices] = *unit_prop;
                        for (size_t i = 0; i < unit.size(); i++) {
                            learned.erase(unit[i]);
                        }

                        for (size_t i = 0; i < unit_indices.size(); i++) {
                            satisfied[unit_indices[i]] = false;
                        }

                        const auto& [pure_lits, pure_indices] = pure;
                        for (size_t i = 0; i < pure_lits.size(); i++) {
                            learned.erase(pure_lits[i]);
                        }

                        for (size_t i = 0; i < pure_indices.size(); i++) {
                            satisfied[pure_indices[i]] = false;
                        }

                        const std::vector<size_t>& satisfied_clauses = *indices;
                        for (size_t i = 0; i < satisfied_clauses.size(); i++) {
                            satisfied[satisfied_clauses[i]] = false;
                        }

                        //relearn everything under the assumption that cur_lit = false
                        const std::optional<learn_pair>& new_unit_prop = learn();

                        //-cur_lit led to a contradiction
                        if (!new_unit_prop.has_value()) {
                            learned.erase(-cur_lit);
                            assignment.pop_back();

                            return false;
                        } else {
                            const learn_pair& new_pure = pure_lit_eliminate();

                            //recheck every clause that is now satisfied
                            const std::optional<std::vector<size_t>>& new_indices = mark_satisfied();

                            if (!new_indices.has_value()) {
                                return true;
                            }

                            if (solve(assignment)) { //now solve again
                                return true;
                            } else {
                                //if that fails, remove cur_lit's assignment
                                learned.erase(-cur_lit);

                                assignment.pop_back();

                                //unlearn everything we learned from cur_lit
                                const auto& [new_unit, new_unit_indices] = *new_unit_prop;
                                for (size_t i = 0; i < new_unit.size(); i++) {
                                    learned.erase(new_unit[i]);
                                }

                                for (size_t i = 0; i < new_unit_indices.size(); i++) {
                                    satisfied[new_unit_indices[i]] = false;
                                }

                                const auto& [new_pure_lits, new_pure_indices] = new_pure;
                                for (size_t i = 0; i < new_pure_lits.size(); i++) {
                                    learned.erase(new_pure_lits[i]);
                                }

                                for (size_t i = 0; i < new_pure_indices.size(); i++) {
                                    satisfied[new_pure_indices[i]] = false;
                                }

                                const std::vector<size_t>& new_satisfied_clauses = *new_indices;
                                for (size_t i = 0; i < new_satisfied_clauses.size(); i++) {
                                    satisfied[new_satisfied_clauses[i]] = false;
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