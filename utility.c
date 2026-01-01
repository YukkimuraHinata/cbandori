#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include ".\includes\myheader.h"

void print_peak_memory() {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    printf("Peak working set: %lu KB\n", pmc.PeakWorkingSetSize / 1024);
    printf("Peak pagefile usage: %lu KB\n", pmc.PeakPagefileUsage / 1024);
}

unsigned int max_u32(unsigned int a, unsigned int b) {
    return a > b ? a : b;
}

// 计算累加和
long long accumulate(int begin, int end, int arr[]) {
    long long acc_result = 0;
    for(int i = begin; i < end; i++) {
        acc_result = arr[i] + acc_result;
    }
    return acc_result;
}

typedef struct Args{
    bool reverse_flag;      // -r 反推抽数
    bool unknow_arg;        // 未知参数
    bool need_to_exit;      // 表明程序需要退出（如遇到未知参数或遇到-v参数）
    int simulations;        // -n 指定模拟次数
    unsigned int threads;   // -t 指定线程
} ArgProcessing;

ArgProcessing arg_processing(int argc, const char* argv[]) {
    ArgProcessing Result = {
        .reverse_flag = false,
        .unknow_arg = false,
        .need_to_exit = false,
        .simulations = Simulations,
        .threads = 1
    };
    SYSTEM_INFO sysInfo;
    GetSystemInfo( &sysInfo );
    unsigned int thread_count = sysInfo.dwNumberOfProcessors;
    if (thread_count == 0) {
        thread_count = 4;
    }
    Result.threads = max_u32(1u,thread_count / 2u);
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (!strcmp(arg,"--reverse") || !strcmp(arg, "-r")) {
            Result.reverse_flag = true;
            printf(ANSI_Yellow_BG"当前处于反推抽数排名模式，结果仅供参考" ANSI_COLOR_RESET "\n");
            } else if (!strcmp(arg, "--thread") || !strcmp(arg, "-t")) {
                i++;
                int user_threads;
                user_threads = atoi(argv[i]);
                if(user_threads < 1) {
                    printf(ANSI_Red_BG"线程数必须大于0，将使用默认值" ANSI_COLOR_RESET "\n");
                } else if (user_threads > thread_count) {
                    printf(ANSI_Red_BG"警告：指定的线程数超过CPU线程，将使用%u线程" ANSI_COLOR_RESET "\n",thread_count);
                    Result.threads = thread_count;
                } else {
                    Result.threads = (unsigned int)user_threads;
                }
            } else if (!strcmp(arg, "--version") || !strcmp(arg, "-v")) {
                printf("\ncbandori,BanG Dream! Gacha in C,version 1.0.7 \n"
                    "GitHub page at: https://github.com/YukkimuraHinata/cbandori \n"
                    "C Version: %ld \n"
                    "Timestamp: %s \n\n",__STDC_VERSION__,__TIMESTAMP__);
                Result.need_to_exit = true;
                break;
            } else if (!strcmp(arg, "--number") || !strcmp(arg, "-n")) {
                i++;
                int tmpSimulations = atoi(argv[i]);
                if (tmpSimulations > minSimulations) {
                    if (tmpSimulations < maxSimulations) {
                        Result.simulations = tmpSimulations;
                    } else {
                        Result.simulations = maxSimulations;
                        printf(ANSI_Yellow_BG "为防止结果溢出，将使用最高允许值:%d" ANSI_COLOR_RESET "\n",maxSimulations);
                    }} else {
                        Result.simulations = minSimulations;
                        printf(ANSI_Yellow_BG "为保证精度，将使用最低允许值:%d" ANSI_COLOR_RESET "\n",minSimulations);
                    }
            } else if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) {
                printf("Options: \n"
                "  --reverse    -r    反推抽数排名\n"
                "  --number     -n    指定模拟次数，不应少于100万次\n"
                "  --thread     -t    指定使用的线程数，不多于CPU的线程\n"
                "  --version    -v    显示版本信息\n"
                "  --help       -h    显示帮助\n");
                Result.need_to_exit = true;
                break;
            } else {
                printf("未知参数: " ANSI_Red "%s \n" ANSI_COLOR_RESET,arg);
                puts("输入“cbandori -h”查看帮助");
                Result.unknow_arg = true;
                Result.need_to_exit = true;
                break;
            }
        }
    return Result;
}
