# AgentTrace：因果图追踪与多 Agent 根因分析

## 基本信息

- 原文：[arXiv:2603.14688v2](https://arxiv.org/abs/2603.14688)
- PDF：[AgentTrace_Causal_Graph_Tracing.pdf](AgentTrace_Causal_Graph_Tracing.pdf)
- 研究类型：多 Agent 系统事后故障诊断
- 与 Godot 研究关联：P0，直接相关

## 论文要解决的问题

多 Agent 系统中的错误常常在下游才表现出来。单看最后的错误日志，无法区分根因、传播节点和错误表现。论文的目标是找出最早一个“如果修正就能阻止最终错误”的根因节点。

## 方法分析

论文把执行轨迹建模成有向图。节点是 Agent 动作、消息、工具调用或决策；边包括同一 Agent 的顺序边、Agent 间通信边和数据依赖边。系统从错误节点反向 BFS，收集限定深度内的上游候选，再用位置、结构、内容、信息流和置信度五组特征排序。

位置特征权重为 0.70，结构特征为 0.20，其余较小。这个结果说明早期规划或路由错误容易产生级联影响，但不能直接假定游戏引擎中越早的事件越可能是根因。

## 实验和证据

- 550 个合成故障场景，覆盖 10 个领域；
- 每条轨迹约 8～15 个动作；
- 故障包括逻辑错误、通信失败、数据污染、缺少验证和角色混淆；
- Hit@1 为 94.9%，Hit@3 为 98.4%；
- GPT-4 全轨迹分析基线 Hit@1 为 68.5%；
- 平均耗时约 0.12 秒，LLM 分析约 8.3 秒。

## 局限

场景是合成的，默认单一根因并假设日志完整。真实游戏故障可能有多个共同根因、缺失观测、异步顺序变化和不可重现的外部输入。

## Godot Engine 落地

Godot 图边应扩展为：脚本/Node 事件顺序、Signal/CallDeferred/RPC/线程任务通信、属性写入和 Resource 引用数据依赖。节点应包含 `patch_id`、`node_path`、`resource_id`、`frame`、`thread`、`state_hash` 和 `lifecycle_state`。

建议实现：

```text
TraceEvent → EdgeBuilder → FailureWindow → BackwardQuery → RootCauseRanker
```

第一版使用确定性反向查询和可解释排序，不要一开始训练模型。必须增加多根因检测、生命周期边、异步边和网络边。

## 研究结论

AgentTrace 最值得吸收的是“错误后反向追踪”和“调试时不依赖 LLM”。它应成为 Godot Agent 调试层的根因候选生成器，而不是完整自动修复系统。
