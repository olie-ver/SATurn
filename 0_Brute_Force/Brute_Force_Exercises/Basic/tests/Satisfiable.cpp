#include <testpp/testpp.hpp>
#include <filesystem>

#include "../src/SAT.hpp"

TEST(satisfiable, uf20_91) {
    int idx = 0;
    int max = 5;
    
    for (const auto& path : std::filesystem::directory_iterator("tests/uf20-91")) {
        // if (idx == max) {
        //     break;
        // }
        // idx++;
        saturn::satsolver solver(path.path().c_str());
        std::cout << path.path() << '\n';
        bool solved = solver.solveCNF();
        ASSERT_TRUE(solved);
    }
}