#include "SAT.hpp"

#include <unordered_set>
#include <queue>

namespace saturn {
    std::pair<std::vector<int>, std::vector<int>> 
    satsolver::propagate(std::vector<var>& assignment, int level) {
        std::queue<int> unit;

        std::vector<int> foundUnit;
        std::vector<int> foundSatisfied;

        //the variable with index level is decided, so unit should hold onto it
        unit.push(level);

        while (!unit.empty()) {
            //prevents us from getting into an infinite loop
            unit.pop();

            for (size_t i = 0; i < numClauses; i++) {
                //if we've already satisfied this clause, move on
                if (satisfied[i]) {
                    continue;
                }

                const std::vector<int>& clause = clauses[i];

                //basically the same scanning logic as textbook unit propagation
                bool isUnit = true;
                int unassignedIdx = -1;
                int numUnassigned = 0;

                for (size_t j = 0; j < clause.size(); j++) {
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

                        //if this clause has something that evaluates to true, we should
                        //  mark it as satisfied and make sure that we mark that we found it 
                        //  to be satisfied
                        satisfied[i] = true;
                        foundSatisfied.push_back(i);
                    }
                }

                //if the clause is unit, then we did not mark it satisfied
                if (isUnit && numUnassigned == 1) {
                    int var = clause[unassignedIdx];
                    int idx = std::abs(var) - 1;

                    if (var > 0) {
                        assignment[idx] = TRUE;
                    } else {
                        assignment[idx] = FALSE;
                    }

                    unit.push(var);
                    foundUnit.push_back(var);

                    //which is why we should mark it satisfied
                    satisfied[i] = true;
                    foundSatisfied.push_back(i);
                }
            }
        }

        //now return the unit variables and the clauses we've marked as satisfied
        return std::pair{std::move(foundUnit), std::move(foundSatisfied)};
    }
}