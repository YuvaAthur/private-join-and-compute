# Figuring out integration using bazel build

+ output of bazel commands 
```
echo $(bazel info command_log) 
```
+ terminal output
    + https://stackoverflow.com/questions/58652382/how-to-capture-fleeting-bazel-console-output
```
bazel build :all --cxxopt='-std=c++17'  >& /tmp/bazel_build.log
bazel-bin/seal_examples
```