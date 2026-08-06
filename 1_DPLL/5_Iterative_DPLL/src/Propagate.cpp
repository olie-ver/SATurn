#include "SAT.hpp"

namespace saturn {
    bool satsolver::propagate(size_t decisionLevel) {
        while (qHead < trail.size()) {
            int prop = trail[qHead].lit;
            qHead++;

            //if this variable is watching nothing, do nothing
            if (!var_to_clause.contains(-prop)) {
                continue;
            }

            //we want to grab the clauses of the false literal
            //ie, if we assign 4 = False, we want to grab clauses
            //  4 is watching, but if we assign 4 = True, we want to grab 
            //  clauses -4 is watching
            //because unit propagation can only happen on the clauses becoming
            //  "more false"
            //since we can pass in negative literals to signify which one is True,
            //  we always want to grab the clauses being watched by -prop
            //eg, -4 = T => 4 = F => watched_clauses = var_to_clause[-(-4)]
            //    4 = T => -4 = F => watched_clauses = var_to_clause[-(4)]
            std::vector<size_t>& watched_clauses = var_to_clause[-prop];

            for (size_t i = 0; i < watched_clauses.size();) {
                size_t clause_idx = watched_clauses[i];

                //get the clause at the current clause index
                const std::vector<int>& clause = clauses[clause_idx];

                //grab the clause's watched literal indices
                std::pair<size_t, size_t>& watches = clause_to_var[clause_idx];

                //if the variable at the first watch index isn't the negation of 
                //  the variable we're propagating on, swap the indices
                if (clause[watches.first] != -prop) {
                    std::swap(watches.first, watches.second);
                }

                bool relocated = false;
                //for each variable in the clause
                for (size_t j = 0; j < clause.size(); j++) {
                    //skip if the index is the same as our current watch indices
                    if (j == watches.first || j == watches.second) {
                        continue;
                    }

                    //grab the UNWATCHED variable at clause[j]
                    int unwatched = clause[j];
 
                    //if it doesn't evaluate to false
                    if (!evals_false(unwatched)) {
                        //the unwatched variable will now watch this clause in addition to 
                        //  any other clauses it may be watching
                        var_to_clause[unwatched].push_back(clause_idx);

                        //the variable that is at the index of the first watched literal
                        //  will no longer watch this clause
                        //swap the clause indices of the current clause and the last clause index
                        //  to perform a swap and pop
                        std::swap(watched_clauses[i], watched_clauses.back());
                        //the current watched literal is not watching this clause anymore
                        watched_clauses.pop_back();

                        //set j to be the new index of the first watched literal
                        watches.first = j;

                        relocated = true;
                        break;
                    }
                }

                if (!relocated) {
                    //we want to increment i here because we didn't swap and pop
                    //  ie, watched_clauses[i] points to the same thing,
                    //  but if we did swap and pop, watched_clauses[i] points to something new
                    i++;
                    //if we couldn't relocate the watched literal, then our only hope for 
                    //  not having a contradiction is for clause[watches.second] to be 
                    //  TRUE or UNASSIGNED

                    int other_watch = clause[watches.second];
                    int idx = std::abs(other_watch) - 1;

                    if (vars[idx] == UNASSIGNED) {

                        //propagate
                        if (other_watch > 0) {
                            vars[idx] = TRUE;
                        } else {
                            vars[idx] = FALSE;
                        }

                        //push a trail entry
                        trail.push_back({other_watch, decisionLevel, clause_idx});
                    } else {

                        if (!evals_false(other_watch)) {
                            //we're all good, move onto the next literal to propagate on
                            continue;
                        }

                        //return false to indicate that we encountered a contradiction
                        return false;
                    }
                }
            }
        }

        //return the changes we made to assignment
        return true;
    }
}