# DebugRepair：自指导调试增强自动程序修复

## 基本信息

- 原文：[arXiv:2604.19305](https://arxiv.org/abs/2604.19305)
- PDF：[DebugRepair_Self_Directed_Debugging.pdf](DebugRepair_Self_Directed_Debugging.pdf)
- 研究类型：带运行时中间状态的自动程序修复
- 与 Godot 研究关联：P1

## 方法分析

论文指出编译错误、Stack Trace 和测试失败等结果级反馈无法揭示根因所需的中间状态。DebugRepair 先净化失败测试上下文，提取最小依赖；然后识别关键变量、插入运行时观测；最后将中间状态、失败信息和历史 Patch 交给修复 Agent。

插桩需要通过两项检查：移除日志后代码与原始代码等价，并且插桩代码可以编译。修复流程分为多个 Session 和每个 Session 内的多个 Round；如果当前观测不足，就重新采集 Trace，而不是无限延长同一轮对话。

## 实验事实

论文在 Defects4J、QuixBugs 等数据集上评估，在多个模型上报告了明显优于普通反馈循环的修复效果；论文摘要报告 GPT-3.5 在 Defects4J 上修复 224 个 Bug，DeepSeek-V3 修复 295 个 Bug。具体结果不能直接外推到 Godot。

## Godot Engine 落地

“测试语义净化”可对应为最小可复现场景：最小 Scene、必要 Node/Resource、输入序列、随机种子、时间步和网络消息。一个调试 Session 应绑定一组观测假设；若无法区分生命周期错误和 Signal 参数错误，则重新规划观测。

## 研究结论

这篇论文支持本项目将“错误窗口内的中间运行状态”作为一等数据，而不是只把日志文本交给 Agent。它还提供了重要的预算控制思想：失败后重新定义上下文，有时比继续追加上下文更有效。
