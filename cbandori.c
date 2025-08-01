#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <pthread.h>
#include ".\includes\myset.h"
#include ".\includes\myheader.h"

#define Simulations 1000000 //默认模拟次数
#define minSimulations 10000 //最低模拟次数

// 获取0-1之间的随机数
double get_random() {
    double num_rand = (double)rand()/RAND_MAX;
    return num_rand;
}

// 获取5星卡的随机编号[1 , total_5star]
int get_5star_random(int total_5star) {
    int dis_5star = rand() % total_5star + 1;
    return dis_5star;
}

/* 到现在还记得教我C语言的老师讲qsort()的时候
“你们不用管为什么是这样，把它背过了就行” */
int cmpfunc(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

typedef struct {
    bool reverse_flag;      // -r 反推抽数
    bool unknow_arg;        // 未知参数
    bool need_to_exit;      // 表明程序需要退出（如遇到未知参数或遇到-v参数）
    int simulations;        // -n 指定模拟次数
    unsigned int threads;   // -t 指定线程
} ArgProcessing;

// 定义线程参数结构体
typedef struct {
    int total_5star;        // 总共的五星卡数量
    int want_5star;         // 想要的五星卡数量
    int total_4star;        // 总共的四星卡数量
    int want_4star;         // 想要的4星卡数量
    int is_normal;          // 是否是常驻
    int start_index;        // 起始索引,还用于memmove()
    int end_index;          // 结束索引
    int* array;             // 输出数组
    pthread_mutex_t* mutex; // 互斥锁指针
} ThreadArgs;

ArgProcessing arg_processing(int, const char**);
unsigned int max_u32(unsigned int, unsigned int);

void* simulate_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int sims_this_thread = args->end_index - args->start_index;
    int *local_draws = calloc(sims_this_thread, sizeof(int));
    if (!local_draws) {
        return NULL;
    }
    int total_4star = args->total_4star;
    int want_5star = args->want_5star;
    int want_4star = args->want_4star;
    int normal = args->is_normal;
    int start = args->start_index;
    int end = args->end_index;
    int  draws;             // 已经抽卡次数
    int choose_times_have;  // 拥有的自选次数
    for (int i = 0; i < sims_this_thread; i++) {
        set *cards_5star = set_create();  // 已拥有的5星卡片集合
        set *cards_4star = set_create();  // 已拥有的4星卡片集合
        draws = 0;  // 已经抽卡次数
        choose_times_have = 0; // 拥有的自选次数

        while (true) {
            draws++;
            // 修改50保底逻辑
            if (draws % 50 == 0 && normal == 1) {
                if (want_5star > 0) {  // 只有当我们想要5星卡时才考虑
                    int roll = get_5star_random(want_5star);  // 随机抽取一张5星卡
                    if (roll <= want_5star) {  // 如果抽到的是想要的卡
                        set_addElement(cards_5star,roll);
                    }
                }
                goto next_draw;  // 50保底必定是5星，跳过4星判定
            }
            else {
                // 正常抽卡
                double rand = get_random();
                // 先判断5星
                if (want_5star > 0) {  // 只有当我们想要5星卡时才判定
                    for (int i = 1; i <= want_5star; i++) {
                        if (rand < 0.005 * i) {  // 0.005 = 0.5%，单张pickup的概率
                            set_addElement(cards_5star,i);
                            goto next_draw;
                        }
                    }
                }

                // 如果想要的5星数量为0，则必定想要4星
                // 看似多此一举的一个else分支，可以为单线程100万次模拟减短大约一秒的计算时间
                else {
                    rand = get_random();
                    for (int i = 1; i <= want_4star; i++) {
                        if(rand < 0.0075 * i) {  // 四星的概率是0.75%
                            set_addElement(cards_4star,i);
                            break;
                        }
                    }
                    goto next_draw;  // 跳过重复的4星判定
                }

                // 再判断4星
                if (total_4star > 0 && want_4star > 0) {
                    rand = get_random();
                    for (int i = 1; i <= want_4star; i++) {
                        if(rand < 0.0075 * i) {
                            set_addElement(cards_4star,i);
                            break;
                        }
                    }
                }
            }
            next_draw:

            // 先把抽卡抽完，再进行自选，避免重复抽到
            choose_times_have = ( draws + 100 ) / 300;
            if(set_getSize(cards_5star) + set_getSize(cards_4star) + choose_times_have >= want_5star + want_4star) {
                break; 
            }
        }
        local_draws[i] = draws;
        set_destroy(cards_4star);
        set_destroy(cards_5star);
    }
    // 使用互斥锁安全更新
    pthread_mutex_lock(args->mutex);
    memmove(&(args->array[start]),local_draws,sims_this_thread * sizeof(int));
    free(local_draws);
    pthread_mutex_unlock(args->mutex);
    return NULL;
}

// 计算累加和
long long accumulate(int begin, int end, int arr[]) {
    long long acc_result = 0;
    for(int i = begin; i < end; i++) {
        acc_result = arr[i] + acc_result;
    }
    return acc_result;
}

// 开启多线程，输出最终结果
int calculate_statistics(int total_5star, int want_5star, int total_4star, int want_4star, int normal, 
                        int simulations, unsigned int thread_count, bool reverseFlag) {
    LARGE_INTEGER freq, start_time, end_time;
    QueryPerformanceFrequency(&freq);   // 获取计数器频率
    QueryPerformanceCounter(&start_time);
    
    puts("----------------输入参数----------------");
    printf("当期5星数量: %d \n",total_5star);
    printf("想要的当期5星数量: %d \n",want_5star);
    printf("想要的当期4星数量: %d \n",want_4star);
    if(normal) {
        puts("有50抽小保底");
    } else {
        puts("无50抽小保底");
    }
    printf("模拟次数: %d \n",simulations);
    printf("使用线程数: %u \n",thread_count);

    // 初始化存放每次抽卡次数的数组
    int *draw_counts = calloc(simulations, sizeof(int));
    if(!draw_counts) {
        return 2;
    }
    // 初始化互斥锁
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    // 使用用户指定的线程数
    pthread_t threads[thread_count];
    ThreadArgs thread_args[thread_count];
    // 每线程的模拟次数
    int sims_per_thread = simulations / thread_count;
    // 创建多个线程执行模拟
    for (unsigned int i = 0; i < thread_count; i++) {
        thread_args[i].is_normal = normal;
        thread_args[i].total_4star = total_4star;
        thread_args[i].total_5star = total_5star;
        thread_args[i].want_4star = want_4star;
        thread_args[i].want_5star = want_5star;
        thread_args[i].array = draw_counts;
        thread_args[i].start_index = i * sims_per_thread;
        thread_args[i].end_index = (i == thread_count- 1u) ? simulations : (i + 1u) * sims_per_thread;
        thread_args[i].mutex = &mutex;

        if (pthread_create(&threads[i], NULL, simulate_thread, &thread_args[i]) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    // 等待所有线程完成
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);

    // 计算总抽数
    long long total_draws = 0;
    total_draws = accumulate(0, simulations, draw_counts);
    
    double expected_draws = (double)total_draws / simulations;
    qsort(draw_counts, simulations, sizeof(int), cmpfunc);
    int percentile_50 = draw_counts[simulations / 2];
    int percentile_90 = draw_counts[(int)(simulations * 0.9)];
    int max_number = draw_counts[simulations - 1];
    
    printf("----------------模拟结果---------------- \n");
    printf("期望抽卡次数: " ANSI_Cyan "%lf \n"ANSI_COLOR_RESET ,expected_draws);
    printf("中位数抽卡次数: %d ，即 %.2lf w星石 \n",percentile_50 ,(double)percentile_50 /40.0);
    printf("90%%玩家在以下抽数内集齐: %d ，即 %.2lf w星石 \n",percentile_90 ,(double)percentile_90 /40.0);
    printf("非酋至多抽卡次数: %d ，即 %.2lf w星石 \n",max_number ,(double)max_number /40.0);
    if (!normal) {
    printf("理论最多抽卡次数：%d \n" ,(want_4star + want_5star) * 300 - 100);
    }
    // 结束计时并计算耗时
    QueryPerformanceCounter(&end_time);
    double time_diff = (double)(end_time.QuadPart - start_time.QuadPart) / freq.QuadPart;
    printf("模拟耗时: %.3lf 秒 \n",time_diff);

    free(draw_counts);

    if(reverseFlag) {
        double input, sigma, z, cdfValue;
        printf("请输入所使用的抽数：");
        scanf("%lf" ,&input);
        if(input < 10.0) {
            puts("输入值过小！");
            return -1;
        } else if(input > max_number) {
            puts("超级非酋:( ");
            return 0;
        } else {
            sigma = (percentile_90 - expected_draws)/1.28155;
            z = (input - expected_draws)/sigma;
            cdfValue = 0.5 * (1 + erf(z / 1.41421)); //sqrt(2)
            printf("输入值 %.0lf 对应累积概率约为 "ANSI_Cyan "%lf %% \n" ANSI_COLOR_RESET, input, cdfValue * 100.0);
        }
    }
    return 0;
}

inline unsigned int max_u32(unsigned int a, unsigned int b) {
    return a > b ? a : b;
}

inline ArgProcessing arg_processing(int argc, const char* argv[]) {
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
    for (int i = 1; i < argc; ++i) {
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
                printf("\ncbandori,BanG Dream! Gacha in C,version 1.0.4,Build 28 \n"
                    "GitHub page at: https://github.com/YukkimuraHinata/cbanduori \n"
                    "C Version: %ld \n"
                    "Timestamp: %s \n\n",__STDC_VERSION__,__TIMESTAMP__);
                Result.need_to_exit = true;
            } else if (!strcmp(arg, "--number") || !strcmp(arg, "-n")) {
                i++;
                int tmpSimulations = atoi(argv[i]);
                if(tmpSimulations > minSimulations){
                    Result.simulations = tmpSimulations;
                    } else {
                        Result.simulations = minSimulations;
                       printf(ANSI_Yellow_BG"为保证精度，将使用最低允许值:%d"ANSI_COLOR_RESET "\n",minSimulations);
                    }
            } else if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) {
                printf("Options: \n"
                "  --reverse    -r    反推抽数排名\n"
                "  --number     -n    指定模拟次数，不应少于100万次\n"
                "  --thread     -t    指定使用的线程数，不多于CPU的线程\n"
                "  --version    -v    显示版本信息\n"
                "  --help       -h    显示帮助\n");
                Result.need_to_exit = true;
            } else {
                printf("未知参数: " ANSI_Red "%s \n"ANSI_COLOR_RESET,arg);
                puts("输入“cbandori -h”查看帮助");
                Result.unknow_arg = true;
                Result.need_to_exit = true;
                //break;
            }
        }
    return Result;
}

int main(int argc, const char* argv[]) {
    ArgProcessing res = arg_processing(argc, argv);
    if(res.need_to_exit) {
        return res.unknow_arg;
    }
    int isNormal = 1;
    int total_5star = 0, want_5star = 0, total_4star = 0, want_4star = 0;
    printf(ANSI_Blue_BG"cbandori,a gacha simulator of Garupa" ANSI_COLOR_RESET "\n");
    printf("请输入当期5星卡的总数量: ");
    scanf("%d",&total_5star);
    // Fes池6%概率最多12张卡，在这里确保输入合法，防止set数组越界
    if (total_5star < 0 || total_5star > 12) {
        printf("卡片数量必须大于等于0且不超过12！");
        return 1;
    }
    if (total_5star > 0) {
        printf("请输入想要抽取的当期5星卡数量: ");
        scanf("%d",&want_5star);
        if(want_5star < 0 || want_5star > total_5star) {
            printf("想要的卡片数量必须在0到 %d 之间！\n",total_5star);
            return 1;
        }
    }
    // 四星卡同理
    printf("请输入想要抽取的当期4星卡数量: ");
    scanf("%d",&want_4star);
    if (want_4star < 0 || want_4star > 12) {
        printf("卡片数量必须大于0且不超过12！");
        return 1;
    }
    total_4star = want_4star;
    printf("是否为常驻池（是否有50小保底）（1: 是，0: 否）: ");
    scanf("%d",&isNormal);
    if (isNormal != 0 && isNormal != 1) {
        printf("输入错误！（1: 是，0: 否）");
        return 1;
    }
    // 初始化种子
    srand((unsigned)time(NULL));

    int return_value = calculate_statistics(total_5star, want_5star, total_4star, want_4star, isNormal, 
                        res.simulations, res.threads, res.reverse_flag);
    return return_value;
}
