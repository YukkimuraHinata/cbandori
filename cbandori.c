#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include ".\includes\myset.h"
#include ".\includes\myheader.h"
#include <time.h>
#include <windows.h>
//#include <pthread.h>
// MinGW好像没提供threads.h

//#define true 1
//#define false 0
#define Simulations 1000000 //默认模拟次数
#define minSimulations 10000 //最低模拟次数

//初始化种子
void rand_init() {
    srand((unsigned)time(NULL));
}

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

typedef struct ArgProcessing {
    bool reverse_flag;
    bool unknow_arg;
    bool need_to_exit;
    int simulations;
    unsigned int threads;
} ArgProcessing;

ArgProcessing arg_processing(int, const char**);
unsigned int max_u32(unsigned int, unsigned int);

int simulate_one_round(int total_5star, int want_5star, int total_4star, int want_4star, int normal) {
    set *cards_5star = set_create();  // 已拥有的5星卡片集合
    set *cards_4star = set_create();  // 已拥有的4星卡片集合
    int draws = 0;  // 已经抽卡次数
    int choose_times_have = 0; // 拥有的自选次数

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
            //看似多此一举的一个else分支，可以为单线程100万次模拟减短大约一秒的计算时间（自豪）
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

        //新的自选逻辑，先把抽卡抽完，再进行自选，避免重复抽到
        choose_times_have = ( draws + 100 ) / 300;
        if(set_getSize(cards_5star) + set_getSize(cards_4star) + choose_times_have >= want_5star + want_4star) {
            break; 
        }
    }
    set_destroy(cards_4star);
    set_destroy(cards_5star);
    return draws;
}

//计算累加和
long long accumulate(int begin, int end, int arr[]) {
    long long acc_result = 0;
    for(int i = begin; i < end; i++) {
        acc_result = arr[i] + acc_result;
    }
    return acc_result;
}

// 修改统计函数签名，添加线程数参数
int calculate_statistics(int total_5star, int want_5star, int total_4star, int want_4star, int normal, 
                        int simulations, unsigned int thread_count, bool reverseFlag) {
    time_t start_time = time(&start_time);
    
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
    int *draw_counts = calloc(simulations, sizeof(int));
    if(!draw_counts) {
        return 2;
    }

    //std::mutex mtx;
    long long total_draws = 0;
    for(int it = 0; it < simulations; it++) {
        draw_counts[it] = simulate_one_round(total_5star, want_5star, total_4star, want_4star, normal);
    }
    
    // 使用用户指定的线程数
    //std::vector<std::thread> threads;
    //int sims_per_thread = simulations / thread_count;
        
    // 创建多个线程执行模拟
    /* 先不管多线程，能跑起来再说
    for (unsigned int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&, i]() {
            GachaRandom random(total_5star);  // 每个线程使用独立的随机数生成器
            std::vector<int> local_draws;     // 线程本地存储
            local_draws.reserve(sims_per_thread);
            
            int start = i * sims_per_thread;
            int end = (i == thread_count - 1) ? simulations : (i + 1) * sims_per_thread;
            
            for (int j = start; j < end; ++j) {
                int draws = simulate_one_round(total_5star, want_5star, total_4star, want_4star, normal, random);
                local_draws.push_back(draws);
            }
            
            // 合并结果
            std::lock_guard<std::mutex> lock(mtx);
            draw_counts.insert(draw_counts.end(), local_draws.begin(), local_draws.end());
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    */
    // 计算总抽数
    total_draws = accumulate(0, simulations, draw_counts);
    
    double expected_draws = (double)total_draws / simulations;
    qsort(draw_counts, simulations, sizeof(int), cmpfunc);
    int percentile_50 = draw_counts[simulations / 2];
    int percentile_90 = draw_counts[(int)(simulations * 0.9)];
    int max_number = draw_counts[simulations - 1];
    
    printf("----------------模拟结果---------------- \n");
    printf("期望抽卡次数: " ANSI_Cyan "%lf \n"ANSI_COLOR_RESET ,expected_draws);
    printf("中位数抽卡次数: %d ，即 %.2lf w星石 \n",percentile_50 ,(double)percentile_50 /40.0);
    printf("90玩家在以下抽数内集齐: %d ，即 %.2lf w星石 \n",percentile_90 ,(double)percentile_90 /40.0);
    printf("非酋至多抽卡次数: %d ，即 %.2lf w星石 \n",max_number ,(double)max_number /40.0);
    if (!normal) {
    printf("理论最多抽卡次数：%d \n" ,(want_4star + want_5star) * 300 - 100);
    }
    // 结束计时并计算耗时
    time_t end_time = time(&end_time);
    double time_diff = difftime(end_time,start_time);

    printf("模拟耗时: %.2lf 秒",time_diff);
    /* for(int j = 0;j < simulations; j++) {
        printf("%d ",draw_counts[j]);
    } */
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
                printf("\ncbandori,BanG Dream! Gacha in C,version 1.0.2,Build 18 \n"
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
    rand_init();
    int return_value = calculate_statistics(total_5star, want_5star, total_4star, want_4star, isNormal, 
                        res.simulations, res.threads, res.reverse_flag);
    return return_value;
}
