//
// Created by Cheng MingBo on 2023/2/12.
//
#include "Logger.h"
#include <chrono>
#include <iostream>
#include <vector>


int main() {
    auto start = std::chrono::steady_clock::now();
    auto& logger = Logger::GetInstance();
//    for (int i = 0; i < 100000; i++)
//        logger.Info("This is Info!!");
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&logger] {
            for (int i = 0; i < 100000; i++)
                logger.Info("This is a info!!");
        });
    }
    for (auto& x : threads) {
        x.join();
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "Elapsed time in milliseconds: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
         << " ms\n";
    return 0;
}