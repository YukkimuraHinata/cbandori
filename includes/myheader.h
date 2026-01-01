// ANSI转义序列（ANSI escape sequences）

// 前景色
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_Red "\x1b[31m"
#define ANSI_Green "\x1b[32m"
#define ANSI_Yellow "\x1b[33m"
#define ANSI_Blue "\x1b[34m"
#define ANSI_Magenta "\x1b[35m"
#define ANSI_Cyan "\x1b[36m"

// 背景色
#define ANSI_Black_BG "\x1b[40m"
#define ANSI_Red_BG "\x1b[41m"
#define ANSI_Green_BG "\x1b[42m"
#define ANSI_Yellow_BG "\x1b[43m"
#define ANSI_Blue_BG "\x1b[44m"
#define ANSI_Magenta_BG "\x1b[45m"
#define ANSI_Cyan_BG "\x1b[46m"

// 定义模拟次数，防止在1万到1亿之间
#define Simulations 1000000 //默认模拟次数
#define minSimulations 10000 //最低模拟次数
#define maxSimulations 100000000 //最多模拟次数

// 程序工具

// 解析参数
typedef struct Args ArgProcessing;
ArgProcessing arg_processing(int, const char**);

// 程序工作集与内存使用量
void print_peak_memory(void);

// 经典自己造轮子之max()
unsigned int max_u32(unsigned int, unsigned int);

// 计算累加和
long long accumulate(int, int, int[]);