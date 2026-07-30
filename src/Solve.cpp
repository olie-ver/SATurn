#include "SAT.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>

namespace SATurn {
    bool SATSolver::solveCNF() {
        try_solved = true;
        assignment.resize(numVars);
        varData.resize(numVars);

        for (size_t i = 0; i < assignment.size(); i++) {
            assignment[i] = Unassigned;
        }

        if (!create_watched()) {
            return false;
        }

        while (true) {
            const std::optional<std::vector<int>>& delta = propagate();

            // delta.has_value() => conflict
            if (delta.has_value()) {
                // std::cout << "\nCurrent level trail:\n";

                //want a NEW clause instead of a reference
                std::unordered_set<int> conflict(delta.value().begin(), delta.value().end());

                //start from the end of the trail
                size_t start_from = trail.size() - 1;
                //end at the beginning of the current decision level
                size_t end_at = decisions.back().level_start;

                size_t current_level_count = 0;

                for (int i = static_cast<int>(start_from); i >= static_cast<int>(end_at); --i) {
                    if (conflict.contains(-trail[i].lit))
                    {
                        current_level_count++;
                    }
                }

                int uip;

                int i;

                for (i = static_cast<int>(start_from); i >= static_cast<int>(end_at); --i) {
                    const TrailEntry& t = trail[i];
                    int pivot = trail[i].lit;

                    if (!conflict.contains(-pivot)) {
                        continue;
                    }

                    current_level_count--;

                    if (current_level_count == 0) {
                        uip = -pivot;
                        break;
                    }

                    assert(*t.reason != -1);

                    const std::vector<int>& reason_clause = clauses[*t.reason];
                    std::unordered_set<int> reason(reason_clause.begin(), reason_clause.end());

                    assert(conflict.contains(-pivot));
                    assert(reason.contains(pivot));

                    conflict.merge(reason);

                    conflict.erase(-pivot);
                    conflict.erase(pivot);

                    current_level_count = 0;

                    for (int lit : conflict)
                    {
                        if (varData[abs(lit)-1].decision_level == decisions.size() - 1)
                            current_level_count++;
                    }
                }

                if (i < end_at) {
                    std::cout << "Ran off beginning of current decision level!\n";
                    abort();
                }

                //at this point, conflict *should* not contain any conflicting literals
                //  and it should now contain only decision literals and the values 
                //  they can't take

                //create the learned clause
                std::vector<int> learned(conflict.begin(), conflict.end());

                //grab the index of the decision level we need to back-jump to
                int current = decisions.size() - 1;

                int jump_to = 0;

                for (int lit : learned)
                {
                    int var = std::abs(lit) - 1;
                    int lvl = varData[var].decision_level;

                    if (lvl != current) {
                        jump_to = std::max(jump_to, lvl);
                    }
                }

                if (jump_to == current) {
                    assert(current == 0);
                    return false;
                }
                
                assert(jump_to < current);

                //back-jump
                while (decisions.size() - 1 > jump_to) {
                    size_t begin = decisions.back().level_start;

                    while (trail.size() > begin)
                    {
                        int lit = trail.back().lit;
                        assignment[std::abs(lit) - 1] = Unassigned;
                        varData[std::abs(lit) - 1].trail_index = -1;
                        varData[std::abs(lit) - 1].decision_level = -1;
                        trail.pop_back();
                    }

                    decisions.pop_back();
                }

                decisions.resize(jump_to + 1);
                qhead = decisions.back().level_start;


                //add it to the clauses database
                clauses.push_back(std::move(learned));

                //add the uip literal and the index of the learned clause to the trail
                //force assignment and varData

                int unassigned = 0;

                for (int lit : clauses.back())
                {
                    auto val = assignment[abs(lit)-1];

                    if (val == Unassigned)
                        unassigned++;

                    else if ((lit > 0 && val == True) ||
                            (lit < 0 && val == False))
                    {
                        std::cout << "Learned clause already satisfied!\n";
                    }
                }

                assert(unassigned == 1);

                int var = std::abs(uip) - 1;

                assignment[var] = (uip > 0) ? True : False;

                trail.push_back({uip, clauses.size() - 1});

                varData[var].trail_index = trail.size() - 1;
                varData[var].decision_level = jump_to;

                qhead = trail.size() - 1;

                //initialize the new clause's watched literals
                create_watched(clauses.back());

                if (decisions.empty()) {
                    return false;
                }

                //resolved conflict, move onto next iteration
                continue;
            }

            //check if all are assigned
            size_t first_unassigned = 0;
            for (size_t i = 0; i < assignment.size(); i++) {
                if (assignment[i] == Unassigned) {
                    first_unassigned = i + 1; //not the index, but the actual literal
                    break;
                }
            }

            //if all are assigned, return true
            if (!first_unassigned) {
                return true;
            }

            assert(assignment[first_unassigned - 1] == Unassigned);

            //otherwise, make a new decision
            assignment[first_unassigned - 1] = True;
            trail.push_back({static_cast<int>(first_unassigned), std::nullopt});
            decisions.push_back({trail.size() - 1, false});
            varData[first_unassigned - 1].trail_index = trail.size() - 1;
            varData[first_unassigned - 1].decision_level = decisions.size() - 1;
            //mark the newest decision as the starting point for propagation
            qhead = trail.size() - 1;
        }
    }
}