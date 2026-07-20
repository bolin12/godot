# RepairAgent：自主式 LLM 程序修复

## 基本信息

- 原文：[arXiv:2403.17134](https://arxiv.org/abs/2403.17134)
- PDF：[RepairAgent_Autonomous_Program_Repair.pdf](RepairAgent_Autonomous_Program_Repair.pdf)
- 研究类型：工具驱动的自主程序修复
- 与 Godot 研究关联：P1

## 方法分析

RepairAgent 使用动态工具集合和有限状态机，让 Agent 自主决定何时读取代码、获取测试、搜索相似 API、进行故障定位、表达假设、收集更多信息、放弃假设或应用 Patch。

应用 Patch 的工具会运行测试；测试失败则自动恢复工作区。系统还通过“表达假设”和“放弃假设”控制诊断过程，避免 Agent 仅靠连续生成 Patch 试错。

## 实验事实

论文在 Defects4J 上修复了 164 个真实 Bug，并报告平均约 270,000 Token/bug 的成本。结果说明工具化 Agent 可以处理复杂、多文件修复，但预算和工具调用次数必须成为评估指标。

## Godot Engine 落地

建议 Godot Agent 状态机为：

```text
UnderstandFailure → FormHypothesis → CollectEngineEvidence
→ ApplyIsolatedPatch → Behavior/Performance Tests
→ Commit or Reject
```

引擎工具应提供读取 Scene/Node/Resource、查询运行时状态、启动最小场景测试、查看错误窗口、运行性能预算测试、应用事务 Patch 和回滚等命令。

## 研究结论

RepairAgent 的核心启发是“Agent 选择调试动作”比“Agent 直接生成代码”更重要。Godot 研究应把假设、证据、Patch 和验证状态做成正式对象，而不是只存在于 Prompt 历史中。
