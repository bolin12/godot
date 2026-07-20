# Knowledge-Based Zero-Replay Debugging：零重放成本的候选事件筛选

## 基本信息

- 原文：[arXiv:2606.14805](https://arxiv.org/abs/2606.14805)
- PDF：[Knowledge_Based_Zero_Replay_Debugging.pdf](Knowledge_Based_Zero_Replay_Debugging.pdf)
- 研究类型：多 Agent Trace 的事件知识图谱和重放预算分配
- 与 Godot 研究关联：P1

## 方法分析

论文把路由、记忆写入、工具调用、不确定性和证据组织成事件知识图谱。它不声称完全取消重放，而是预测哪些事件最值得消耗有限重放预算，减少逐一重放所有候选事件的成本。

论文使用可观察特征、结构特征、不确定性和潜在证据信号训练排序预测器，并在固定重放 Oracle 下评估预测结果。

## 实验事实

在 37 类轨迹上，Branch Recall@5 从 0.73 提升到 0.93，且不需要先执行重放 Oracle。论文强调这是成本—准确率折中，不是所有场景都能零成本得到正确反事实结论。

## Godot Engine 落地

Godot 重放应当视为有限预算资源：先从错误窗口收集候选事件，再根据因果图中心性、依赖距离、Patch 关联度和状态不确定性排序，只恢复 Top-k 检查点。

```text
Failure Window → Candidate Events → Rank
→ Replay Budget → State Diff → Root Cause Confidence
```

## 研究结论

这篇论文适合指导“自适应观测”和“重放调度”，不适合直接作为 Godot 的完整重放实现。Godot 还必须解决随机数、输入、异步完成顺序、网络包和外部资源版本的确定性记录。
