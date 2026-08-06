#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::solveCNF() {
        try_solved = true;
        lits.resize(numVars);

        for (size_t i = 0; i < lits.size(); i++) {
            lits[i] = Unassigned;
        }

        if (!create_watched()) {
            return false;
        }

        return solve(0);
    }

    bool SATSolver::solve(size_t level) {
        if (level == numVars) {
            bool solved = true;
            for (size_t i = 0; i < clauses.size(); i++) {
                bool true_clause = false;
                //evaluate whether or not the current clause evaluates to true
                for (size_t j = 0; j < clauses[i].size(); j++) {
                    int val = clauses[i][j];
                    int idx = std::abs(val) - 1;
                    if (val < 0) {
                        true_clause |= !lits[idx];
                    } else {
                        true_clause |= lits[idx];
                    }
                }
                //check if the whole equation evaluates to true
                solved &= true_clause;
            }
            return solved;
        } else {
            //we will assume that all propagation is done upon learn time, not assignment time
            int cur_lit = level + 1;
            if (lits[level] != Unassigned) {
                return solve(level + 1);
            } else {
                lits[level] = True;

                //if propagation leads to a contradiction

                const std::optional<std::vector<int>>& delta = propagate(cur_lit);
                if (!delta.has_value()) {
                    //assume lits[level] = False
                    lits[level] = False;

                    //if that leads to a contradiction, make it Unassigned
                    const std::optional<std::vector<int>>& new_delta = propagate(-cur_lit);

                    if (!new_delta.has_value()) {
                        lits[level] = Unassigned;
                        return false;
                    } else {
                        //if you couldn't solve it as False, make it Unassigned
                        if (!solve(level + 1)) {
                            lits[level] = Unassigned;

                            //undo everything from propagate
                            const std::vector<int>& new_delta_lits = *new_delta;
                            for (size_t i = 0; i < new_delta_lits.size(); i++) {
                                lits[std::abs(new_delta_lits[i]) - 1] = Unassigned;
                            }
                            return false;
                        }
                        return true;
                    }
                }

                //if you couldn't solve it as True, try it as False
                if (!solve(level + 1)) {
                    lits[level] = False;

                    //undo the propagation
                    const std::vector<int>& delta_lits = *delta;
                    for (size_t i = 0; i < delta_lits.size(); i++) {
                        lits[std::abs(delta_lits[i]) - 1] = Unassigned;
                    }

                    //if couldn't propagate as False, make it Unassigned
                    const std::optional<std::vector<int>>& new_delta = propagate(-cur_lit);
                    if (!new_delta.has_value()) {
                        lits[level] = Unassigned;
                        return false;
                    } else {
                        //if couldn't solve with it as False, make it Unassigned
                        if (!solve(level + 1)) {
                            lits[level] = Unassigned;
                            
                            //undo everything from propagate
                            const std::vector<int>& new_delta_lits = *new_delta;
                            for (size_t i = 0; i < new_delta_lits.size(); i++) {
                                lits[std::abs(new_delta_lits[i]) - 1] = Unassigned;
                            }
                            return false;
                        }

                        return true;
                    }
                }
                return true;
            }
        }
    }
}