#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    std::optional<SATSolver::learn_pair> SATSolver::learn() {
        std::vector<int> newly_learned;
        std::vector<size_t> indices;

        size_t cur_size = learned.size();

        //ensures that cur_size and new_size are not the same initially
        size_t new_size = cur_size + 1;

        while (cur_size != new_size) {
            cur_size = learned.size();
            for (size_t i = 0; i < clauses.size(); i++) {
                if (satisfied[i]) {
                    continue;
                }

                bool at_least_one_true = false;

                const std::vector<int>& clause = clauses[i];
                int num_not_learnt = 0;
                bool all_else_false = false;
                size_t idx = 0;

                for (size_t j = 0; j < clause.size(); j++) {
                    //if it's learned/assigned, then at least one is true, so break
                    if (learned.contains(clause[j])) {
                        at_least_one_true = true;
                        break;
                    //if the literal is not learned and neither is its negation
                    } else if (!learned.contains(-clause[j])) {
                        idx = j;
                        num_not_learnt++;
                        if (num_not_learnt > 1) {
                            break;
                        }
                    }
                }

                //at this point, all assignments in the clause are assigned and evaluate to false
                // except for this one, so we can learn that it is its value inside the clause
                // and mark that we learned it 
                if (num_not_learnt == 1 && !at_least_one_true) {
                    //contradiction because we're forced to learn the wrong assignment
                    if (learned.contains(-clause[idx])) {
                        for (size_t i = 0; i < newly_learned.size(); i++) {
                            learned.erase(newly_learned[i]);
                        }

                        for (size_t i = 0; i < indices.size(); i++) {
                            satisfied[indices[i]] = false;
                        }
                        // we will need to undo any changes made to learned
                        // and satisfied in here
                        return std::nullopt;
                    }
                    learned.insert(clause[idx]);
                    newly_learned.push_back(clause[idx]);
                    satisfied[i] = true;
                    indices.push_back(i);
                } 
                //contradiction because all are assigned and none of them evaluate to true
                // so this clause is not satisfiable
                else if (num_not_learnt == 0 && !at_least_one_true) {
                    // we will need to undo any changes made to learned
                    // and satisfied in here
                    for (size_t i = 0; i < newly_learned.size(); i++) {
                        learned.erase(newly_learned[i]);
                    }

                    for (size_t i = 0; i < indices.size(); i++) {
                        satisfied[indices[i]] = false;
                    }
                    return std::nullopt;
                }
            }
            new_size = learned.size();
        }

        //should print out the state of learned before and after
        // and make sure newly_learned does not overlap with state of learned beforehand

        return learn_pair{std::move(newly_learned), std::move(indices)};
    }

    SATSolver::learn_pair SATSolver::pure_lit_eliminate() {
        std::vector<int> pure;
        std::vector<size_t> indices;
        std::unordered_map<int, u_int> counts;

        //for each currently unsatisfied clause, add its literal
        //  counts to the counts map
        for (size_t i = 0; i < numClauses; i++) {
            if (satisfied[i]) {
                continue;
            }

            const std::vector<int>& clause = clauses[i];
            for (size_t j = 0; j < clause.size(); j++) {
                counts[clause[j]]++;
            }
        }

        //while num_pure != 0

        //is every clause satisfied? => all indices[i] == true => all clauses get skipped
        while (true) {
            int num_pure = 0;
            //for each literal-count pair
            // if counts doesn't contain -lit => -lit never appeared in an unsatisfied clause
            // and learned doesn't contain -lit => -lit is not forced yet
            // add it to our list of pure literals, learn it, increment the count
            for (auto const& [lit, count] : counts) {
                if (!learned.contains(lit) && !counts.contains(-lit) && !learned.contains(-lit))
                {
                    auto [it, inserted] = learned.insert(lit);
                    if (inserted)
                    {
                        pure.push_back(lit);
                        num_pure++;
                    }
                }
            }

            //if there are no pure literals, break
            if (num_pure == 0) {
                break;
            }

            //for each unsatisfied clause
            //check if the clause is now satisfied from learning a pure literal
            //then if it is, decrease the counts of each literal in that clause
            for (size_t i = 0; i < numClauses; i++) {
                if (satisfied[i]) {
                    continue;
                }

                //check if the current clause is satisfied by a pure literal (one already learned)
                const std::vector<int>& clause = clauses[i];
                for (size_t j = 0; j < clause.size(); j++) {
                    if (learned.contains(clause[j])) {
                        satisfied[i] = true;
                        indices.push_back(i);
                        break;
                    }
                }

                //if the clause is satisfied, then decrease the counts of every literal in the clause
                if (satisfied[i]) {
                    for (size_t j = 0; j < clause.size(); j++) {
                        counts[clause[j]]--;
                    }
                }
            }

            //iterate over the map and then erase all elements with a count of 0
            //this means that when we loop back to the top, our check for pure literals
            // remains the same
            for (auto it = counts.begin(); it != counts.end();) {
                if (it->second == 0) {
                    it = counts.erase(it);
                } else {
                    ++it;
                }
            }
        }

        return learn_pair{std::move(pure), std::move(indices)};
    }

    std::optional<std::vector<size_t>> SATSolver::mark_satisfied() {

        std::vector<size_t> indices;
        size_t num_satisfied = 0;
        for (size_t i = 0; i < numClauses; i++) {
            if (satisfied[i]) {
                num_satisfied++;
                continue;
            }

            const std::vector<int>& clause = clauses[i];
            for (size_t j = 0; j < clause.size(); j++) {
                //no else branch needed because learned implicitly contains 
                // all literal assignments, even if we haven't concretely assigned
                // a literal in solve()
                if (learned.contains(clause[j])) {
                    satisfied[i] = true;
                    indices.push_back(i);
                    num_satisfied++;
                    break;
                }
            }
        }

        //if everything is already solved => return nothing, to let the solver know it's done
        if (num_satisfied == numClauses) {
            return std::nullopt;
        }

        return indices;
    }
}