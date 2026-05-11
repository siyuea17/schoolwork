#include <iostream>
#include <omp.h>
#include <ctime>
#include <cstdlib>
#include <random>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    const long long total_points = 10000000;  // 总点数
    long long count_in_circle = 0;            // 圆内点数

    // 记录开始时间
    double start_time = omp_get_wtime();

#pragma omp parallel reduction(+:count_in_circle)
    {
        // 使用线程ID和时间混合种子，避免 random_device 开销
        int thread_id = omp_get_thread_num();
        unsigned seed = static_cast<unsigned>(time(nullptr) ^ (thread_id << 16));
        std::mt19937 gen(seed);
        std::uniform_real_distribution<double> dist(-1.0, 1.0);

#pragma omp for
        for (long long i = 0; i < total_points; i++) {
            double x = dist(gen);
            double y = dist(gen);
            // 直接判断，避免创建 point 对象
            if (x * x + y * y < 1.0) {
                count_in_circle++;
            }
        }
    }

    // 记录结束时间
    double end_time = omp_get_wtime();

    // 计算π值
    double pi = 4.0 * count_in_circle / total_points;

    std::cout << "总点数: " << total_points << std::endl;
    std::cout << "圆内点数: " << count_in_circle << std::endl;
    std::cout << "估计的π值: " << pi << std::endl;
    std::cout << "真实的π值: " << M_PI << std::endl;
    std::cout << "绝对误差: " << std::abs(pi - M_PI) << std::endl;
    std::cout << "相对误差: " << std::abs(pi - M_PI) / M_PI * 100.0 << "%" << std::endl;
    std::cout << "计算时间: " << (end_time - start_time) << " 秒" << std::endl;

    // 测试不同线程数的性能
    std::cout << "\n测试不同线程数的性能:" << std::endl;

    double single_thread_time = 0.0;

    for (int num_threads = 1; num_threads <= 8; num_threads *= 2) {
        omp_set_num_threads(num_threads);

        double start = omp_get_wtime();
        long long local_count = 0;

#pragma omp parallel reduction(+:local_count)
        {
            int thread_id = omp_get_thread_num();
            unsigned seed = static_cast<unsigned>(time(nullptr) ^ (thread_id << 16) ^ (num_threads << 8));
            std::mt19937 gen(seed);
            std::uniform_real_distribution<double> dist(-1.0, 1.0);

#pragma omp for
            for (long long i = 0; i < total_points; i++) {
                double x = dist(gen);
                double y = dist(gen);
                if (x * x + y * y < 1.0) {
                    local_count++;
                }
            }
        }

        double end = omp_get_wtime();
        double duration = end - start;
        double pi_est = 4.0 * local_count / total_points;
        double speedup = (num_threads == 1) ? 1.0 : (single_thread_time / duration);

        if (num_threads == 1) {
            single_thread_time = duration;
        }

        std::cout << "线程数: " << num_threads
            << ", 时间: " << duration << " 秒"
            << ", π估计: " << pi_est
            << ", 加速比: " << speedup << std::endl;
    }

    return 0;
}