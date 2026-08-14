#include "SAT.hpp"

#include <algorithm>
#include <limits>
#include <unordered_set>
#include <cassert>
#include <iostream>

namespace saturn {
    bool satsolver::solveCNF() {
        //if we can't create watched literals, this equation is unsatisfiable
        if (!createWatched()) {
            return false;
        }

        size_t decisionLevel = 0;

        //start a new decision level at the root
        trail_starts.push_back(0);

        //if createWatched found unit clauses, then they're on the trail
        if (trail.size() > 0) {
            //propagate and if it failed, we can't do anything about it
            if (propagate(decisionLevel).has_value()) {
                return false;
            }
        }

        while (true) {
            if (qHead == trail.size()) {
                decisionLevel++;

                std::optional<int> firstUnassigned;
                for (size_t i = 0; i < numVars; i++) {
                    if (vars[i] == UNASSIGNED) {
                        //variable = idx + 1
                        firstUnassigned = i + 1;
                        break;
                    }
                }

                //if there is none => all assigned => satisfied
                if (!firstUnassigned.has_value()) {
                    return true;
                }

                //assign this variable to be True (okay because firstUnassigned must be positive => True)
                vars[*firstUnassigned - 1] = TRUE;

                //push back the first unassigned variable (assumed True), the decision level
                //  and nullopt to show that this was a decision, not propagation/inference
                trail.push_back({*firstUnassigned, decisionLevel, std::nullopt});

                //Record the index in the trail where the first unassigned variable went
                var_to_trail[*firstUnassigned - 1] = trail.size() - 1;

                //the newest decision level begins at the index of the last trail entry
                trail_starts.push_back(trail.size() - 1);

                //our propagation queue now starts at the latest decision
                qHead = trail_starts.back();
            }

            const std::optional<std::vector<int>>& contradiction = propagate(decisionLevel);

            //if there's a conflicting clause
            if (contradiction.has_value()) {    
                //we want a copy instead of a reference since we'll be mutating the conflict 
                //  clause into a learned clause
                std::vector<int> conflict = *contradiction;  

                //Two passes: pass 1 gathers the trail index of the least recently
                //  propagated variable 
                //Pass 2 will resolve on the other propagated variables until 
                //  the pointer or however I'm keeping track of each propagated variable
                //  reaches the pointer of the first propagated variable

                size_t end = trail.size();
                size_t begin = 0;
                int resolve_var = 0;

                for (size_t i = 0; i < conflict.size(); i++) {
                    int var = conflict[i];
                    int idx = std::abs(var) - 1;
                    size_t t_idx = *var_to_trail[idx];

                    //get the index of the least recently propagated literal
                    //  by checking if its decision level is the current one
                    //  and that it has a reason clause
                    //  then taking the minimum of the current trail end
                    //  and the trail index of that literal
                    //get the index of the most recently propagated literal
                    //  in the trail by getting the maximum trail index
                    //  of current decision level propagated literals
                    if (trail[t_idx].decisionLevel == decisionLevel)
                    {
                        end = std::min(end, t_idx);
                        begin = std::max(begin, t_idx);
                        if (trail[t_idx].reasonIdx.has_value()) {
                            resolve_var = trail[t_idx].lit;
                        }
                    }
                }

                //we continue resolving on literals until the begin pointer
                //  equals the end pointer (that's when we've encountered the 
                //      last propagated literal)
                while (begin != end) {
                    assert(resolve_var != 0);
                    size_t v_idx = *var_to_trail[std::abs(resolve_var) - 1];

                    const std::vector<int>& reason = clauses[*trail[v_idx].reasonIdx];
                    conflict.insert(conflict.end(), reason.begin(), reason.end());
                    //remove the lit we're resolving on
                    for (size_t i = 0; i < conflict.size();) {
                        if (std::abs(conflict[i]) == std::abs(resolve_var)) {
                            std::swap(conflict[i], conflict.back());
                            conflict.pop_back();
                        } else {
                            i++;
                        }
                    }

                    //remove all duplicates
                    std::unordered_set<int> duplicates;
                    duplicates.reserve(conflict.size());
                    for (size_t i = 0; i < conflict.size();) {
                        if (duplicates.contains(conflict[i])) {
                            std::swap(conflict[i], conflict.back());
                            conflict.pop_back();
                        } else {
                            duplicates.insert(conflict[i]);
                            i++;
                        }
                    }

                    //since begin is the max, we need to reset it before 
                    //  we recompute it
                    //since end is the min, reset it as well
                    end = ULONG_MAX;
                    begin = 0;

                    for (size_t i = 0; i < conflict.size(); i++) {
                        int var = conflict[i];
                        int idx = std::abs(var) - 1;
                        size_t t_idx = *var_to_trail[idx];

                        if (trail[t_idx].decisionLevel == decisionLevel)
                        {
                            end = std::min(end, t_idx);
                            begin = std::max(begin, t_idx);

                            if (trail[t_idx].reasonIdx.has_value()) {
                                resolve_var = trail[t_idx].lit;
                            }
                        }
                    }
                }

                if (conflict.empty()) {
                    return false;
                }

                //find the maximum decision level

                size_t idx = 0;
                for (size_t i = 0; i < conflict.size(); i++) {
                    assert(var_to_trail[std::abs(conflict[i]) - 1].has_value());
                    idx = std::max(idx, trail[*var_to_trail[std::abs(conflict[i]) - 1]].decisionLevel);
                }

                //find the second largest decision level
                size_t t_idx = 0;
                for (size_t i = 0; i < conflict.size(); i++) {
                    assert(var_to_trail[std::abs(conflict[i]) - 1].has_value());
                    size_t decision = trail[*var_to_trail[std::abs(conflict[i]) - 1]].decisionLevel;
                    if (decision != idx) {
                        t_idx = std::max(t_idx, decision);
                    }
                }

                size_t backjumpTo = t_idx;
                // std::cout << "backjumping to: " << backjumpTo << std::endl;

                //backtracking
                while (!trail.empty() && trail.back().decisionLevel > backjumpTo) {
                    size_t decision_level = trail_starts.back();

                    while (trail.size() > decision_level) {
                        int lit = trail.back().lit;
                        vars[std::abs(lit) - 1] = UNASSIGNED;
                        var_to_trail[std::abs(lit) - 1] = std::nullopt;
                        trail.pop_back();
                    }

                    trail_starts.pop_back();
                }

                decisionLevel = backjumpTo;

                //if we have no more decisions levels or we backtrack to the
                //  root level (which MUST be True), then it's unsatisfiable
                if (trail_starts.empty()) {
                    return false;
                }

                qHead = trail_starts.back();

                clauses.push_back(std::move(conflict));
                clause_to_var.emplace_back();

                //since all the propagation (on this decision level)
                //  has been undone, we can now initialize watched literals
                //  for this clause
                // assert(createWatched(clauses.size() - 1, decisionLevel));
                if (!createWatched(clauses.size() - 1, decisionLevel)) {
                    assert(decisionLevel == 0);
                    return false;
                }

                continue;
            }
        }
    }
}