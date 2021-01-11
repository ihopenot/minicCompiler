#include <cstdio>
long a[100];

int main() {
    long x;
    scanf("%ld", &x);
    for(int i=0; i<x; i = i+1) 
        scanf("%ld", &a[i]);
    for(int i=0; i<x; i = i+1)
        printf("%ld ", a[i]);
    return 0;
}