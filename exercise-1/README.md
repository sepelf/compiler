
# Links
- https://llvm-tutorial-cn.readthedocs.io/en/latest/index.html
- https://github.com/leeonfield/Semantic-analysis

## How to make it works
```bash
# bc.cc 简单的手写计算器，参考他人的办法用递归实现了运算符优先级。
g++ -g -std=c++11 bc.cc -o bc

# toy.cc 实现了llvm-tutorial前两章，词法分析、语法分析，优雅地实现了运算符优先级。
g++ -g -std=c++11 toy.cc -o toy
```