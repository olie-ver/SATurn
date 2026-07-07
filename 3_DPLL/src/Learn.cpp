#include "SAT.hpp"

#include <algorithm>
#include <iostream>

namespace SATurn {
    //in order to effectively learn, it isn't just one pass.
    // it needs to look at clauses with multiple literals and check if there are 
    // enough satisfied variables in a clause to learn something else

    //while the number of learnt clauses is not growing
    //go through each clause
    // if the clause's size is 1, add in the literal as it is
    // otherwise, check if everything in the clause, except for 1 literal is 
    //  learnt. if everything but one has been learnt, then the one that hasn't been learnt
    //  can now be learnt
    // ie: {1}, {-1, 2}, {1, -2, 3}
    // => 1 is learnt. Since -1 = false => 2 = true => 2 is learnt
    // => third clause is satisfied and 3 can be either
    // it should be regardless of whether or not a clause has been satisfied, but
    // we could check which clauses were affected
    std::optional<std::vector<int>> SATSolver::learn() {
        std::unordered_set<int> learnt(learned.begin(), learned.end());
        std::vector<bool> indices = satisfied; //indices of clauses that were satisfied
        indices.resize(clauses.size());
        // size_t cur_size = learned.size();
        size_t cur_size = learnt.size();

        //ensures that cur_size and new_size are not the same initially
        size_t new_size = cur_size + 1;

        while (cur_size != new_size) {
            // cur_size = learned.size();
            cur_size = learnt.size();
            for (size_t i = 0; i < clauses.size(); i++) {
                if (!indices[i]) {//just stops us from relearning clauses on each iteration
                    bool at_least_one_true = false;
                    // if (clauses[i].size() == 1 && !learned.contains(clauses[i][0])) {
                    if (clauses[i].size() == 1 && !learnt.contains(clauses[i][0])) {
                        if (learnt.contains(-clauses[i][0])) {
                            return std::nullopt;
                        }

                        learnt.insert(clauses[i][0]);
                        // learned.insert(clauses[i][0]);
                        indices[i] = true;
                    } else {
                        const std::vector<int>& clause = clauses[i];
                        int num_not_learnt = 0;
                        bool else_false = false;
                        size_t idx = 0;
                        //we can only add clause[j] if everything else is assigned or learnt AND false
                        for (size_t j = 0; j < clause.size(); j++) { 
                            // bool was_learned = learned.contains(clause[j]) || learned.contains(-clause[j]);
                            bool was_learned = learnt.contains(clause[j]) || learnt.contains(-clause[j]);

                            //if it wasn't learned and it's not assigned
                            if (!was_learned && std::abs(clause[j]) > (*lits).size()) {
                                idx = j;
                                num_not_learnt++;

                                if (num_not_learnt > 1) {
                                    break;
                                } 
                            } else {
                                //it's either learned or assigned
                                //if it was learned


                                //if it was learned, I need to get the learned value
                                // it it was assigned I need to get the assigned value
                                // I then need to evaluate it as it is within the clause
                                bool forced_val;
                                if (was_learned) {
                                    if (clause[j] < 0) {
                                        //clause[j] < 0 => negated
                                        //learned.contains(clause[j]) == true
                                        //forced_val = false
                                        //learned.contains(clause[j]) == false
                                        //forced_val = true

                                        // forced_val = !learned.contains(clause[j]);
                                        forced_val = !learnt.contains(clause[j]);
                                    } else {
                                        //clause[j] > 0 => not negated
                                        //learned.contains(clause[j]) == true
                                        //forced_val = true
                                        //learned.contains(clause[j]) == false
                                        //forced_val = false

                                        // forced_val = learned.contains(clause[j]);
                                        forced_val = learnt.contains(clause[j]);
                                    }

                                } else { //if it was assigned
                                    forced_val = (*lits)[std::abs(clause[j]) - 1];
                                }

                                bool eval;

                                if (clause[j] < 0) {
                                    eval = !forced_val;
                                } else {
                                    eval = forced_val;
                                }

                                //if it evaluates to true within the clause
                                // then I can't learn anything within the clause
                                if (eval) {
                                    at_least_one_true = true;
                                }

                            }
                        }

                        //at this point, all assignments in the clause are assigned and evaluate to false
                        // except for this one, so we can learn that it is its value inside the clause
                        // and mark that we learned it 
                        if (num_not_learnt == 1 && !at_least_one_true) {
                            if (learnt.contains(-clause[idx])) {
                                return std::nullopt;
                            }
                            learnt.insert(clause[idx]);
                            // learned.insert(clause[idx]);
                            indices[i] = true; //this clause is now satisfied
                        } 
                        //contradiction because all are assigned and none of them evaluate to true
                        // so this clause is not satisfiable
                        else if (num_not_learnt == 0 && !at_least_one_true) {
                            return std::nullopt;
                        }
                    }
                }
            }
            // new_size = learned.size();
            new_size = learnt.size();
        }

        std::vector<int> delta;

        for (int lit : learnt)
        {
            if (!learned.contains(lit)) {
                delta.push_back(lit);
            }
        }

        return delta;

        // return std::vector(learnt.begin(), learnt.end());
    }

    std::vector<int> SATSolver::pure_lit_eliminate() {
        // std::vector<int> pure;
        std::unordered_set<int> pure;
        std::vector<bool> indices = satisfied;
        std::unordered_map<int, u_int> counts;

        //for each currently unsatisfied clause, add its literal
        //  counts to the counts map
        for (size_t i = 0; i < numClauses; i++) {
            if (indices[i]) {
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
                if (!learned.contains(lit) && !counts.contains(-lit) 
                    && !learned.contains(-lit) && !pure.contains(lit))
                {
                    pure.insert(lit);
                    num_pure++;
                    // auto [it, inserted] = learned.insert(lit);
                    // if (inserted)
                    // {
                    //     pure.push_back(lit);
                    //     num_pure++;
                    // }
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
                if (indices[i]) {
                    continue;
                }

                //check if the current clause is satisfied by a pure literal (one already learned)
                const std::vector<int>& clause = clauses[i];
                bool is_satisfied = false;
                for (size_t j = 0; j < clause.size(); j++) {
                    // if (learned.contains(clause[j])) {
                    if (learned.contains(clause[j]) || pure.contains(clause[j])) {
                        is_satisfied = true;
                        indices[i] = true;
                        break;
                    }
                }

                //if the clause is satisfied, then decrease the counts of every literal in the clause
                if (is_satisfied) {
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

        // return pure;
        return std::vector<int>(pure.begin(), pure.end());
    }

    //make it check the lit's current assignment as well if applicable
    std::vector<size_t> SATSolver::mark_satisfied() {
        std::vector<size_t> indices;
        for (size_t i = 0; i < numClauses; i++) {
            if (satisfied[i]) {
                continue;
            }

            const std::vector<int>& clause = clauses[i];
            for (size_t j = 0; j < clause.size(); j++) {
                if (learned.contains(clause[j])) {
                    indices.push_back(i);
                    satisfied[i] = true;
                    break;
                } else {
                    int idx = std::abs(clause[j]) - 1;
                    if (idx < (*lits).size()) {
                        bool val;
                        if (clause[j] < 0) {
                            val = !(*lits)[idx];
                        } else {
                            val = (*lits)[idx];
                        }

                        if (val) {
                            indices.push_back(i);
                            satisfied[i] = true;
                            break;
                        }
                    }
                }
            }
        }

        return indices;
    }
}