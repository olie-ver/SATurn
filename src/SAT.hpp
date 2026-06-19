#pragma once

#ifndef SAT_H
#define SAT_H

#include <string>
#include <string_view>
#include <vector>

namespace SATurn {
    /// @brief A SAT solver
    class SATSolver {
        public:
            /// @brief A unified constructor for a SATSolver
            /// @param param a string_view to either a file path or a CNF equation
            SATSolver(std::string_view param);

            /// @brief Gets the number of variables in the SAT solver
            /// @return the number of literals in the SAT solver
            const size_t getNumVars() const;

            /// @brief Gets the number of clauses in the SAT solver
            /// @return the number of clauses in the SAT solver
            const size_t getNumClauses() const;

            const std::vector<std::vector<int>>& getClauses() const;

            /// @brief Solves the given problem as if it was in CNF format
            /// @return true iff there's a solution, false otherwise
            bool solveCNF();

            /// @brief Solves the given problem as if it was in DNF format
            /// @return true iff there's a solution, false otherwise
            bool solveDNF();

        private:
            /// @brief parses the string_view into the SATSolver
            /// @param equation the cnf equation
            void parse(std::string_view equation);

            size_t numVars;
            size_t numClauses;
            std::vector<bool> lits;
            std::vector<std::vector<int>> clauses;
    };
}

#endif