#include "SAT.hpp"
#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::create_watched() {
        for (size_t i = 0; i < clauses.size(); i++) {
            const std::vector<int>& clause = clauses[i];

            if (clause.size() >= 2) {
                //clause i is being watched by the literals at indices 0 and 1
                clause_to_watch[i] = {0, 1};

                //literal 1 is watching clause i
                watch_to_clause[clause[0]].push_back(i);

                //literal 2 is watching clause i
                watch_to_clause[clause[1]].push_back(i);
            } else if (clause.size() == 1) {
                //clause[i] is being watched by only the literal at index 0
                clause_to_watch[i] = {0, 0};

                //literal 1 is watching clause i
                watch_to_clause[clause[0]].push_back(i);
            } else {
                //there are no literals to watch this clause with
                //ie, it's an unsatisfiable clause, so return false
                return false;
            }
        }

        return true;
    }
}