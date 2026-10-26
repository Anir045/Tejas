int main() {
    int x = 10;
    int y = 20;

    int *p = &x;
    int *q = &y;

    return *p + *q;
}