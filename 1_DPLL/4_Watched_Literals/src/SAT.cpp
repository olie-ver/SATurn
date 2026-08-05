#include "SAT.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

namespace saturn {
    bool satsolver::solveCNF() {
        vars.resize(numVars);

        for (size_t i = 0; i < numVars; i++) {
            vars[i] = UNASSIGNED;
        }

        //if we can't create watched literals, this equation is unsatisfiable
        if (!createWatched()) {
            return false;
        }

        //if propagation failed at the root level, return false
        //  because that means there's a root level contradiction
        if (!propagate(0).has_value()) {
            return false;
        }

        return solve(0);
    }

    bool satsolver::solve(int level) {
        //because propagate() now does contradiction detection for us, 
        //  satisfiability is equivalent to all variables being assigned
        if (level == numVars) {
            return true;
        }

        //if this variable is learned, continue onto the next level
        if (vars[level] != UNASSIGNED) {
            return solve(level + 1);
        }

        int cur_var = level + 1;

        //start out with True
        vars[level] = TRUE;

        //propagate
        const std::optional<std::vector<int>>& prop_result = propagate(cur_var);

        //if there was a contradiction
        if (!prop_result.has_value()) {
            //try False
            vars[level] = FALSE;

            //propagate again, pass in -level because we're propagating on the negation
            const std::optional<std::vector<int>>& new_prop_result = propagate(-cur_var);

            //if there was another contradiction, go back up,
            //no need to undo anything because there's nothing to undo from the 
            //first propagation
            if (!new_prop_result.has_value()) {
                return false;
            }

            //if we can satisfy the equation, return true
            if (solve(level + 1)) {
                return true;
            }

            //otherwise, undo the propagation and return false
            const std::vector<int>& new_delta = *new_prop_result;

            for (size_t i = 0; i < new_delta.size(); i++) {
                vars[new_delta[i]] = UNASSIGNED;
            }

            return false;
        } 

        //if there was no contradiction, try solving at the next level
        if (solve(level + 1)) {
            return true;
        }

        //undo the first propagation
        const std::vector<int>& delta = *prop_result;

        for (size_t i = 0; i < delta.size(); i++) {
            vars[delta[i]] = UNASSIGNED;
        }

        //now try again with False
        vars[level] = FALSE;

        //propagate again, pass in -level because we're propagating on the negation
        const std::optional<std::vector<int>>& new_prop_result = propagate(-cur_var);

        //if there was a contradiction, go back up
        if (!new_prop_result.has_value()) {
            return false;
        }

        //if there was no contradiction, try solving again
        if (solve(level + 1)) {
            return true;
        }

        //if we couldn't satisfy the equation, undo the second propagation
        //  and return false
        const std::vector<int>& new_delta = *new_prop_result;

        for (size_t i = 0; i < new_delta.size(); i++) {
            vars[new_delta[i]] = UNASSIGNED;
        }

        return false;
    }
}