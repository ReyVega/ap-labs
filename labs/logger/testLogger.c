extern int infof(const char *format, ...);
extern int warnf(const char *format, ...);
extern int errorf(const char *format, ...);
extern int panicf(const char *format, ...);

int main() {
    infof("Hello, I am %s, I am %d years old\n", "Reynaldo", 21);
    warnf("WARNING: line %d, %d, %d are innecessary\n", 1, 59, 102);
    errorf("ERROR: you can't declare more than 2 %s\n", "statements");
    panicf("CORE DUMPED: line not recheable at %d\n", 179);
    return 0;
}
