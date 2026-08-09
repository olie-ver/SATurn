#pragma once

#ifndef SAT_H
#define SAT_H

#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace SATurn {
    /// @brief A SAT solver
    class SATSolver {
        public:
            enum lit {
                False,
                True,
                Unassigned
            };
        
            /// @brief A unified constructor for a SATSolver
            /// @param param a string_view to either a file path or a CNF equation
            SATSolver(std::string_view param);

            /// @brief Gets the number of variables in the SAT solver
            /// @return the number of literals in the SAT solver
            const size_t getNumVars() const;

            /// @brief Gets the number of clauses in the SAT solver
            /// @return the number of clauses in the SAT solver
            const size_t getNumClauses() const;

            /// @brief Gets the clauses used in the equation
            /// @return A reference to the clauses field
            const std::vector<std::vector<int>>& getClauses() const;

            /// @brief Gets the solution, if solveCNF() was called
            /// @return A reference to the solution
            const std::vector<lit>& getSolution() const;

            /// @brief Prints out the solution, if the equation was attempted to be solved
            void printSolution();

            /// @brief Solves the given problem as if it was in CNF format
            /// @return true iff there's a solution, false otherwise
            bool solveCNF();

        private:  
            struct TrailEntry {
                int lit;
                //clause == std::nullopt => decision, else clause => reason clause index (implication)
                const std::optional<size_t> reason;
            };

            struct VarData {
                int trail_index = -1;
            };

            struct DecisionLevel {
                size_t level_start;
                bool tried_second_branch;
            };

            /// @brief parses the string_view into the SATSolver
            /// @param equation the cnf equation
            void parse(std::string_view equation);

            /// @brief initializes the map between literals and their relevant clauses
            /// @return true iff there are no empty clauses, false otherwise
            bool create_watched();

            /// @brief initializes a new clause's watched literals in a valid state
            /// @param clause the clause whose watched literals need to be initialized 
            /// @return true iff the clause can have watched literals, false otherwise
            bool create_watched(const std::vector<int>& clause);

            /// @brief propagates decisions
            // std::optional<std::vector<int>> propagate(int cur_lit);
            std::optional<std::vector<int>> propagate();

            /// @brief recursively assigns boolean values and evaluates the cnf equation
            /// @param assignment the current boolean assignment
            /// @return true if the current assignment is satisfiable, false otherwise
            bool solve(size_t level);

            bool try_solved = false;
            size_t numVars{};
            size_t numClauses{};

            size_t qhead{};

            //now a vector of literal assignments and decision levels
            std::vector<TrailEntry> trail{};
            std::vector<VarData> varData{};

            //a vector of indices where levelStarts[i] is the index of 
            // where a new decision level starts in trail
            std::vector<DecisionLevel> decisions{};

            std::vector<lit> assignment{};
            std::vector<std::vector<int>> clauses{};

            //a map from literals to a vector of indices of the clauses watching that literal
            std::unordered_map<int, std::vector<size_t>> watch_to_clause{};

            //a map from clause indices to their watched literal index pairs, good for fast lookup
            std::unordered_map<size_t, std::pair<size_t, size_t>> clause_to_watch{};

        //need to refactor solve() to become purely iterative
        //need to create a create_watched(size_t idx) overload to create the watched literals
        //  for a clause at the idx
        //add in a vector<size_t> levelStarts where levelStarts[i] is the index of where a new 
        //  decision level starts
        //Need a struct for the literals which is {lit value; Clause* reason; size_t level;}
        //Then we make a vector of these literal structs as a trail of decision levels
    };
}

#endif