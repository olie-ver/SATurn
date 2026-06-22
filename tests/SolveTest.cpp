#include <testpp/testpp.hpp>
#include <filesystem>
#include <chrono>

#include "../src/SAT.hpp"

TEST(Solve, headerless_uf20_91) {
    for (const auto& path : std::filesystem::directory_iterator("tests/no_header/uf20-91")) {
        SATurn::SATSolver solver(path.path().c_str());
        // std::cout << path.path() << '\n';
        ASSERT_TRUE(solver.solveCNF());
        // solver.printSolution();
    }
}

TEST(Solve, uf20_91) {
    const auto start = std::chrono::steady_clock::now();
    for (const auto& path : std::filesystem::directory_iterator("tests/uf20-91")) {
        SATurn::SATSolver solver(path.path().c_str());
        // std::cout << path.path() << '\n';
        ASSERT_TRUE(solver.solveCNF());
        // solver.printSolution();
    }
    const auto end = std::chrono::steady_clock::now();
    const auto diff = end - start;
    std::cout << diff << std::endl;
}