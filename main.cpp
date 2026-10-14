// g++ -std=c++17 -O2 -fopenmp main.cpp -o main.exe
// .\main.exe                                      

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <omp.h>

double PI = 3.14159265358979323846;
int maxIter = 50;
long long num_steps = 100000000;
const int num_threads = 4;
double global_sum[num_threads];
double step = 1.0 / static_cast<double>(num_steps);

// Последовательный алгоритм
double SequentialAlg() {
    double sum = 0.0, x = 0.0;
    for (long long i = 0; i < num_steps; ++i) {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
        //std::cout << "Sum: " << sum << std::endl;
    }
    return sum * step;
}

// Параллельный алгоритм
void ParallelAlgThread( int thread_num) {
    double x, sum = 0.0;
    for (long long i = thread_num; i < num_steps; i += num_threads) {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    global_sum[thread_num] = sum;
}


double ParallelAlgOpenMP() {
    double x, sum = 0.0;
    #pragma omp parallel num_threads(num_threads) private(x)
    #pragma omp for schedule(static) reduction(+:sum)
    for (long long i = 0; i < num_steps; ++i) {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    return sum * step;
}

int main() {

    std::chrono::microseconds sumTumeSeqAlg{0};
    std::chrono::microseconds sumTumeParAlg{0};
    std::chrono::microseconds sumTumeParAlgOmp{0};

    double errorSeqAlg = 0.0;
    double errorParAlg = 0.0;
    double errorParAlgOmp = 0.0;

    for ( int inter = 0; inter < maxIter; ++inter) {
        std::cout << "Iteration: " << inter + 1 << std::endl;

        std::cout << "----------------------------------------" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        double result = SequentialAlg();
        std::cout << "Result sequential alg: " << std::setprecision(10) << result << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Time sequential alg: " << duration.count() << " ms" << std::endl;
        std::cout << "Error sequential alg: " << std::setprecision(10) << std::abs(PI - result) << std::endl;
        sumTumeSeqAlg += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        errorSeqAlg += std::abs(PI - result);
        std::cout << "----------------------------------------" << std::endl;

        
        start = std::chrono::high_resolution_clock::now();
        std::thread threads[num_threads] = {
            std::thread(ParallelAlgThread, 0),
            std::thread(ParallelAlgThread, 1),
            std::thread(ParallelAlgThread, 2),
            std::thread(ParallelAlgThread, 3)
        };
        double paralPi = 0.0;
        for (long long i = 0; i < num_threads; ++i) {
            threads[i].join();
            paralPi += global_sum[i] * step;
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Result parallel alg Thread: " << std::setprecision(10) << paralPi << std::endl;
        std::cout << "Time parallel alg Thread: " << duration.count() << " ms" << std::endl;
        std::cout << "Error parallel alg Thread: " << std::setprecision(10) << std::abs(PI - paralPi) << std::endl;
        sumTumeParAlg += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        errorParAlg += std::abs(PI - paralPi);
        std::cout << "----------------------------------------" << std::endl;


        start = std::chrono::high_resolution_clock::now();
        double resultOmp = ParallelAlgOpenMP();
        std::cout << "Result parallel alg OpenMP: " << std::setprecision(10) << resultOmp << std::endl;
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Time parallel alg OpenMP: " << duration.count() << " ms" << std::endl;
        std::cout << "Error parallel alg OpenMP: " << std::setprecision(10) << std::abs(PI - resultOmp) << std::endl;
        sumTumeParAlgOmp += std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        errorParAlgOmp += std::abs(PI - resultOmp);

        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Max iterations: " << maxIter << std::endl;
    std::cout << "Average time sequential alg: " << sumTumeSeqAlg.count() / maxIter << " ms" << std::endl;
    std::cout << "Average time parallel alg Thread: " << sumTumeParAlg.count() / maxIter << " ms" << std::endl;
    std::cout << "Average time parallel alg OpenMP: " << sumTumeParAlgOmp.count() / maxIter << " ms" << std::endl;

    std::cout << "Average error sequential alg: " << errorSeqAlg / maxIter << std::endl;
    std::cout << "Average error parallel alg Thread: " << errorParAlg / maxIter << std::endl;
    std::cout << "Average error parallel alg OpenMP: " << errorParAlgOmp / maxIter << std::endl;
    return 0;
}