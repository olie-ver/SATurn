#include "SAT.hpp"

#include <algorithm>
#include <thread>

//for each thread, the range they have to cover is 2^numVars / numThreads
#define interval (0b1 << numVars) / numThreads

namespace saturn {
    bool satsolver::solveCNF() {
        size_t numThreads = std::thread::hardware_concurrency();

        std::vector<std::thread> threads;

        numThreads = std::max(1ul, numThreads); //forces there to be at least one thread

        //creates numThreads - 1 NEW threads 
        //(we want this current running thread to do work as well)
        for (size_t i = 1; i < numThreads; i++) {
            threads.emplace_back(threadWorker, i * interval + 1, (i + 1) * interval);
        }

        //if numThreads = 1, then this main thread does all the work
        threadWorker(0, interval);

        for (auto& thread : threads) {
            thread.join();
        }

        //if no thread finds a satisfiable assignment, vars will hold nothing
        //  therefore whether or not we've solved the equation is equivalent
        //  to vars having a value.
        return vars.has_value();
    }

    void satsolver::threadWorker(size_t low, size_t high) {
        for (size_t i = low; i < high && !stop; i++) {
            bool solved = true;
            for (size_t j = 0; j < numClauses; j++) {
                const std::vector<int>& clause = clauses[j];

                bool satisfied_clause = false;
                for (size_t k = 0; k < clause.size(); k++) {
                    if (clause[k] > 0) {
                        satisfied_clause |= i & (0b1 << numVars);
                    } else {
                        satisfied_clause |= !(i & (0b1 << numVars));
                    }
                }

                solved &= satisfied_clause;

                if (!solved) {
                    break;
                }
            }

            if (solved) {
                const std::lock_guard<std::mutex> lock(vars_mutex);

                //we change the atomic bool's value, so any other thread
                // running the evaluation loop will check the conditional
                // and see that it's now invalid, so they will terminate
                stop = true;
                (*vars).reserve(numVars);
                
                for (size_t j = 0; j < numVars; j++) {
                    (*vars)[j] = i & (0b1 << j);
                }
            }
        }
    }
}