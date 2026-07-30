#include "SAT.hpp"
#include <algorithm>
#include <iostream>

namespace SATurn {
    bool SATSolver::create_watched() {
        for (size_t i = 0; i < clauses.size(); i++) {
            const std::vector<int>& clause = clauses[i];

            if (clause.size() >= 2) {
                //clause i is being watched by the literals at indices 0 and 1
                clause_to_watch[i] = {0, 1};

                //literal 1 is watching clause i
                watch_to_clause[clause[0]].push_back(i);

                //literal 2 is watching clause i
                watch_to_clause[clause[1]].push_back(i);
            } else if (clause.size() == 1) {
                //clause[i] is being watched by only the literal at index 0
                clause_to_watch[i] = {0, 0};

                //literal 1 is watching clause i
                watch_to_clause[clause[0]].push_back(i);
            } else {
                //there are no literals to watch this clause with, so return false
                return false;
            }
        }
        return true;
    }

    bool SATSolver::create_watched(const std::vector<int>& clause) {
        if (clause.size() >= 2) {
            //the last clause must be watched by the first two literals
            //  that don't evaluate to false within the clause

            bool found_first = false;
            bool found_second = false;
            size_t first{};
            size_t second{};
            for (size_t i = 0; i < clause.size(); i++) {
                bool assign;
                if (assignment[std::abs(clause[i]) - 1] == Unassigned) {
                    assign = true;
                } else {
                    if (clause[i] > 0) {
                        assign = assignment[std::abs(clause[i]) - 1] == True;
                    } else {
                        assign = assignment[std::abs(clause[i]) - 1] == False;
                    }
                }

                if (assign) {
                    found_first = true;
                    first = i;
                    break;
                }
            }

            if (!found_first) {
                return false;
            }

            for (size_t i = first + 1; i < clause.size(); i++) {
                bool assign;
                if (assignment[std::abs(clause[i]) - 1] == Unassigned) {
                    assign = true;
                } else {
                    if (clause[i] > 0) {
                        assign = assignment[std::abs(clause[i]) - 1] == True;
                    } else {
                        assign = assignment[std::abs(clause[i]) - 1] == False;
                    }
                }

                if (assign) {
                    found_second = true;
                    second = i;
                    break;
                }
            }

            if (!found_second) {
                clause_to_watch[clauses.size() - 1] = {first, first};
                watch_to_clause[clause[first]].push_back(clauses.size() - 1);
            } else {
                clause_to_watch[clauses.size() - 1] = {first, second};
                watch_to_clause[clause[first]].push_back(clauses.size() - 1);
                watch_to_clause[clause[second]].push_back(clauses.size() - 1);
            }
        } else if (clause.size() == 1) {
            // bool assign = assignment[std::abs(clause[0]) - 1] == clause[0] > 0;

            bool assign;
            if (assignment[std::abs(clause[0]) - 1] == Unassigned) {
                assign = true;
            } else {
                if (clause[0] > 0) {
                    assign = assignment[std::abs(clause[0]) - 1] == True;
                } else {
                    assign = assignment[std::abs(clause[0]) - 1] == False;
                }
            }

            //(false and < 0) and (true and > 0) are the only valid watch literal states
            //  so if it's not that, then there are no valid watched literals
            //Unassigned = 0b11 => 1 for bools
            if (!assign) {
                return false;
            }

            //the last clause is being watched by only the literal at index 0
            clause_to_watch[clauses.size() - 1] = {0, 0};

            //literal 1 is watching the last clause
            watch_to_clause[clause[0]].push_back(clauses.size() - 1);
        } else {
            //there are no literals to watch this clause with, so return false
            return false;
        }

        return true;
    }
}