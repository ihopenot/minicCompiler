#include <ctime>
#include <string>
#include <cstdio>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ABS(x) ((x)>0?(x):-(x))
#define TEST_INIT(name) std::string fname = wd + #name; \
    std::string oname = owd + fname.substr(fname.find_last_of('/')+1); \
    std::string odata = oname + ".dat"; \
    std::string oout  = oname + ".out"; \
    FILE* f; \
    compile(fname, oname); 
#define RUN_TEST std::string cmdline = "cat " + odata + " | " + oname + " > " + oout;\
    system(cmdline.c_str());
#define OPEN_DATA f = fopen(odata.c_str(), "w");
#define CLOSE_DATA fclose(f);
#define OPEN_OUTPUT f = fopen(oout.c_str(), "r");
#define CLOSE_OUTPUT fclose(f);

std::string wd;
std::string owd = "./test/";

const float exp = 1e-5;
bool fcmp(float a, float b) {
    return ABS(a-b) < exp;
}

void compile(std::string file, std::string ofile) {
    std::string cmdline = "./compiler " + file + " -o " + ofile;
        printf("%s\n",cmdline.c_str());
    system(cmdline.c_str());
}

void calcTest() {
    printf("Calc Test:\n");
    int testcase = 200;
    TEST_INIT(calcTest);

    for(int i = 1; i<=testcase; i++) {
        printf("\rtest case\t\t%d/%d", i, testcase);

        OPEN_DATA;
        long x = rand(), y = rand();
        double a = rand() / 10000.0, b = rand() / 10000.0;
        fprintf(f, "%ld %ld\n", x, y);
        fprintf(f, "%lf %lf\n", a, b);
        CLOSE_DATA;

        RUN_TEST;

        OPEN_OUTPUT;
        long t; double ft;

        fscanf(f, "%ld", &t);
        assert(t == x+y);
        fscanf(f, "%ld", &t);
        assert(t == x-y);
        fscanf(f, "%ld", &t);
        assert(t == x*y);
        fscanf(f, "%ld", &t);
        assert(t == x/y);
        fscanf(f, "%ld", &t);
        assert(t == (x&y));
        fscanf(f, "%ld", &t);
        assert(t == (x|y));
        fscanf(f, "%ld", &t);
        assert(t == ((x+100)/5+2*y-1));
        fscanf(f, "%ld", &t);
        assert(t == ((x+y)&(x-12)|123+(x&2)*y));

        fscanf(f, "%lf", &ft);
        assert(fcmp(ft, a+b));
        fscanf(f, "%lf", &ft);
        assert(fcmp(ft, a-b));
        fscanf(f, "%lf", &ft);
        assert(fcmp(ft, a*b));
        fscanf(f, "%lf", &ft);
        assert(fcmp(ft, a/b));
        fscanf(f, "%lf", &ft);
        assert(fcmp(ft, a+b*a+b/3*2));
        CLOSE_OUTPUT;
    }
    printf("\npass\n");
}

void loopTest() {
    printf("Loop Test:\n");
    TEST_INIT(loopTest);

    int testcase = 100;
    for(int i=1; i<=testcase; i++) {
        printf("\rtest case\t\t%d/%d", i, testcase);

        OPEN_DATA;
        int l = rand() % 100, r = rand() % 100;
        if(l > r) l^=r^=l^=r;
        fprintf(f, "%d %d\n", l, r);
        int w = rand() % 20 + 1, h = rand() % 20 + 1;
        fprintf(f, "%d %d\n", w, h);
        CLOSE_DATA;

        RUN_TEST;

        OPEN_OUTPUT;
        for(int t=0, p; t<2; t++)
        for(int i=l; i<=r; i++) {
            fscanf(f, "%d", &p);
            assert(p == i);
        }
        for(int t=0, p; t<2; t++)
        for(int i=1; i<=h; i++) {
            fscanf(f, "%d", &p);
            assert(p == i);
            for(int j=1; j<=w; j++) {
                fscanf(f, "%d", &p);
                assert(p == j);
            }
        }
        CLOSE_OUTPUT;
    }
    printf("\npass\n");
}

void ifTest() {
    printf("If Test:\n");
    TEST_INIT(ifTest);

    int testcase = 1000;
    for(int i=1; i<=testcase; i++) {
        printf("\rtest case\t\t%d/%d", i, testcase);

        OPEN_DATA;
        int a = rand()%100;
        fprintf(f, "%d\n", a);
        CLOSE_DATA;

        RUN_TEST;

        OPEN_OUTPUT;
        int t;
        fscanf(f, "%d", &t);
        if(a >= 50) {
            if(a >=75) a = 4;
            else a = 3;
        }
        else if(a >= 30) a = 2;
        else if(a >= 20) a = 1;
        else a = 0;
        assert(t == a);
        CLOSE_OUTPUT;
    }
    printf("\npass\n");
}

void arrayTest() {
    printf("Array Test:\n");
    TEST_INIT(arrayTest);

    int testcase = 100;
    for(int i=1; i<=testcase; i++) {
        printf("\rtest case\t\t%d/%d", i, testcase);

        OPEN_DATA;
        int a[100];
        int x = rand()%100 + 1;
        fprintf(f, "%d\n", x);
        for(int i=0; i<x; i++) {
            a[i] = rand();
            fprintf(f, "%d ", a[i]);
        }
        int w = rand() % 20 +1, h = rand() % 20 +1;
        int b[h][w];
        fprintf(f, "%d %d\n", w, h);
        for(int i = 0; i<h; i++)
            for(int j=0; j<w; j++) {
                b[i][j] = rand();
                fprintf(f, "%d ", b[i][j]);
            }
        CLOSE_DATA;

        RUN_TEST;

        OPEN_OUTPUT;
        for(int i=0, p; i<x; i++) {
            fscanf(f, "%d", &p);
            assert(p == a[i]);
        }
        for(int i=0, p; i<h; i++) {
            for(int j=0; j<w; j++) {
                fscanf(f, "%d", &p);
                assert(p == b[i][j]);
            }
        }
        CLOSE_OUTPUT;
    }
    printf("\npass\n");
}

int fib(int x) {
    if(x<2) return 1;
    return fib(x-1) + fib(x-2);
}

void fibTest() {
    printf("fibTest\n");
    TEST_INIT(fibTest);

    int testcase = 100;
    for(int i=1; i<=testcase; i++) {
        printf("\rtest case\t\t%d/%d", i, testcase);

        OPEN_DATA;
        int x = rand() % 30;
        fprintf(f, "%d", x);
        CLOSE_DATA;

        RUN_TEST;

        OPEN_OUTPUT;
        int y = fib(x);
        fscanf(f, "%d", &x);
        assert(x == y);
        CLOSE_OUTPUT;
    }
    printf("\npass\n");
}

int main(int argc, char** argv) {
    wd = std::string(argv[1]);
    mkdir(owd.c_str(), 00777);
    srand(time(0));

    calcTest();
    loopTest();
    ifTest();
    arrayTest();
    fibTest();

    return 0;
}