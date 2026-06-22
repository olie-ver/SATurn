#include <testpp/testpp.hpp>
#include <filesystem>
#include <chrono>

#include "../src/SAT.hpp"

// TEST(Solve, headerless_uf20_91) {
//     for (const auto& path : std::filesystem::directory_iterator("tests/no_header/uf20-91")) {
//         SATurn::SATSolver solver(path.path().c_str());
//         // std::cout << path.path() << '\n';
//         ASSERT_TRUE(solver.solveCNF());
//         // solver.printSolution();
//     }
// }

// TEST(Solve, small_UUF50_218_1000) {
//     int idx = 0;
//     int max = 1;
//     for (const auto& path : std::filesystem::directory_iterator("tests/UUF50.218.1000")) {
//         if (idx < max) {
//             SATurn::SATSolver solver(path.path().c_str());

//             ASSERT_FALSE(solver.solveCNF());
//             idx++;
//         } else {
//             break;
//         }
//     }
// }

// TEST(Solve, unsat) {
//     SATurn::SATSolver solver("tests/header/unsat.cnf");
//     ASSERT_FALSE(solver.solveCNF());
// }

TEST(Solve, uf20_91) {
    for (const auto& path : std::filesystem::directory_iterator("tests/uf20-91")) {
        SATurn::SATSolver solver(path.path().c_str());
        // std::cout << path.path() << '\n';
        ASSERT_TRUE(solver.solveCNF());
        // solver.printSolution();
    }
}