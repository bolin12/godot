# Agent-centric Debugging Interface：面向 Agent 的动态调试接口

## 基本信息

- 原文：[arXiv:2604.24212](https://arxiv.org/abs/2604.24212)
- PDF：[Agent_Centric_Debugging_Interface.pdf](Agent_Centric_Debugging_Interface.pdf)
- 研究类型：面向 Agent 的动态程序分析和调试接口
- 与 Godot 研究关联：P0，直接相关

## 论文要解决的问题

传统调试器按行、按变量、按调用栈进行交互，要求人持续操作。LLM Agent 进行大量低层调试调用会消耗上下文和预算，因此需要面向 Agent 推理方式的高层状态摘要。

## 方法分析

核心抽象是 Frame Lifetime Trace（FLT），记录一次函数调用的完整生命周期：调用帧 ID、调用者、参数、返回值，以及函数内部的有序语句执行、状态变化和子调用。系统先记录轻量级函数调用帧，只对当前关注的函数生成详细 Trace。

这是一种“先粗后细”的自适应观测设计，比全量记录每条语句更适合 Agent。

## 实验和证据

- 在 SWE-bench Verified 上达到 63.8% 任务解决率；
- 平均成本约 1.28 美元/任务；
- 接入现有 Agent 后，解决任务数提升约 6.2%～18.5%；
- 论文仍需进一步验证跨语言、跨项目和非代码运行时的泛化。

## Godot Engine 落地

Godot 应提供 `GodotInvocationTrace`：

```text
invocation_id + caller_id + method + node_path
+ inputs + outputs + property_diffs
+ emitted_signals + awaited_operations + child_invocations
```

Agent API 应优先提供对象级和函数级命令：查询 Node 状态历史、Signal 链、Resource 引用链、调用前后 Diff、进入一次调用、对比两次调用和按假设开启细粒度追踪。

不建议直接暴露逐帧查看所有属性的接口。Agent 需要的是“某个对象为什么变成当前状态”，而不是完整的引擎内部流水账。

## 研究结论

这篇论文直接指导 Godot 的 Agent Debug API 和 Trace Schema 设计。最重要的迁移原则是：以函数/对象调用为基本调试单元，以状态变化为主要证据，以按需细化控制开销。
