#include "SAT.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>

namespace SATurn {
    std::optional<std::vector<int>> SATSolver::propagate() {
        std::vector<int> delta;

        while (qhead < trail.size()) {
            int lit = trail[qhead].lit;
            qhead++;

            if (watch_to_clause.contains(-lit)) {
                std::vector<size_t>& clause_indices = watch_to_clause[-lit];

                size_t i = 0;
                while (i < clause_indices.size()) {
                    size_t clause_idx = clause_indices[i];
                    //grab the clause
                    const std::vector<int>& clause = clauses[clause_idx];

                    //grab the watch_indices
                    std::pair<size_t, size_t>& watch_indices = clause_to_watch[clause_idx];

                    assert(clause[watch_indices.first] == -lit || clause[watch_indices.second] == -lit);
                    if (clause[watch_indices.first] != -lit) {
                        std::swap(watch_indices.first, watch_indices.second);
                    }
                    assert(clause[watch_indices.first] == -lit);

                    //find the next eligible watched literal, which is either a true or unassigned literal
                    //if there is none, and lits[std::abs(clause[watch_indices.second]) - 1] == true, do nothing
                    //else if std::abs(clause[watch_indices.second]) - 1 > lits.size(), assign it true, and propagate 
                    //  on clause[watch_indices.second]
                    //otherwise it's a conflict, so return false and undo anything learned during this time
                    bool relocated = false;
                    for (size_t j = 0; j < clause.size(); j++) {
                        if (j == watch_indices.first || j == watch_indices.second) {
                            continue;
                        }

                        size_t lit_idx = std::abs(clause[j]) - 1;
                        //since false = 0, true = 1, unassigned = 2, lits[lit_idx] => true or unassigned

                        if (assignment[lit_idx] == Unassigned) {
                            watch_indices.first = j;

                            clause_indices[i] = clause_indices.back();
                            clause_indices.pop_back();
                            watch_to_clause[clause[j]].push_back(clause_idx);
                            
                            relocated = true;
                            break;
                        } else {
                            if (clause[j] < 0) {
                                if (assignment[lit_idx] == False) {
                                    watch_indices.first = j;

                                    clause_indices[i] = clause_indices.back();
                                    clause_indices.pop_back();
                                    watch_to_clause[clause[j]].push_back(clause_idx);
                                    
                                    relocated = true;
                                    break;
                                }
                            } else {
                                if (assignment[lit_idx] == True) {
                                    watch_indices.first = j;

                                    clause_indices[i] = clause_indices.back();
                                    clause_indices.pop_back();
                                    watch_to_clause[clause[j]].push_back(clause_idx);
                                    
                                    relocated = true;
                                    break;
                                }
                            }
                        }
                    }

                    //if we couldn't relocate the watched literal
                    if (!relocated) {
                        i++;
                        //if it's unassigned
                        size_t lit_idx = std::abs(clause[watch_indices.second]) - 1;
                        if (assignment[lit_idx] == Unassigned) {
                            assert(assignment[std::abs(clause[watch_indices.second]) - 1] == Unassigned);
                            //We only push back into delta (imply a variable) here
                            // therefore, in our trail, we add in a new implication
                            // containing the assignment (T/F), the level (same as the one at the previous index)
                            // and the decision clause (clause)
                            delta.push_back(clause[watch_indices.second]);
                            //if it's positive, the literal becomes true, otherwise it becomes false
                            if (clause[watch_indices.second] > 0) {
                                assignment[lit_idx] = True;
                            } else {
                                assignment[lit_idx] = False;
                            }

                            //add this to the trail
                            trail.push_back({clause[watch_indices.second], clause_idx});

                            //mark this literal's trail index
                            int lit = clause[watch_indices.second];

                            int var = std::abs(lit) - 1;

                            // std::cout << "Trying to imply " << lit << '\n';

                            // std::cout << "Current assignment = "
                            //         << assignment[var]
                            //         << '\n';

                            // std::cout << "Clause: ";

                            // for (int x : clause)
                            //     std::cout << x << ' ';

                            // std::cout << '\n';

                            varData[std::abs(clause[watch_indices.second]) - 1].trail_index = trail.size() - 1;
                        } 
                        else {
                            //if it's true within the clause, move on
                            if (clause[watch_indices.second] > 0 && assignment[lit_idx] == True) {
                                continue;
                            }

                            if (clause[watch_indices.second] < 0 && assignment[lit_idx] == False) {
                                continue;
                            }

                            return clause;
                        }
                    }
                }
            }
        }

        return std::nullopt;
    }
}