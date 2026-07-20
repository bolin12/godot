# AI 原生游戏引擎与 Agent 调试研究对话记录

## 1. 讨论起点：Cocos、Godot 与 AI 原生游戏引擎

### 用户观点

用户提出，Cocos 和 Godot 是否都属于较小型、更加 AI 原生的游戏引擎。

### 核心结论

Cocos 和 Godot 都可以视为：

- 轻量级游戏引擎；
- 更容易被 AI Agent 接入和控制；
- 更适合作为 AI 游戏生成或 AI 编辑器研究底座。

但严格来说，它们目前仍然不是完整意义上的“AI 原生游戏引擎”。

更准确的定位是：

> 它们是传统游戏引擎，但由于代码库更小、场景结构更统一、文本化程度更高、编辑器扩展更容易，因此比 Unreal Engine 更容易被改造成 Agent 驱动的开发环境。

### Godot 的特点

Godot 适合作为 AI Agent 研究底座，原因包括：

- Node/Scene/Resource 抽象统一；
- GDScript、C# 和 C++ 扩展体系清楚；
- 场景与资源相对文本化；
- 编辑器插件机制开放；
- 引擎本体完整开源；
- 更容易建立场景树、状态查询和自动测试闭环。

适合的方向：

- AI 自动创建场景；
- Agent 操作节点树；
- 自动生成脚本；
- 自动运行和截图；
- 自动调试；
- Agent-native Engine 架构研究。

### Cocos Creator 的特点

Cocos 更适合：

- Web 游戏；
- 小游戏；
- 移动端轻量游戏；
- AI 批量生成项目；
- 模板化换皮；
- 自动构建和发布；
- TypeScript 驱动的 AI 内容生产流水线。

可将其理解为：

> Godot 更适合研究“AI 如何控制一个完整游戏引擎”，Cocos 更适合研究“AI 如何批量生产和发布轻量游戏”。

---

## 2. AI 时代是否会削弱 Unreal Engine 的市场

### 用户判断

用户认为，未来 AI 时代，UE 这种重型引擎可能市场不会很好，而小型 AI 引擎会有更多市场。

### 核心判断

未来更可能是“市场分层”，而不是轻量引擎全面取代 UE。

可能形成如下格局：

- 轻量 AI 引擎：项目数量多、开发者数量多；
- UE：项目数量较少，但单项目价值高；
- 中间层传统引擎：最容易被挤压。

### 小型 AI 引擎的优势

小型 AI 引擎更适合：

- 自然语言生成小游戏；
- 独立创作者；
- 教育和营销内容；
- Web 与小游戏平台；
- 个人定制内容；
- UGC 平台；
- 快速原型。

关键原因是 Agent 的闭环更快：

```text
修改
→ Reload
→ Run
→ 读取错误
→ 再修改
```

而 UE 的反馈链通常更长：

```text
修改
→ 编译
→ 启动编辑器
→ 加载项目
→ 构建或 Cook
→ 测试
```

### UE 仍然保有优势的市场

UE 的复杂性很大一部分是为了解决真实的大型工程问题：

- AAA 游戏；
- 高品质 3D；
- 大世界；
- 主机平台；
- 大型团队协作；
- 动画、物理和高级渲染；
- 虚拟制片；
- 工业仿真；
- 数字孪生；
- 机器人和合成数据。

因此更准确的结论是：

> 轻量 AI 引擎未来可能占据更广泛的项目数量，而 UE 仍会控制高价值、高复杂度和高品质实时三维市场。

---

## 3. UE Agent 相关论文与研究工作

重点讨论了以下工作。

### 3.1 AutoUE

论文：

**AutoUE: Automated Generation of 3D Games in Unreal Engine via Multi-Agent Systems**

核心内容：

- 多 Agent 分工；
- 3D 模型检索；
- 场景生成；
- Gameplay C++ 生成；
- 交互对象生成；
- 自动 Play-testing；
- UE 文档 RAG；
- MCP 集成。

主要工作流：

```text
自然语言需求
→ 需求分解
→ 模型检索 Agent
→ 场景生成 Agent
→ Gameplay Agent
→ 交互对象 Agent
→ Play-testing Agent
→ Unreal Engine
```

### 3.2 DreamGarden

核心思路：

- 从一句自然语言提示生成 UE 场景或游戏原型；
- 层次化任务规划；
- 生成 C++ Actor；
- 使用 Python Editor Scripting 布局场景；
- 编译失败后读取日志并修复；
- 截图后通过视觉模型评价结果。

形成闭环：

```text
Code
→ Compile
→ Run
→ Screenshot
→ VLM Evaluation
→ Repair
```

### 3.3 GameEngineBench

关注点不是从零生成游戏，而是：

- 评估 Coding Agent 修改真实 UE C++ 项目的能力；
- 使用真实代码库；
- 进行编译；
- 执行行为测试；
- 计算 Pass@1 等指标。

重要启示：

UE Agent 的评价不能只看代码生成，还要看：

- 编译是否成功；
- 行为是否正确；
- 是否引入回归；
- 运行时是否正确；
- Token 消耗；
- 工具调用次数；
- 执行时间；
- 修复轮次。

---

## 4. AutoUE 是否只是一个 UE 外壳

### 结论

AutoUE 基本没有修改 UE 核心结构。

它使用的是：

```text
现有 UE5
+ PCG
+ 生成 C++
+ 自定义 Gameplay 框架
+ MCP
+ 多 Agent 编排
+ 自动测试
```

没有明显修改：

- UObject 模型；
- Blueprint 序列化；
- Blueprint Compiler；
- Asset Registry；
- Package 系统；
- Editor Transaction；
- Reflection Metadata；
- World Partition；
- Cook 和 Build；
- 引擎核心 Agent 抽象。

### 正确定位

AutoUE 不是 Agent-native Engine 架构研究，而是：

> 在现有 UE 之上建立一条受约束、可自动执行、可测试的游戏生成流水线。

它并非简单聊天壳，而是一个“厚壳”，因为它加入了：

- Agent 分工；
- UE 文档 RAG；
- 结构化 specification；
- PCG Graph 生成；
- Gameplay 模块框架；
- Runtime 测试。

但它没有回答：

> UE 本身应该如何改变，才能真正适合 Agent。

---

## 5. 为什么 UE 需要被重新设计才能真正适配 Agent

UE 当前很多设计默认使用者是人类：

- 人理解 UI；
- 人能处理隐式状态；
- 人能观察上下文；
- 人能手动判断副作用；
- 人能在 C++、Blueprint、资产和配置之间跳转。

Agent 更适合：

- 显式状态；
- 强类型接口；
- 确定性输出；
- 结构化错误；
- 可查询依赖；
- 可回滚事务；
- 可验证结果。

### 5.1 Blueprint 需要语义 IR

Blueprint 对 Agent 不友好，因为它包含：

- 二进制资产；
- 节点坐标；
- Pin；
- GUID；
- 隐式转换；
- 图结构；
- 编译和重实例化。

建议建立：

```text
Blueprint Graph
↕
Canonical Semantic IR
↕
Agent
```

该 IR 应具备：

- 稳定；
- 文本化；
- 可 Diff；
- 可局部 Patch；
- 可验证；
- 可逆转换回 Blueprint。

### 5.2 资产系统需要语义事务

Agent 不应该只调用：

```text
SetProperty()
SaveAsset()
```

而应使用：

```text
BeginTransaction()
ModifyAsset()
PreviewImpact()
Validate()
Commit()
```

并返回：

- 受影响资产；
- 重编 Blueprint 数量；
- Shader 重编数量；
- Cook 影响；
- 外部引用变化；
- 性能代价。

### 5.3 反射系统需要机器语义

仅有类型信息不够。

Agent 还需要：

- 单位；
- 合理范围；
- 修改副作用；
- 性能影响；
- 是否运行时可改；
- 是否需要重建；
- 与哪些属性互斥；
- 零值的语义。

### 5.4 编辑器需要去 UI 化

Agent 不应依赖：

- 鼠标；
- 屏幕坐标；
- Slate 层级；
- 当前选中项；
- 打开的 Tab；
- 弹窗。

人和 Agent 都应该调用统一的 Editor Command Layer。

### 5.5 错误和诊断需要结构化

需要统一诊断格式，覆盖：

- C++ 编译；
- Blueprint；
- Material；
- Shader；
- Cook；
- Packaging；
- Niagara；
- Runtime Ensure；
- Crash；
- Automation Test。

### 5.6 UE 需要一等测试语义

测试不能只验证项目能启动。

应支持：

```text
Given
When
Then
```

并绑定：

- World；
- Actor；
- Component；
- Gameplay Event；
- 网络状态；
- 性能指标。

### 5.7 World 状态需要可查询

Agent 应能查询：

- Actor；
- Component；
- Visibility；
- Collision；
- Gameplay Tag；
- Animation；
- Network Role；
- Resource；
- Recent Events。

### 5.8 需要权限、沙盒和回滚

Agent 不应拥有无限权限。

应支持：

- 资产范围；
- 模块范围；
- 操作白名单；
- 编译预算；
- 性能预算；
- 自动回滚；
- 修改审查。

---

## 6. 深度改造引擎是否能帮助发现复杂 Bug

可以显著改善，但不能自动解决所有 Bug。

真正需要的是：

```text
Agent-friendly Engine
+ 自动测试生成
+ 状态覆盖探索
+ Runtime Instrumentation
+ Invariant
+ 故障注入
```

不同问题需要不同的观测能力。

| 问题 | 可加入的引擎能力 |
|---|---|
| 内存泄漏 | 对象分配图、所有权图、快照 Diff |
| 生命周期错误 | 创建、引用、销毁时间线 |
| 网络不同步 | Server/Client 状态快照、RPC 因果链 |
| 偶发 Crash | 确定性重放、输入录制、崩溃前快照 |
| Tick 性能问题 | Tick 依赖图、调用频率、预算归因 |
| 输入边界错误 | 属性约束、模糊测试、状态覆盖 |
| 长期状态错误 | 长时间模拟、持久化状态 Diff |
| 资产污染 | 事务 Patch、依赖分析、非目标修改检测 |

核心不是日志更多，而是：

> 建立“修改 → 状态变化 → 因果关系 → 最终错误”的可追踪链条。

---

## 7. 为什么个人研究不适合直接深改 UE

在 UE 上做这类研究，容易出现：

```text
80% 时间适配 UE 历史复杂度
20% 时间研究真正问题
```

常见负担包括：

- UObject 和 GC；
- Editor/Runtime 双环境；
- Game Thread；
- `.uasset` 与 Package；
- Blueprint 编译；
- Live Coding；
- Build.cs；
- 插件版本兼容；
- Cook；
- 自动化不稳定；
- 长编译闭环。

结果可能只是：

> 成功把数据导出。

而不是：

> 证明某种 Agent-native 架构显著提升了调试效率。

因此更适合个人的路线是：

```text
Godot：主研究平台
UE：迁移验证平台
Cocos：商业生态和内容生产参考
```

---

## 8. Godot 与 Cocos 的选择

### Godot 更适合 Agent-native Engine 研究

原因：

- 完整开源；
- C++ 核心；
- Node/Scene/Resource 统一；
- 文本化程度较高；
- EditorPlugin；
- GDExtension；
- 可无头运行；
- 易于扩展 Debugger 和 Profiler；
- 可逐步深入引擎核心。

推荐路线：

```text
阶段 1：EditorPlugin
阶段 2：GDExtension
阶段 3：修改 Godot Core
```

适合研究：

- 生命周期事件流；
- 状态快照；
- 因果图；
- 确定性重放；
- Runtime 插桩；
- Agent 调试协议；
- 可复现 Benchmark。

### Cocos 更适合 AI 游戏生产平台

原因：

- TypeScript 对 LLM 友好；
- 编辑器扩展简单；
- Node + Component 清晰；
- Web、小游戏、移动端生态强；
- 自动构建和发布更直接；
- MCP 社区活跃。

适合研究：

- Agent 操作编辑器；
- 意图级工具；
- Token 优化；
- 自动小游戏生成；
- 自动发布；
- 商业化 UGC 流水线。

---

## 9. AI 原生是否会损害传统引擎性能

不一定。

正确设计是双层架构：

```text
Agent 语义层
→ 校验与编译
→ 高性能 Runtime 表示
```

开发阶段：

- 高语义；
- 高可观测；
- 高验证能力；
- 完整追踪。

发布阶段：

- 裁剪调试数据；
- 静态化；
- 数据导向；
- 低运行时开销。

类似：

```text
C++ 源代码
→ 优化机器码

Material Graph
→ Shader

Agent Semantic IR
→ Runtime Component Data
```

### 不应让 Agent 每帧参与

合理模式：

```text
Agent
→ 修改项目
→ 编译
→ 运行测试
→ 分析结果
```

而不是：

```text
每帧状态
→ LLM
→ 决定下一帧
```

### 有些 Agent-native 改造甚至会提高性能

例如自动分析：

- Shader permutation；
- Draw Call；
- Tick 数量；
- 内存引用；
- 无法卸载的资源；
- 批处理破坏；
- 性能预算回归。

最终可实现：

```text
修改前预测
+ 修改后验证
+ 性能回归自动阻止
```

---

## 10. 追踪系统为什么能显著降低 Agent 调试成本

Agent 调试的主要瓶颈不是模型不会推理，而是缺乏准确因果信息。

普通模式：

```text
日志
+ 崩溃栈
+ 少量代码
+ 猜测
+ 修改
+ 再运行
```

Agent-native Trace 模式：

```text
错误发生
→ 定位相关对象
→ 回溯状态变化
→ 找到触发事件
→ 对应到 Patch、函数和资源
→ 生成修复
```

### 关键价值

可以减少：

- 上下文窗口占用；
- 无关代码读取；
- 重复检索；
- 无效测试；
- 修复轮次；
- Token 消耗；
- 工具调用次数；
- 运行验证次数。

### 追踪的四类数据

#### 代码修改追踪

记录：

- 哪个 Agent；
- 修改哪些文件；
- 修改哪些函数；
- 修改目的；
- 测试结果；
- 是否引入回归。

#### 对象与资源追踪

记录：

- 创建；
- 引用；
- 所有权；
- 加载；
- 卸载；
- 销毁；
- 异步回调。

#### 状态与事件追踪

记录：

- 输入；
- 状态机；
- Gameplay Event；
- RPC；
- 属性复制；
- 信号；
- 随机种子。

#### 性能因果追踪

例如：

```text
Agent Patch
→ 增加 1200 个 Tick 对象
→ Game Thread 增加 3.2 ms
```

### 追踪不应是全量日志

“任何都追踪”会导致：

- 内存膨胀；
- I/O 压力；
- 性能扰动；
- 数据噪声；
- 改变 Bug 的发生概率。

推荐多级追踪：

```text
Level 0：Release
Level 1：轻量统计
Level 2：采样事件
Level 3：对象/系统定向追踪
Level 4：失败窗口完整追踪
```

并使用 Ring Buffer：

```text
持续保存最近 10～30 秒
→ Crash/Assert/性能异常发生
→ 冻结并导出故障窗口
```

### 更高级的方向：自适应观测

```text
发现网络异常
→ 开启网络追踪
→ 缩小到相关 Actor
→ 缩小到属性和 RPC
→ 重放并定位
```

核心研究目标：

> 可解释的最小充分追踪，而不是无限量日志。

---

## 11. Godot 和 Cocos 是否已经有人完成这些方案

### 已有能力

Godot 和 Cocos 已经有：

- MCP；
- Agent 操作编辑器；
- 创建节点、场景和资源；
- 运行项目；
- 读取日志；
- 截图；
- Debugger；
- Profiler；
- Runtime 状态查看；
- 自定义性能指标。

### 尚未形成成熟整合的能力

目前公开生态中，尚未看到完整系统化实现：

- Agent Patch 归因；
- 对象生命周期因果图；
- 状态快照与语义 Diff；
- 确定性重放；
- 自动根因定位；
- 事务式修复；
- 行为和性能联合验证。

现状更像：

```text
已有：
Agent 能操作引擎
传统调试和性能工具

缺少：
Agent-oriented Causal Debugger
```

### 相关领域已有参考

可借鉴的方向包括：

- Time-travel debugging；
- Replay 系统；
- Agent 自身的 Trace；
- 软件因果分析；
- 状态转换树；
- 崩溃前 Ring Buffer；
- Deterministic Replay；
- Fault Injection。

因此研究机会是：

> 把 Agent 修改历史、游戏运行状态、对象生命周期、性能变化和最终错误组织成机器可查询的因果链。

---

## 12. 建议的研究方向

推荐使用 Godot 作为主平台，设计：

# Agent-Oriented Observability and Verification Layer

核心模块：

### 12.1 Patch Attribution

每次 Agent 修改都生成：

- Patch ID；
- Intent；
- 文件和资源变化；
- 预期效果；
- 行为不变量；
- 性能预算。

### 12.2 Causal State Graph

将以下元素组织成因果图：

- Agent Patch；
- Node；
- Resource；
- Event；
- Signal；
- State；
- Thread；
- Performance Sample；
- Error。

### 12.3 Failure-window Trace

仅保存失败前后的关键时间窗口：

- 输入；
- 状态；
- 生命周期；
- 资源；
-性能；
- 异步事件。

### 12.4 State Snapshot / Diff

支持：

- 场景树快照；
- 关键属性；
- 资源引用；
- 状态机；
- 性能数据；
- 修改前后 Diff。

### 12.5 Semantic Runtime Assertions

示例：

- 对象销毁后不得继续接收回调；
- Health 不得小于零；
- Server/Client 状态最终应收敛；
- Tick 时间不得超过预算；
- Agent 不得修改指定目录外资产。

### 12.6 Transactional Repair Loop

```text
Agent Patch
→ 隔离执行
→ 自动测试
→ 检查行为和性能
→ 通过则合并
→ 失败则回滚
```

---

## 13. 推荐实验设计

### Baseline

```text
源码
+ 普通日志
+ 崩溃栈
```

### Proposed

```text
源码
+ Patch Attribution
+ 生命周期事件
+ 状态快照
+ 因果图
+ 性能数据
+ 自动重放
```

### 评估指标

- Bug 定位成功率；
- 修复成功率；
- 平均 Token 消耗；
- 平均工具调用次数；
- 修复轮数；
- 总执行时间；
- 回归率；
- 人工介入次数；
- 非目标修改数量；
- Trace 运行时开销；
- Release 构建开销；
- Debug 构建开销；
- 状态恢复成功率。

---

## 14. 最终研究定位

这轮讨论最后形成的核心判断是：

> 不要把研究目标定义为“再做一个能操作编辑器的 MCP Agent”。

更有价值的目标是：

> 研究游戏引擎如何为 Autonomous Agent 提供可观测、可操作、可验证、可恢复和高性能的开发环境。

可将研究问题表述为：

> 哪种 Agent-oriented runtime observability architecture，能够在较低运行时开销下，显著减少 Agent 调试中的 Token、工具调用、修复轮数和总时间？

推荐技术路线：

```text
Godot 完整原型
+
Cocos 生态和工具设计参考
+
UE 小规模迁移验证
```

最终应避免直接长期维护大型 UE Fork，而是把核心贡献放在：

- 语义层；
- 因果追踪；
- 验证；
- 回滚；
- 自适应观测；
- 性能感知；
- 跨引擎抽象。

---

## 15. AgentTrace：面向多 Agent 系统的因果图根因定位

### 论文信息

- **论文**：AgentTrace: Causal Graph Tracing for Root Cause Analysis in Deployed Multi-Agent Systems
- **作者**：Zhaohui Geoffrey Wang
- **版本**：arXiv:2603.14688v2，2026-03-27 修订；发表于 ICLR 2026 Workshop on Agents in the Wild
- **原文**：[arXiv 页面](https://arxiv.org/abs/2603.14688)；用户提供的 [alphaXiv 页面](https://www.alphaxiv.org/zh/overview/2603.14688)
- **关键词**：multi-agent systems、causal tracing、root-cause analysis、execution logs、observability、post-hoc debugging

### 摘要整理（中文）

多 Agent 系统部署到真实环境后，故障往往会因级联效应、隐藏依赖和很长的执行轨迹而难以诊断。论文提出 AgentTrace，一个轻量级的事后故障诊断框架。它从执行日志重建因果图，从错误表现沿因果关系反向追踪，并利用可解释的结构特征和位置特征对候选根因排序。调试时不需要再次调用 LLM。在覆盖常见部署模式的多 Agent 故障基准上，AgentTrace 能以亚秒级延迟定位根因，并显著优于启发式方法和基于 LLM 的基线。论文据此认为，因果追踪可以成为提升 Agent 系统可靠性和可信度的实用基础。

### 对本研究最重要的启发

1. **日志不应只是文本，而应能还原因果结构。** AgentTrace 的关键不是增加日志数量，而是把 Agent、工具调用、消息传递、状态变化和错误表现组织成可追踪的图。
2. **根因定位可以与 LLM 解耦。** LLM 可以负责理解意图、生成 Patch 和解释结果，但错误发生后的第一轮候选根因排序应尽量由确定性或轻量算法完成，以降低 Token、延迟和不稳定性。
3. **从错误向前回溯比扫描完整轨迹更有效。** 游戏运行时可能产生大量 Tick、渲染、输入和网络事件，应从最终错误、断言、行为偏差或性能回归反向收缩到相关 Actor、Component、Resource 和 Patch。
4. **结构特征应成为诊断信号。** 可优先考虑：依赖距离、事件时间位置、跨 Agent/线程边界、错误传播次数、资源引用关系、最近 Patch 的关联度，以及状态变化是否违反不变量。
5. **需要单独建立游戏引擎故障基准。** 论文面向通用多 Agent 部署场景；本研究可以构造 Godot 专用故障族，例如节点生命周期、信号连接、资源引用、场景切换、输入状态、网络同步、帧时间回归和 Agent 非目标修改。

### 与现有研究方案的对应关系

| AgentTrace 概念 | Godot Agent 调试层的对应设计 |
|---|---|
| Execution log | Agent 操作、工具调用、引擎事件、运行时事件和测试结果的统一事件流 |
| Causal graph | Patch → Scene/Node/Resource → Event/Signal → State → Error/Performance 的因果图 |
| Error manifestation | 崩溃、断言、测试失败、行为不符合预期、性能预算超标 |
| Backward tracing | 从错误反向追踪相关对象、属性、信号、资源和最近修改 |
| Structural/positional ranking | 依赖距离、时间邻近、跨边界传播、Patch 归属和错误窗口位置 |
| No LLM at debugging time | 先由图查询和规则筛选候选根因，再交给 Agent 解释和修复 |

### 建议加入原型的最小闭环

```text
Agent Patch
→ structured event stream
→ causal edge construction
→ error-window detection
→ backward candidate ranking
→ state snapshot / diff
→ Agent repair proposal
→ isolated validation
→ commit or rollback
```

### 需要谨慎验证的地方

- 论文摘要说明了高准确率和亚秒级延迟，但当前文档中不应把这一结果直接外推为 Godot 场景中的效果；需要在游戏引擎故障集上重新测量。
- 论文主要讨论部署后的事后诊断，尚未直接解决引擎内部的资源语义、可逆事务、确定性重放和跨线程状态一致性。
- 因果边的质量决定系统上限。仅凭日志时间顺序建立边可能产生伪因果，需要结合调用关系、对象引用、消息 ID、Patch ID、线程和生命周期信息。

---

## 16. 相关研究地图：可作为后续研究方向的论文

以下条目按与“AI 原生游戏引擎 + Agent 自动调试”的关联度分类。除特别注明外，内容是根据论文摘要整理的研究定位，不代表已经在 Godot 上验证。

### A. 直接相关：Agent 轨迹、因果诊断与自动调试

#### A.1 TraceCoder：Trace-Driven Multi-Agent 自动调试

- **论文**：[TraceCoder: A Trace-Driven Multi-Agent Framework for Automated Debugging of LLM-Generated Code](https://arxiv.org/abs/2602.06875)
- **核心方法**：通过诊断探针采集细粒度运行时 Trace，进行因果分析定位根因；利用历史失败经验避免重复错误，并通过 Rollback 保证每轮修复必须取得进展。
- **对 Godot 的启发**：可以把“探针 + 因果分析 + 历史教训 + 回滚”移植到 GDScript/C# 运行时和 EditorPlugin 调试器；每个失败修复轮次都应保存 Patch、观测、假设、测试结果和回滚点。
- **优先级**：高。与当前第 12、13 节的研究设计几乎一一对应。

#### A.2 Agent-centric Debugging Interface：面向 Agent 的动态分析接口

- **论文**：[Empowering Autonomous Debugging Agents with Efficient Dynamic Analysis](https://arxiv.org/abs/2604.24212)
- **核心方法**：认为传统逐行调试器对 LLM Agent 成本过高，提出函数级 Agent-centric Debugging Interface，并使用 Frame Lifetime Trace 记录函数生命周期状态和高层导航操作。
- **对 Godot 的启发**：不要把 GDB/调试器的全部低层操作原样暴露给 Agent；应提供面向引擎语义的查询，例如“某 Node 最近一次状态变化”“某 Signal 的发送者和接收者”“某 Resource 的引用链”“某函数调用期间属性如何变化”。
- **优先级**：高。可直接指导 Agent Debug API 和 Trace Schema 的设计。

#### A.3 Knowledge-Based Zero-Replay Debugging

- **论文**：[Knowledge-Based Zero-Replay Debugging of Multi-Agent LLM Traces](https://arxiv.org/abs/2606.14805)
- **核心方法**：把路由、记忆写入、工具使用、不确定性和证据组织成事件知识图谱，在有限重放预算下预测最值得重放的事件，从而避免对所有候选事件逐一重放。
- **对 Godot 的启发**：将确定性重放视为昂贵资源。先用因果图、结构中心性、状态差异和不确定性选择少量关键时间点，再进行场景/运行时重放。
- **优先级**：高。适合研究“自适应观测”和“重放预算分配”。

#### A.4 DebugRepair：自指导的自动程序修复

- **论文**：[DebugRepair: Enhancing LLM-Based Automated Program Repair via Self-Directed Debugging](https://arxiv.org/abs/2604.19305)
- **核心方法**：指出编译错误和 Stack Trace 等结果级反馈不足以支撑根因分析，结合测试语义净化、模拟插桩、运行时中间状态和对话式修复来改进补丁。
- **对 Godot 的启发**：测试失败后不应只把错误文本交给 Agent；应自动生成针对性的观测点，例如变量、Node 状态、Resource 路径、信号参数和关键时间窗口。
- **优先级**：高。可用于设计“按假设生成观测”的调试策略。

### B. 观测、执行溯源与可信 Agent

#### B.1 AgentTrace：结构化 Agent 可观测性

- **论文**：[AgentTrace: A Structured Logging Framework for Agent System Observability](https://arxiv.org/abs/2602.10133)
- **核心方法**：在运行时持续采集结构化 Trace，覆盖 operational、cognitive 和 contextual 三个表面，用于调试、安全、问责和实时监控。
- **对 Godot 的启发**：当前方案需要定义统一事件协议，而不是分别记录 Editor 日志、游戏日志和 Agent 对话。建议至少包含 `operation_id`、`patch_id`、`actor_id`、`resource_id`、`parent_event_id`、`state_hash`、`thread` 和 `timestamp`。
- **优先级**：中高。它偏观测基础设施，不直接提供根因算法。

#### B.2 From Agent Traces to Trust：执行溯源综述

- **论文**：[From Agent Traces to Trust: Evidence Tracing and Execution Provenance in LLM Agents](https://arxiv.org/abs/2606.04990)
- **核心方法**：从 provenance 视角连接检索证据、工具输出、记忆、环境观测、中间结论、动作和最终结果，并总结统一 Trace Schema、语义溯源、审计、调试和恢复等开放问题。
- **对 Godot 的启发**：除了追踪“代码改了什么”，还要追踪“Agent 为什么认为应该这么改”：需求、检索文档、工具结果、运行时观测、修复假设和验证证据之间应可关联。
- **优先级**：中高。适合补强 Patch Attribution 和可解释性。

### C. 自动程序修复与工具使用闭环

#### C.1 RepairAgent：自主式 LLM 程序修复

- **论文**：[RepairAgent: An Autonomous, LLM-Based Agent for Program Repair](https://arxiv.org/abs/2403.17134)
- **核心方法**：让 Agent 自主决定何时收集缺陷信息、获取修复材料、调用工具和验证修复，而不是固定提示词或固定反馈循环；使用工具集合、动态 Prompt 和有限状态机组织流程。
- **对 Godot 的启发**：研究重点应从“让 Agent 生成代码”转向“让 Agent 选择下一项最有价值的调试动作”，例如查场景、查依赖、运行测试、采样状态、查看性能或回滚。
- **优先级**：中高。适合设计 Agent 调试状态机和工具选择策略。

#### C.2 VRpilot：带补丁验证反馈的漏洞修复

- **论文**：[A Case Study of LLM for Automated Vulnerability Repair](https://arxiv.org/abs/2405.15690)
- **核心方法**：结合推理、编译器、测试和安全 Sanitizer 的外部反馈迭代生成和筛选补丁。
- **对 Godot 的启发**：验证不应只有“项目能否启动”，还应覆盖场景测试、运行时断言、输入边界、网络状态、性能预算和资源完整性。
- **优先级**：中。更偏传统代码/安全修复，但验证反馈设计可复用。

### D. 可逆调试、确定性重放与基础设施

#### D.1 Causal-Consistent Reversible Debugging

- **论文**：[Causal-Consistent Reversible Debugging](https://arxiv.org/abs/2406.05719)
- **核心方法**：研究并发/消息系统中的可逆语义，使程序能够沿执行历史向后移动，并保持因果一致性。
- **对 Godot 的启发**：游戏中的信号、协程、异步资源加载、网络消息和多线程任务不能简单依赖“回退几帧”。需要记录足以恢复状态和副作用的因果历史。
- **优先级**：中高。适合理论化 State Snapshot、事件回滚和确定性重放。

#### D.2 Replay Debugging of Complex Real-Time Systems

- **论文**：[Replay Debugging of Complex Real-Time Systems](https://arxiv.org/abs/cs/0311019)
- **核心方法**：通过确定性重放支持复杂实时系统的事后交互式调试，并讨论工业系统中的插桩可行性和开销。
- **对 Godot 的启发**：确定性重放不是新概念，但必须围绕随机种子、输入、时间步、网络包、异步完成顺序和外部资源建立录制协议；应同时测量插桩开销。
- **优先级**：中高。是游戏引擎调试基础设施的重要参考，而非 Agent 专属研究。

---

## 17. 后续研究方向与建议排序

### 方向 1：Godot Agent Debug Trace Schema（最优先）

先定义统一事件模型和关联 ID，覆盖 Agent Patch、Editor 操作、Scene/Node/Resource 变化、Signal、输入、网络、线程、性能采样、断言和测试结果。没有统一 Trace，后续因果图和自动修复都难以评估。

### 方向 2：面向游戏语义的因果图

实现第一版 `Patch → Object/Resource → Event → State → Failure` 图。初期不追求完整因果推断，先利用显式引用、调用关系、父子节点、信号连接、时间窗口和状态 Hash 建立可解释边。

### 方向 3：Agent 的高层调试接口

设计函数级/对象级查询，不暴露过多逐行调试细节。建议优先实现：

- `query_object_history`
- `query_signal_chain`
- `query_resource_references`
- `query_state_diff`
- `query_failure_window`
- `request_targeted_instrumentation`
- `replay_from_checkpoint`

### 方向 4：按假设自适应插桩

Agent 先提出可验证假设，再由引擎只开启相关观测。例如怀疑“节点已销毁但仍收到信号”，就开启生命周期和信号接收链追踪，而不是记录所有 Tick。

### 方向 5：低成本根因排序与有限重放

将 AgentTrace 的无 LLM 因果排序与 Zero-Replay Debugging 的重放预算思想结合：先筛选候选根因，再把有限的重放次数用于最有信息价值的事件或状态快照。

### 方向 6：事务化自动修复

每个修复都应绑定 Patch ID、预期不变量、测试集合、性能预算和回滚点。只有在编译、行为、资源完整性和性能验证均通过时才提交；失败则自动回滚并把失败经验写入后续上下文。

### 方向 7：Godot 游戏引擎 Agent Debug Benchmark

建立可重复的故障基准和统一指标：根因 Top-k 命中率、修复成功率、回归率、平均修复轮数、Token、工具调用次数、运行时间、人工介入次数、Trace 开销、重放成功率和非目标修改数。

### 推荐实验顺序

```text
可控 Godot 小项目
→ 统一 Trace Schema
→ 10~20 个可注入故障
→ 因果图与根因排序
→ Agent 高层调试 API
→ 有限重放与自适应插桩
→ 自动修复和事务回滚
→ 与纯日志 + LLM 基线比较
```

### 当前研究定位更新

AgentTrace 使本项目的核心问题可以进一步收敛为：

> 如何为游戏引擎构建一种低开销、可解释、可回滚、可重放的 Agent-oriented causal observability layer，使 Agent 能够从游戏运行时故障反向定位根因，并在有限工具调用和 Token 预算内完成验证式修复？

这比单独研究“让 Agent 操作 Godot 编辑器”更聚焦，也能形成引擎接口、Trace 数据模型、因果诊断算法、自动修复闭环和 Benchmark 五个可独立评估的研究成果。

### 本节资料检索说明

本次整理以论文原始 arXiv 页面和摘要为主要依据，检索时间为 2026-07-20。相关论文可能仍处于预印本或 workshop 阶段；正式版本、代码和实验数据在后续复核时应再次确认。

---

## 18. 已下载论文原文与全文分析

本节对应的论文原文已下载到仓库目录：

`D:\godot\research_papers\agent_debugging\`

这些 PDF 不是只作为参考链接，而是作为后续 Godot Engine 研究的本地资料库。以下分析基于论文全文中的方法、实验和限制部分。

### 18.1 AgentTrace：因果图根因定位

- **本地原文**：[AgentTrace_Causal_Graph_Tracing.pdf](research_papers/agent_debugging/01_AgentTrace_Causal_Graph/AgentTrace_Causal_Graph_Tracing.pdf)
- **原文**：[arXiv:2603.14688v2](https://arxiv.org/abs/2603.14688)

#### 论文实际做了什么

论文把一次多 Agent 执行建模为有向无环图：节点是 Agent 的动作、工具调用、消息或决策；边分为三类：

1. **Sequential edge**：同一 Agent 相邻动作之间的顺序关系；
2. **Communication edge**：一个 Agent 发送消息到另一个 Agent 接收消息；
3. **Data dependency edge**：一个动作产生的数据被另一个动作消费。

当错误节点出现时，框架从错误节点执行限定深度的反向 BFS，收集上游候选节点，再用五组可解释特征排序：位置、结构、内容、信息流和置信度。论文给出的排序权重中，位置特征占 0.70，结构特征占 0.20，其余特征权重较小。

#### 实验事实

- 构造 550 个故障场景，覆盖 10 个领域；
- 每个场景约 8～15 个 Agent 动作；
- 故障类型包括逻辑错误、通信失败、数据污染、缺少验证和角色混淆；
- AgentTrace 的 Hit@1 为 94.9%，Hit@3 为 98.4%；
- GPT-4 全轨迹分析基线 Hit@1 为 68.5%；
- 平均处理时间约 0.12 秒，LLM 基线约 8.3 秒；
- 论文的主要限制是：场景为合成数据、默认单一根因、假设日志完整。

#### 对 Godot Engine 的直接迁移

Godot 版本的三类边不能照搬 Agent 消息关系，而应改成：

| AgentTrace | Godot Engine 版本 |
|---|---|
| Sequential edge | 同一脚本、Node、系统或线程中的事件顺序 |
| Communication edge | Signal、CallDeferred、RPC、WorkerThreadPool 任务、编辑器命令 |
| Data dependency edge | 属性写入、Resource 引用、Scene 实例化、输入数据、配置数据 |

Godot 根因节点可以定义为“最早一个如果被修正，就能阻止最终故障的 Patch 或运行时事件”。这一定义比“错误发生的那一行”更适合游戏，因为错误常常在多个帧、多个 Node 或多个线程之后才表现出来。

#### 不能直接照搬的地方

论文中位置特征效果很强，但 Godot 的 Tick、异步加载和网络同步可能导致“越早越可能是根因”并不总成立。必须增加：生命周期状态、对象引用、线程、帧号、网络方向、随机种子和资源版本等游戏语义特征。还要支持多个共同根因，而不是只返回一个节点。

#### 应形成的 Engine 模块

```text
SNTraceEvent
  → SNTraceEdgeBuilder
  → SNFailureWindow
  → SNCausalBackwardQuery
  → SNRootCauseRanker
```

第一版不需要机器学习。先实现确定性的反向图查询和可解释排序，再以 Godot 故障基准评估是否需要学习型排序器。

### 18.2 Agent-centric Debugging Interface：Agent 高层动态调试接口

- **本地原文**：[Agent_Centric_Debugging_Interface.pdf](research_papers/agent_debugging/02_Agent_Centric_Debugging_Interface/Agent_Centric_Debugging_Interface.pdf)
- **原文**：[arXiv:2604.24212](https://arxiv.org/abs/2604.24212)

#### 论文实际做了什么

论文认为传统调试器的逐行、逐变量交互方式对 LLM Agent 成本太高。它提出 ADI，并把一次函数调用抽象成 **Frame Lifetime Trace（FLT）**：

```text
FLT = frame_id
    + caller_frame_id
    + arguments
    + return_value
    + ordered_statement_trace
```

每条 statement trace 记录语句、状态修改、被调用的子函数以及调用顺序。Agent 通过高层命令在函数级别导航，而不是反复调用低层单步命令。系统只对当前关注的函数生成详细 Trace，其他函数先保留轻量调用帧，从而降低观测开销。

#### 实验事实

- 在 SWE-bench Verified 上，基础 Agent 配备 ADI 后达到 63.8% 任务解决率；
- 平均成本约 1.28 美元/任务；
- 接入现有 Agent 后，解决任务数提升约 6.2%～18.5%；
- 论文的主要风险是对其他语言、其他类型任务和更复杂运行时的泛化仍需验证。

#### 对 Godot 的直接迁移

Godot 不应该首先给 Agent 一个“逐帧查看所有属性”的接口，而应提供类似 FLT 的语义对象：

```text
GodotInvocationTrace
  = invocation_id
  + caller_id
  + script_or_method
  + node_path
  + input_values
  + output_values
  + property_diffs
  + emitted_signals
  + awaited_operations
  + child_invocations
```

推荐的 Agent 命令不是 `step_line`，而是：

- 查询某个 Node 在一次调用期间的状态变化；
- 查询某个 Signal 的发送者、接收者和参数；
- 查询某个 Resource 的加载和引用历史；
- 进入某一次函数调用或返回上一级调用；
- 比较两个调用实例的输入、输出和属性 Diff；
- 只对当前假设相关的函数开启细粒度追踪。

这直接支持本项目第 12 节的“高层 Editor Command Layer”和“自适应观测”。

### 18.3 TraceCoder：插桩、因果分析、历史学习和回滚

- **本地原文**：[TraceCoder_Trace_Driven_Debugging.pdf](research_papers/agent_debugging/03_TraceCoder_Trace_Driven_Debugging/TraceCoder_Trace_Driven_Debugging.pdf)
- **原文**：[arXiv:2602.06875](https://arxiv.org/abs/2602.06875)

#### 论文实际做了什么

TraceCoder 将流程拆成三个 Agent：

```text
Instrumentation Agent
→ Analysis Agent
→ Repair Agent
```

Instrumention Agent 根据测试失败和分析建议插入探针；Analysis Agent 从运行时 Trace 中找根因，并同时给出下一轮插桩建议；Repair Agent 按修复计划生成代码。框架还包含：

- **Historical Lesson Learning Mechanism**：保存失败修复计划、失败原因和通过测试数；
- **Rollback Mechanism**：失败修改不留在工作区，保证每轮从干净状态开始；
- **Instrumentation Purity**：插桩只能增加观测语句，不得改变业务逻辑；
- **TraceRunner**：隔离执行环境并结构化收集日志和异常。

#### 实验与限制

论文报告相对既有基线最多 34.43% 的 Pass@1 相对提升，并显示历史失败学习和迭代修复对结果有明显贡献。它的插桩主要面向 Python 代码，依赖 LLM 生成观测语句，因此不能直接当作 Godot 引擎级 Trace 实现。Godot 更适合由引擎提供不可变、类型安全的探针 API，让 Agent 只选择观测目标。

#### 对 Godot 的直接迁移

对应关系应为：

```text
Godot Observation Planner
→ Engine Trace Instrumentation
→ Causal Analysis Agent
→ Patch Agent
→ Isolated Run + Test
→ Commit / Rollback
```

其中 Observation Planner 可以根据“怀疑节点生命周期错误”“怀疑信号参数错误”“怀疑资源引用失效”等假设，选择不同的观测模板，而不是让 LLM 任意修改 GDScript 来插入 print。

### 18.4 DebugRepair：从结果级反馈升级到运行时中间状态

- **本地原文**：[DebugRepair_Self_Directed_Debugging.pdf](research_papers/agent_debugging/04_DebugRepair_Self_Directed_Debugging/DebugRepair_Self_Directed_Debugging.pdf)
- **原文**：[arXiv:2604.19305](https://arxiv.org/abs/2604.19305)

#### 论文实际做了什么

DebugRepair 先对失败测试做语义净化，提取最小测试上下文和依赖；随后识别关键变量并生成插桩版本；插桩必须满足两项检查：移除日志后与原程序等价，并且能够编译。运行后把中间状态、失败信息和上一轮 Patch 一起交给修复 Agent。

它还采用分层修复循环：一次调试 Session 内进行多轮 Patch；如果多轮仍失败，则丢弃旧对话，重新采集一份 Trace，避免错误上下文持续污染后续修复。

#### 对 Godot 的直接迁移

Godot 的“语义净化”可实现为最小可复现运行单元：

- 最小 Scene；
- 触发错误所需的 Node/Resource 子集；
- 必需的输入序列；
- 必需的随机种子和时间步；
- 必需的网络消息或异步资源结果。

“分层 Session”也非常适合游戏调试：如果同一批观测无法区分“场景切换错误”和“旧 Node 收到信号”，就重新选择观测假设，而不是继续让 Agent 在同一份长日志上循环。

### 18.5 Knowledge-Based Zero-Replay Debugging：有限重放预算

- **本地原文**：[Knowledge_Based_Zero_Replay_Debugging.pdf](research_papers/agent_debugging/05_Zero_Replay_Debugging/Knowledge_Based_Zero_Replay_Debugging.pdf)
- **原文**：[arXiv:2606.14805](https://arxiv.org/abs/2606.14805)

#### 论文实际做了什么

论文把多 Agent Trace 编译成事件知识图谱，节点包含路由、内存写入、工具调用、不确定性和证据。它不主张完全取消重放，而是预测哪些事件最值得消耗重放预算。实验中，Branch Recall@5 从 0.73 提升到 0.93，且不需要先运行昂贵的重放 Oracle。

#### 对 Godot 的直接迁移

Godot 的重放也应是有限预算资源。建议按以下顺序：

1. 从错误窗口构造候选事件；
2. 根据图中心性、依赖距离、状态不确定性和 Patch 关联度排序；
3. 只对 Top-k 事件恢复快照或重放；
4. 用重放结果更新根因置信度；
5. 达到预算后交给 Agent 选择修复方案。

这比“每个怀疑点都重新运行整个游戏”更适合实时游戏项目。

### 18.6 From Agent Traces to Trust：执行溯源综述

- **本地原文**：[Agent_Traces_to_Trust_Provenance.pdf](research_papers/agent_debugging/06_Agent_Traces_to_Trust_Provenance/Agent_Traces_to_Trust_Provenance.pdf)
- **原文**：[arXiv:2606.04990](https://arxiv.org/abs/2606.04990)

#### 论文实际贡献

这是一篇综述，不是单一调试系统。它把 Agent 的证据和执行溯源拆成：Trace 来源、执行单元、溯源关系、粒度、表示形式和信任功能，并讨论工具调用、记忆写入、环境观测、中间结论、动作和最终输出之间的血缘关系。

论文指出当前研究缺少统一 Trace Schema、语义级溯源、真实执行轨迹基准、恢复导向评估和隐私安全的审计基础设施。

#### 对 Godot 的直接迁移

本项目的 Patch Attribution 不能只记录“Agent 修改了哪个文件”，还要记录：

```text
需求 / 假设
→ 检索到的引擎文档
→ Agent 工具调用
→ 观测到的运行时证据
→ 生成的 Patch
→ 测试和性能证据
→ 最终提交或回滚
```

这会让系统能够回答：“为什么这个 Patch 被认为合理？”以及“这个结论依赖了哪个运行时观测？”

### 18.7 RepairAgent：工具选择、假设状态机和自动回滚

- **本地原文**：[RepairAgent_Autonomous_Program_Repair.pdf](research_papers/agent_debugging/07_RepairAgent_Autonomous_Program_Repair/RepairAgent_Autonomous_Program_Repair.pdf)
- **原文**：[arXiv:2403.17134](https://arxiv.org/abs/2403.17134)

#### 论文实际做了什么

RepairAgent 不使用固定的“生成 Patch—跑测试—再生成 Patch”循环，而是让 Agent 在状态机中选择工具：读取代码、获取测试、搜索相似调用、运行测试、故障定位、表达假设、收集更多信息、放弃假设和应用修复。应用 Patch 的工具会在测试失败时自动恢复工作区。

论文在 Defects4J 上修复了 164 个 Bug，并明确记录了较高的 Token 成本，说明自主工具使用提高能力的同时也必须进行预算控制。

#### 对 Godot 的直接迁移

Godot Agent 应具有显式的调试状态，而不是只依赖 Prompt：

```text
UnderstandFailure
→ FormHypothesis
→ CollectEngineEvidence
→ ApplyIsolatedPatch
→ RunBehaviorAndPerformanceTests
→ Commit / Reject
```

“表达假设”和“放弃假设”应成为正式 API，以便记录诊断过程和避免无意义重复调用。

### 18.8 Causal-Consistent Reversible Debugging：并发和异步回退

- **本地原文**：[Causal_Consistent_Reversible_Debugging.pdf](research_papers/agent_debugging/08_Causal_Consistent_Reversible_Debugging/Causal_Consistent_Reversible_Debugging.pdf)
- **原文**：[arXiv:2406.05719](https://arxiv.org/abs/2406.05719)

#### 论文实际贡献

论文研究 Erlang 并发程序的因果一致可逆调试：一个动作只有在所有依赖它的动作都撤销后才能被撤销。它把用户驱动调试和 Replay Debugging 统一到一个框架，并通过执行日志重放原始计算或因果等价计算。

#### 对 Godot 的直接迁移

Godot 中的异步加载、Signal、协程、RPC 和 WorkerThreadPool 都可能存在先后依赖。因此“回滚一个属性”并不等于“恢复了运行状态”。需要记录：

- 事件依赖；
- 创建和销毁关系；
- 消息发送与接收；
- 异步任务完成顺序；
- 随机种子和输入；
- 外部资源版本；
- 可撤销和不可撤销的副作用。

第一版可以只支持单线程、固定随机种子和有限的信号/输入模型，再逐步扩展到网络和多线程。

---

## 19. 综合比较：哪些论文应该优先用于 Godot Engine 原型

| 优先级 | 论文 | 应吸收的核心机制 | Godot 原型落点 |
|---|---|---|---|
| P0 | AgentTrace | 因果图、反向追踪、可解释排序、无 LLM 诊断 | `CausalStateGraph`、根因 Top-k |
| P0 | Agent-centric Debugging Interface | 函数/对象级 Trace、高层调试命令、按需细化 | `InvocationTrace`、对象查询 API |
| P0 | TraceCoder | 目标插桩、分析/修复分工、历史教训、回滚 | Observation Planner + Patch Loop |
| P1 | DebugRepair | 最小测试上下文、运行时中间状态、重新采集 Session | Failure Minimizer + Re-debug Session |
| P1 | Zero-Replay Debugging | 重放预算分配、知识图谱、候选分支排序 | Checkpoint/Replay Scheduler |
| P1 | RepairAgent | 假设状态机、工具选择、隔离 Patch 和自动撤销 | Agent Debug State Machine |
| P2 | Agent Traces to Trust | 执行溯源、证据血缘、统一 Schema、审计 | Provenance Graph |
| P2 | Reversible Debugging | 因果一致回退、异步依赖、可逆事件模型 | Reversible Event Log |

### 结论

后续不应先修改 Godot Core，而应先在 Engine 外围实现可验证原型：

```text
EditorPlugin / GDExtension
+ structured trace collector
+ semantic debug API
+ causal graph query
+ isolated patch runner
+ deterministic test harness
```

当原型证明以下指标有改善后，再考虑进入 Godot Engine 核心：

- 根因 Top-1/Top-3 命中率；
- 自动修复成功率；
- 回归率和非目标修改数；
- 平均修复轮数；
- Token 与工具调用次数；
- Trace 和插桩运行时开销；
- 重放成功率；
- 人工介入次数。

当前最小可行研究系统应定义为：

> 一个能记录 Agent Patch、Godot 对象状态变化和运行时错误，并从错误窗口反向返回 Top-k 候选根因、生成针对性观测、在隔离环境验证 Patch、失败自动回滚的 Godot Agent 调试原型。

这才是后续研究“AI 原生游戏引擎”的 Engine 基础，而不是简单增加一个能够点击编辑器的 MCP Agent。

---

## 20. 论文独立研究文档索引

每篇论文的 PDF 和分析文档位于同一个目录：

1. [AgentTrace：因果图追踪与多 Agent 根因分析](research_papers/agent_debugging/01_AgentTrace_Causal_Graph/analysis.md)
2. [Agent-centric Debugging Interface：面向 Agent 的动态调试接口](research_papers/agent_debugging/02_Agent_Centric_Debugging_Interface/analysis.md)
3. [TraceCoder：Trace 驱动的多 Agent 自动调试](research_papers/agent_debugging/03_TraceCoder_Trace_Driven_Debugging/analysis.md)
4. [DebugRepair：自指导调试增强自动程序修复](research_papers/agent_debugging/04_DebugRepair_Self_Directed_Debugging/analysis.md)
5. [Knowledge-Based Zero-Replay Debugging：有限重放预算](research_papers/agent_debugging/05_Zero_Replay_Debugging/analysis.md)
6. [From Agent Traces to Trust：执行溯源与证据血缘](research_papers/agent_debugging/06_Agent_Traces_to_Trust_Provenance/analysis.md)
7. [RepairAgent：自主式 LLM 程序修复](research_papers/agent_debugging/07_RepairAgent_Autonomous_Program_Repair/analysis.md)
8. [Causal-Consistent Reversible Debugging：因果一致可逆调试](research_papers/agent_debugging/08_Causal_Consistent_Reversible_Debugging/analysis.md)
