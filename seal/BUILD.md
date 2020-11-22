# Figuring out integration using bazel build

+ output of bazel commands 
```
echo $(bazel info command_log) 
```
+ terminal output
    + https://stackoverflow.com/questions/58652382/how-to-capture-fleeting-bazel-console-output
```
bazel build :my_project >& /tmp/bazel_build.log
```