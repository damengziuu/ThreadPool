# C++ Thread Pool 线程池

📖 **For English users, you can refer to the [English version](README_EN.md).**

这是一个基于现代C++的线程池实现，旨在通过线程池高效执行并发任务，避免频繁创建、销毁线程的开销。

## 简介

本**线程池库**是一款强大且高度可配置的 C++ 库，旨在通过提供一种管理线程池的高效机制来简化并发编程。它封装了线程创建、同步和任务调度的复杂性，使开发者能专注于实现应用逻辑，同时享受到性能提升和资源利用率的优化。

### 关键优势

- **易用性**: 简化了在应用中引入多线程的过程，缩短开发时间，降低复杂度。
- **高效资源分配**: 自动管理线程生命周期并将任务分配给可用线程，减少开销，最大化 CPU 利用率。
- **可扩展性**：支持动态调整线程池大小，以应对工作负载变化，确保灵活适应不同资源占用需求。
- **可配置性**: 提供对线程池行为的精细控制，如调整线程数、设定任务队列限制以及控制池暂停、恢复和终止。
- **跨平台性**: 

## 功能

### 线程管理

- **动态线程池**: 维护一组负责执行应用程序提交任务的工作线程。线程池大小可以在运行时手动调整以适应不同工作负载。

### 任务调度与控制

- **任务队列**: 接收任务并按照所选调度策略进行组织。队列长度可以设限，超出时将拒绝新的提交，直至空间释放。
- **调度策略**: 目前支持的策略是先入先出（First-In, First-Out, FIFO），后续计划支持更多的调度策略。

### 池暂停与恢复

- **暂停与恢复**: 允许线程池暂时挂起，使所有活动线程阻塞并等待进一步指令。恢复操作则解除线程阻塞，恢复正常任务处理。

### 终止模式

- **优雅关闭**: 提供等待所有当前排队任务完成后终止线程池的选择。确保干净退出，不丢失任何正在进行的工作。
- **立即终止**: 线程池可以立即终止，放弃队列中剩余的所有任务。
  - 当线程池实例自然销毁时，将触发这一终止模式，以确保资源迅速正确释放。

## 目标用户

本线程池库适用于：

- **C++ 开发者**：希望在应用中引入多线程而无需处理底层线程管理。
- **对性能敏感的应用**：如服务器后台、实时数据处理、计算密集型任务等受益于并行执行的场景。
- **开源项目**：希望通过集成一个可靠且文档详尽的线程池组件来增强其并发能力。
- **学习者**：希望深入理解C++、多线程编程以及线程池原理的学习者，可以通过研究本库的源码、文档和示例，获得宝贵的实践经验。

## 项目文件结构

```
ThreadPool/
├── README.md
├── README_EN.md	# 英文README
├── LICENSE			# MIT LICENSE
├── .gitignore		# Git忽略文件，告诉Git哪些文件或文件夹不应该被版本控制
├── .github/workflows/     # GitHub Action工作流
│	└── cmake-multi-platform.yml	# GitHub Action工作流的配置
├── include/		# 头文件目录
│	└── thread_pool.hpp		# 包含线程池类的声明
├── src/			# cpp文件目录，包含头文件中声明的成员函数等的实现
│	├── thread_pool.cpp		# 包含线程池类中非模版/内联成员函数的实现
│	└── worker_thread.cpp	# 包含工作线程类中非模版/内联成员函数的实现
└── tests/			# 测试文件目录，包含各个测试程序
	├── functional_test.cpp	# 对线程池的基本功能的测试程序
	└── performance_test.cpp	# 对线程池的性能的测试程序
```

安装指南
---
### 从源代码编译
要从源代码构建线程池库，请按照以下步骤操作：
1. 克隆仓库（取得源代码）：
   ```shell
   git clone https://github.com/Razirp/ThreadPool.git
   ```

   > 亦可通过你所熟悉的任何可行的方式（如ssh/GitHub CLI进行克隆、直接下载zip压缩包等）获得源代码。

2. 进入项目目录：

   ```shell
   cd ThreadPool
   ```

   > 目录名取决于你的命名。

3. 创建并进入构建目录：

   ```shell
   mkdir build
   cd build
   ```

   > 推荐，但非必需。创建专门的构建目录的目的是为了更方便地管理构建/编译过程中生成的文件。

4. 运行`cmake`生成构建文件：

   ```shell
   cmake ..
   ```

   > `cmake` 后的路径为CMakeLists.txt所在目录。

5. 编译库文件：

   ```
   cmake --build .
   ```

   > 或使用GNU make在Makefile所在目录下直接使用`make`命令进行编译：
   >
   > ```shell
   > make
   > ```

#### 已知问题

在使用 Microsoft Visual C++ (`cl`) 编译器在 Windows 平台上构建本线程池库时，可能会遇到如下链接错误：

```shell
LINK : fatal error LNK1104: 无法打开文件“Debug\ThreadPool.lib” [xxx\ThreadPool\build\FunctionalTest.vcxproj]
```

尽管目前尚未明确此问题的根本原因，但观察到当使用 MinGW-w64 工具链中的 `g++` 编译器时，该问题并未出现。鉴于此情况，对于 Windows 用户，我们现阶段建议优先选用 `g++` 进行编译以确保顺利构建。

> 欢迎了解该问题成因的社区成员提供宝贵见解与解决方案！

### 使用预编译二进制文件

在“Releases”部分提供了适用于Linux、Mac OS和Windows平台的预编译二进制文件（动态链接库）。请下载适合您的平台的文件，并将其解压至您选择的位置，然后将其正确链接至您的项目中。

在这种情况下，您只需要在项目中包含`thread_pool.hpp`头文件，然后调用相应的接口即可。

> 动态链接库的链接方法取决于你所使用的构建系统/编译工具，请参阅你所使用的工具的文档/参考资料。

使用方法
---
要使用线程池库，请在您的项目中包含`thread_pool.hpp`头文件，并参考以下示例代码：

```cpp
#include "thread_pool.hpp"
#include <iostream>
#include <future>

void task(int id) {
    std::cout << "Hello from thread " << id << std::endl;
}

int main() {
    thread_utils::thread_pool pool(4); // 创建包含4个线程的线程池
    for (int i = 0; i < 8; ++i) {
        pool.submit(task, i);
    }
    pool.shutdown(); // 可手动终止线程池
    // 或者待线程池对象离开作用域后自动终止析构
    return 0;
}
```

> 亦可参照[测试目录](tests/)下的测试程序（如 [`functional_test.cpp`](tests/functional_test.cpp) 等），以了解更多功能的效果与使用方法。

## TODO 待办事项

### 计划实现的特性

- **更多任务调度策略**：目前仅支持先进先出（FIFO）策略，未来将加入更多调度算法，如优先级调度、定时调度等，以满足不同应用场景的需求。

- **核心线程数与最大线程数概念**：引入核心线程数和最大线程数的概念，使得线程池能在工作负载变化时自动调整线程数量。核心线程始终保留在池中，而最大线程数则限定线程池可动态扩展的上限。

- **任务拒绝策略**：除了现有的队列满时拒绝新任务外，将实现更丰富的任务拒绝策略，并允许用户自定义拒绝策略，以应对任务队列饱和的不同处理需求。

- **深度测试与验证**：进行更全面的功能测试、性能基准测试，包括与现有线程池库的对比分析，以充分展现本库的功能稳定性和性能优势。

## 征集

### 向社区征集

我们诚挚邀请开源社区成员参与到以下贡献中：

- **功能实现**：协助完成上述 TODO 列表中的计划功能，如实现新的任务调度策略、核心线程数与最大线程数的支持、自定义任务拒绝策略等。

- **测试与基准**：提供更多的功能测试用例、性能测试数据，尤其是与现有线程池库的对比测试结果，以丰富本库的测试覆盖度和性能评估数据。

- **实践经验分享**：征集社区成员在使用本线程池库过程中的配置建议、性能优化、错误排查、安全注意事项等方面的经验分享，以帮助其他用户更好地理解和运用本库。


欢迎有兴趣的开发者通过 GitHub 提交 Pull Request 或 Issue，共同推进本线程池库的发展。

贡献与支持
---

欢迎任何形式的贡献！如果您发现了任何问题或有改进意见，请提交issue或发起pull request。

许可证
---
本项目采用MIT许可协议 – 详情请参阅[LICENSE](LICENSE)文件。

联系方式
---
如有任何问题或咨询，请通过RenPengyu2001@outlook.com与我联系。