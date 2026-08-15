#include <testpp/testpp.hpp>
#include <filesystem>

#include "../src/SAT.hpp"

TEST(satisfiable, uf20_91) {
    int idx = 0;
    int max = 1;
    
    for (const auto& path : std::filesystem::directory_iterator("test_sets/uf20-91")) {
        // if (idx == max) {
        //     break;
        // }
        // idx++;
        // std::cout << path.path() << '\n';
        saturn::satsolver solver(path.path().c_str());
        bool solved = solver.solveCNF();
        ASSERT_TRUE(solved);
    }

    // saturn::satsolver solver("test_sets/uf20-91/uf20-0778.cnf");
    // bool solved = solver.solveCNF();
    // ASSERT_TRUE(solved);
}

TEST(unsatisfiable, UUF50_218_1000) {
    int idx = 0;
    int max = 1;
    
    for (const auto& path : std::filesystem::directory_iterator("test_sets/UUF50.218.1000")) {
        // if (idx == max) {
        //     break;
        // }
        // idx++;
        // std::cout << path.path() << '\n';
        saturn::satsolver solver(path.path().c_str());
        bool solved = solver.solveCNF();
        ASSERT_FALSE(solved);
    }
    // saturn::satsolver solver("test_sets/UUF50.218.1000/uuf50-0410.cnf");
    // bool solved = solver.solveCNF();
    // ASSERT_FALSE(solved);
}