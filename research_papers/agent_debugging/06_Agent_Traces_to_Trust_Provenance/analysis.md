# From Agent Traces to Trust：Agent 执行溯源与证据血缘

## 基本信息

- 原文：[arXiv:2606.04990](https://arxiv.org/abs/2606.04990)
- PDF：[Agent_Traces_to_Trust_Provenance.pdf](Agent_Traces_to_Trust_Provenance.pdf)
- 研究类型：LLM Agent 执行溯源综述
- 与 Godot 研究关联：P2，基础设施方向

## 综述内容

论文从 provenance 视角整理 Agent 的 Trace 来源、执行单元、溯源关系、粒度、表示形式和信任功能。覆盖检索证据、工具参数和输出、记忆写入、环境观测、中间结论、动作和最终结果。

论文指出当前领域的主要缺口：统一且可互操作的 Trace Schema、语义级证据归因、记忆血缘、真实执行轨迹基准、恢复导向评估以及隐私和审计基础设施。

## Godot Engine 落地

Patch Attribution 不应只记录“哪个 Agent 改了哪个文件”，还要记录：需求、调试假设、引擎文档、工具调用、运行时观测、Patch、测试证据和提交/回滚结果。

```text
Requirement → Hypothesis → Tool Call → Runtime Evidence
→ Patch → Validation Evidence → Commit/Rollback
```

建议所有节点共享 `trace_id`、`patch_id`、`evidence_id` 和 `parent_event_id`，让系统能够回答“为什么这个 Patch 被认为合理”和“这个结论依赖哪个观测”。

## 研究结论

这篇综述不提供可直接运行的根因算法，但为 Engine 研究提供了数据模型和评价维度。它应作为 Trace Schema、审计和可解释修复的理论参考。
