#include "SAT.hpp"

#include <algorithm>

#include <cassert>
#include <iostream>

namespace saturn {
    bool satsolver::createWatched() {
        for (size_t i = 0; i < numClauses; i++) {
            const std::vector<int>& clause = clauses[i];

            if (clause.size() >= 2) {
                var_to_clause[var_to_widx(clause[0])].push_back(i);
                var_to_clause[var_to_widx(clause[1])].push_back(i);
                
                clause_to_var[i] = {0, 1};
            } else if (clause.size() == 1) {
                int var = clause[0];
                var_to_clause[var_to_widx(var)].push_back(i);

                clause_to_var[i] = {0, 0};

                //we want to be able to immediately perform unit propagation if
                //  there are unit clauses
                //so in our trail, we create a trail entry of the unit variable,
                //  its level (0), and the reason clause index (i)
                trail.push_back({var, 0, i});
                
                int idx = std::abs(var) - 1;
                if (var > 0) {
                    vars[idx] = TRUE;
                } else {
                    vars[idx] = FALSE;
                }

            } else {
                return false;
            }
        }

        return true;
    }

    bool satsolver::createWatched(size_t index, size_t decisionLevel) {
        const std::vector<int>& clause = clauses[index];

        //we need to check if the clause is unit
        //maybe the plan should be to get the index of the first True
        //  and/or Unassigned literal
        //and keep a count of how many true/false/unassigned literals there are
        //and if unassigned + false == clause.size() only then it's unit
        //if you can only find one place to have a watched literal, then the 
        //  other can be anywhere else except for the current watched literal
        //  (unless the clause size == 1)
        if (clause.size() == 0) {
            return false;
        }

        bool foundFirst = false;
        size_t firstIdx = 0;
        bool foundSecond = false;
        size_t secondIdx = 0;

        int numUnassigned = 0;
        int numFalse = 0;
        int numTrue = 0;

        for (size_t i = 0; i < clause.size(); i++) {
            int idx = std::abs(clause[i]) - 1;
            if (vars[idx] == UNASSIGNED) {
                numUnassigned++;
                if (!foundFirst) {
                    foundFirst = true;
                    firstIdx = i;
                } else if (!foundSecond) {
                    foundSecond = true;
                    secondIdx = i;
                }
            } else if (evals_false(clause[i])) {
                numFalse++;
            } else {
                numTrue++;

                if (!foundFirst) {
                    foundFirst = true;
                    firstIdx = i;
                } else if (!foundSecond) {
                    foundSecond = true;
                    secondIdx = i;
                }
            }
        }

        if (!foundFirst) {
            return false;
        }

        assert(foundFirst);

        if (clause.size() >= 2) {
            if (!foundSecond) {
                secondIdx = (firstIdx + 1) % clause.size();
            }

            var_to_clause[var_to_widx(clause[firstIdx])].push_back(index);
            var_to_clause[var_to_widx(clause[secondIdx])].push_back(index);
            
            clause_to_var[index] = {firstIdx, secondIdx};
        } else { //clause.size() == 1 since we already checked for == 0 at the top
            var_to_clause[var_to_widx(clause[firstIdx])].push_back(index);
            clause_to_var[index] = {firstIdx, firstIdx};
        }

        assert(numUnassigned == 1 && numTrue == 0);

        if (numUnassigned == 1 && numTrue == 0) {
            int var = clause[firstIdx];
            int idx = std::abs(var) - 1;
            trail.push_back({var, decisionLevel, index});
            var_to_trail[idx] = trail.size() - 1;

            if (var > 0) {
                vars[idx] = TRUE;
            } else {
                vars[idx] = FALSE;
            }
        }

        return true;
    }
}