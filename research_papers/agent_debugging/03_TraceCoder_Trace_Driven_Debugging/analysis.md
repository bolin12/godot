# TraceCoder：Trace 驱动的多 Agent 自动调试

## 基本信息

- 原文：[arXiv:2602.06875](https://arxiv.org/abs/2602.06875)
- PDF：[TraceCoder_Trace_Driven_Debugging.pdf](TraceCoder_Trace_Driven_Debugging.pdf)
- 研究类型：LLM 生成代码的运行时调试和自动修复
- 与 Godot 研究关联：P0

## 方法分析

TraceCoder 将流程拆成 Instrumentation Agent、Analysis Agent 和 Repair Agent。插桩 Agent 根据失败信息选择探针位置；分析 Agent 结合运行时 Trace 和历史失败记录定位根因；修复 Agent 执行修复计划。

它还包含 Historical Lesson Learning Mechanism 和 Rollback Mechanism。每轮失败都记录修复计划、错误、通过测试数和失败原因；失败 Patch 会被撤销，避免后续 Agent 在污染工作区上继续推理。

插桩强调四点：逻辑分解、状态和控制流可追踪、插桩纯净、结构化输出。论文报告插桩语义保持率超过 99%，但仍依赖 LLM 生成探针，不能视作形式化保证。

## Godot Engine 落地

不要让 Agent 任意改 GDScript 插入 `print`。应由引擎提供类型安全的探针模板：Node 生命周期、属性写入、Signal 发送/接收、Resource 加载、输入、网络和性能事件。Agent 只提交观测目标和原因。

推荐流程：

```text
Observation Planner → Engine Instrumentation → Causal Analysis
→ Patch Agent → Isolated Run/Test → Commit or Rollback
```

历史记录应使用结构化字段，避免只保存自然语言总结。至少包括假设、观测点、Patch、失败类型和验证结果。

## 研究结论

TraceCoder 对本项目的最大价值是证明了“插桩—分析—修复—回滚”应成为多 Agent 分工，而不是让一个 Agent 同时完成所有动作。Godot 版本应把插桩权收回引擎语义层，减少 LLM 对运行时的破坏性修改。
