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
    std::vector<int> SATSolver::learn() {
        std::unordered_set<int> learnt;
        std::vector<bool> indices; //indices of clauses that were satisfied
        indices.resize(clauses.size());
        size_t cur_size = learned.size();

        //ensures that cur_size and new_size are not the same initially
        size_t new_size = cur_size + 1;

        while (cur_size != new_size) {
            cur_size = learned.size();
            for (size_t i = 0; i < clauses.size(); i++) {
                if (!indices[i]) {//just stops us from relearning clauses on each iteration
                    bool at_least_one_true = false;
                    if (clauses[i].size() == 1 && !learned.contains(clauses[i][0])) {
                        learnt.insert(clauses[i][0]);
                        learned.insert(clauses[i][0]);
                        indices[i] = true;
                    } else {
                        const std::vector<int>& clause = clauses[i];
                        int num_not_learnt = 0;
                        bool else_false = false;
                        size_t idx = 0;
                        //we can only add clause[j] if everything else is assigned or learnt AND false
                        for (size_t j = 0; j < clause.size(); j++) { 
                            bool was_learned = learned.contains(clause[j]) || learned.contains(-clause[j]);

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
                                        forced_val = !learned.contains(clause[j]);
                                    } else {
                                        //clause[j] > 0 => not negated
                                        //learned.contains(clause[j]) == true
                                        //forced_val = true
                                        //learned.contains(clause[j]) == false
                                        //forced_val = false
                                        forced_val = learned.contains(clause[j]);
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
                            learnt.insert(clause[idx]);
                            learned.insert(clause[idx]);
                            indices[i] = true; //this clause is now satisfied
                        }
                    }
                }
            }
            new_size = learned.size();
        }

        return std::vector(learnt.begin(), learnt.end());
    }
}