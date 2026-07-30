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

// TEST(Solve, unsat) {
//     SATurn::SATSolver solver("tests/header/unsat.cnf");
//     ASSERT_FALSE(solver.solveCNF());
// }

// TEST(Solve, learn) {
//     SATurn::SATSolver solver("tests/header/learn.cnf");
//     // solver.solveCNF();
//     ASSERT_TRUE(solver.solveCNF());
//     // solver.printSolution();
// }

// TEST(Solve, empty_clause) {
//     SATurn::SATSolver solver("tests/header/empty_clause.cnf");
//     ASSERT_FALSE(solver.solveCNF());
// }

// TEST(Solve, UUF50_218_1000) {
//     int idx = 0;
//     int max = 1;
//     for (const auto& path : std::filesystem::directory_iterator("tests/UUF50.218.1000")) {
//         // if (idx == max) {
//         //     break;
//         // }
//         // idx++;
//         std::cout << path.path() << '\n';
//         SATurn::SATSolver solver(path.path().c_str());

//         ASSERT_FALSE(solver.solveCNF());
//     }
// }

TEST(Solve, uf20_91) {
    int idx = 0;
    int max = 1;
    
    // for (const auto& path : std::filesystem::directory_iterator("tests/uf20-91")) {
    //     // if (idx == max) {
    //     //     break;
    //     // }
    //     // idx++;
    //     SATurn::SATSolver solver(path.path().c_str());
    //     std::cout << path.path() << '\n';
    //     bool solved = solver.solveCNF();
    //     // solver.printSolution();
    //     ASSERT_TRUE(solved);
    //     // idx++;
    // }
    SATurn::SATSolver solver("tests/UUF50.218.1000/uuf50-0703.cnf");
    bool solved = solver.solveCNF();
    ASSERT_TRUE(solved);

    // SATurn::SATSolver solver("tests/uf20-91/uf20-0750.cnf");
    // ASSERT_TRUE(solver.solveCNF());
}

// TEST(watch_list, basic) {
//     SATurn::SATSolver solver("tests/header/learn.cnf");
//     solver.solveCNF();
// }