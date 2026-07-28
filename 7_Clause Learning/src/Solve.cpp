#include "SAT.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

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
                //want a NEW clause instead of a reference
                std::unordered_set<int> conflict(delta.value().begin(), delta.value().end());

                bool found = false;
                int trail_idx;

                for (int i = trail.size() - 1; i >= 0; --i)
                {
                    if (!trail[i].reason) {
                        continue;
                    }

                    if (conflict.contains(-trail[i].lit))
                    {
                        found = true;
                        trail_idx = i;
                        break;
                    }
                }

                while (found) {
                    found = false;
                    const TrailEntry& t = trail[trail_idx];
                    int pivot = t.lit;

                    const std::vector<int>& reason_clause = clauses[*t.reason];
                    std::unordered_set<int> reason(reason_clause.begin(), reason_clause.end());

                    // std::cout << "Resolving literal " << pivot << '\n';

                    // std::cout << "Conflict: ";
                    // for (int x : conflict)
                    //     std::cout << x << ' ';
                    // std::cout << '\n';

                    // std::cout << "Reason Clause: ";
                    // for (int x : reason_clause)
                    //     std::cout << x << ' ';
                    // std::cout << '\n';

                    // std::cout << "Reason: ";
                    // for (int x : reason)
                    //     std::cout << x << ' ';
                    // std::cout << '\n';

                    // std::cout << "Trail literal: "
                    //         << trail[trail_idx].lit
                    //         << '\n';
                    // std::cout << std::endl;

                    assert(conflict.contains(-pivot));
                    assert(reason.contains(pivot));

                    conflict.merge(reason);

                    if (conflict.contains(-pivot)) {
                        conflict.erase(-pivot);
                        conflict.erase(pivot);
                    }

                    for (int i = trail.size() - 1; i >= 0; --i)
                    {
                        if (!trail[i].reason) {
                            continue;
                        }

                        if (conflict.contains(-trail[i].lit))
                        {
                            found = true;
                            trail_idx = i;
                            break;
                        }
                    }

                    if (!found) {
                        trail_idx = -1;
                        break;
                    }
                }

                //at this point, conflict *should* not contain any conflicting literals
                //  and it should now contain only decision literals and the values 
                //  they can't take

                //create the learned clause
                std::vector<int> learned(conflict.begin(), conflict.end());

                //add it to the clauses database
                clauses.push_back(std::move(learned));

                //initialize the new clause's watched literals
                create_watched(clauses.back());

                //backtrack one level
                while (decisions.size() > 0) {
                    DecisionLevel& decision_level = decisions.back();

                    while (trail.size() > decision_level.level_start + 1)
                    {
                        int lit = trail.back().lit;
                        assignment[std::abs(lit) - 1] = Unassigned;
                        varData[std::abs(lit) - 1].trail_index = -1;
                        trail.pop_back();
                    }

                    if (!decision_level.tried_second_branch) {
                        TrailEntry& decision = trail.back();
                        assignment[std::abs(decision.lit) - 1] = Unassigned;
                        decision.lit = -decision.lit;
                        assignment[std::abs(decision.lit) - 1] = static_cast<lit>(decision.lit > 0);

                        //try the second branch
                        decision_level.tried_second_branch = true;
                        varData[abs(decision.lit)-1].trail_index = decision_level.level_start;
                        qhead = decision_level.level_start;
                        break; //something sus about this
                    } else {
                        int lit = trail.back().lit;
                        assignment[std::abs(lit) - 1] = Unassigned;
                        varData[std::abs(lit) - 1].trail_index = -1;
                        trail.pop_back();
                        decisions.pop_back();
                        qhead = decisions.back().level_start;
                    }
                }

                // std::cout << "\n=== AFTER BACKTRACK ===\n";

                // std::cout << "Trail:\n";
                // for (auto &t : trail)
                //     std::cout << t.lit << '\n';

                // std::cout << "\nAssignments:\n";
                // for (size_t i = 0; i < assignment.size(); i++)
                //     std::cout << i + 1 << ": " << assignment[i] << '\n';

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
                    first_unassigned = i + 1;
                    break;
                }
            }

            //if all are assigned, return true
            if (!first_unassigned) {
                return true;
            }

            //otherwise, make a new decision
            assignment[first_unassigned - 1] = True;
            trail.push_back({static_cast<int>(first_unassigned), std::nullopt});
            varData[first_unassigned - 1].trail_index = trail.size() - 1;
            decisions.push_back({trail.size() - 1, false});
            //mark the newest decision as the starting point for propagation
            qhead = trail.size() - 1;
        }
    }
}