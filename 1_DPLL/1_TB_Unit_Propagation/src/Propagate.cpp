#include "SAT.hpp"

#include <unordered_set>

namespace saturn {
    void satsolver::propagate(std::vector<var>& assignment) {
        //we want this loop to terminate when we don't find a new unit clause
        // and when we don't find any new unit variables
        while (true) {
            //we will be repeatedly rescanning clauses made unit, 
            // but this implementation isn't exactly speed-focused to begin with
            std::unordered_set<int> unitClauses;
            std::unordered_set<int> unitVars;

            //gathers new unit clauses and unit variables
            for (size_t i = 0; i < numClauses; i++) {
                const std::vector<int>& clause = clauses[i];

                bool isUnit = true;
                int unassignedIdx = -1;
                int numUnassigned = 0;

                for (size_t j = 0; j < clause.size() && isUnit; j++) {
                    int var = clause[j];
                    int idx = std::abs(var) - 1;
                    if (assignment[idx] == UNASSIGNED) {
                        numUnassigned++;
                        unassignedIdx = j;
                    } 
                    //if anything evaluates to true, then the clause is not unit
                    else if (var > 0 && assignment[idx] == TRUE  || var < 0 && assignment[idx] == FALSE)
                    {
                        isUnit = false;
                    }
                    isUnit = isUnit && numUnassigned == 1;
                }

                if (isUnit && numUnassigned == 1) {
                    unitClauses.insert(i);
                    unitVars.insert(clause[unassignedIdx]);
                    
                    int var = clause[unassignedIdx];
                    int idx = std::abs(var) - 1;

                    if (var > 0) {
                        assignment[idx] = TRUE;
                    } else {
                        assignment[idx] = FALSE;
                    }
                }
            }

            //erases clauses and variables from their clauses
            size_t clauseIdx = 0;
            for (auto it = clauses.begin(); it != clauses.end();) {
                bool erasedClause = false;
                std::vector<int>& clause = *it;

                for (auto idx = clause.begin(); idx != clause.end();) {
                    if (unitVars.contains(-(*idx))) {
                        idx = clause.erase(idx);
                    } else {
                        idx++;
                    }

                    if (unitVars.contains(*idx) && !unitClauses.contains(clauseIdx)) {
                        it = clauses.erase(it);
                        erasedClause = true;
                        break;
                    }
                }

                clauseIdx++;

                if (!erasedClause) {
                    it++;
                }
            }

            //once we find no more unit clauses and no more unit variables
            //  we break out of the loop and exit the function
            if (unitClauses.size() == 0 && unitVars.size() == 0) {
                break;
            }
        }
    }
}