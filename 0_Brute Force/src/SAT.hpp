#pragma once

#ifndef SAT_H
#define SAT_H

#include <string>
#include <string_view>
#include <optional>
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

            /// @brief Gets the clauses used in the equation
            /// @return A reference to the clauses field
            const std::vector<std::vector<int>>& getClauses() const;

            /// @brief Gets the solution, if solveCNF() was called
            /// @return A reference to the solution
            const std::optional<std::vector<bool>>& getSolution() const;

            /// @brief Prints out the solution, if the equation was attempted to be solved
            void printSolution();

            /// @brief Solves the given problem as if it was in CNF format
            /// @return true iff there's a solution, false otherwise
            bool solveCNF();

        private:
            /// @brief parses the string_view into the SATSolver
            /// @param equation the cnf equation
            void parse(std::string_view equation);

            size_t numVars{};
            size_t numClauses{};
            std::optional<std::vector<bool>> lits{};
            std::vector<std::vector<int>> clauses{};
    };
}

#endif