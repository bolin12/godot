# Godot 社区 AI 与 Agent 插件调研

## 1. 调研目的和范围

本调研用于判断 Godot 社区现有 AI、Agent 和 MCP 项目已经解决了哪些问题，以及“Agent 原生引擎调试层”还需要补足哪些能力。

调研日期为 **2026-07-20**。热度来自调研当日 GitHub 公开数据，只作为筛选信号，不代表代码质量。所有源码均以浅克隆方式保存在本目录的 `sources/` 下，没有安装到项目、没有启用插件，也没有执行第三方脚本。

本次选择覆盖以下类型：

- GitHub 热度最高的 Godot MCP 项目；
- Godot 官方 Asset Library 中近期活跃的 AI Assistant/MCP 插件；
- 编辑器内 Agent、权限与 Undo 实现；
- Runtime 控制、截图、输入和调试器桥；
- 原生/无 Sidecar MCP 实现；
- 小型 HTTP 控制接口，用作最小基线。

## 2. 下载快照

| 项目 | 调研时 Stars | 本地目录 | Commit | 类型 |
|---|---:|---|---|---|
| [Coding-Solo/godot-mcp](https://github.com/Coding-Solo/godot-mcp) | 4806 | `sources/coding-solo_godot-mcp` | `1209744fad78` | 外部 MCP/CLI |
| [hi-godot/godot-ai](https://github.com/hi-godot/godot-ai) | 1073 | `sources/hi-godot_godot-ai` | `f94116ce0f75` | EditorPlugin + Python MCP Server |
| [ee0pdt/Godot-MCP](https://github.com/ee0pdt/Godot-MCP) | 597 | `sources/ee0pdt_godot-mcp` | `b7d08eb43ac9` | EditorPlugin + Node MCP Server |
| [youichi-uda/godot-mcp-pro](https://github.com/youichi-uda/godot-mcp-pro) | 515 | `sources/youichi-uda_godot-mcp-pro` | `c17a182d92f2` | 大工具集 EditorPlugin |
| [yurineko73/Godot-MCP-Native](https://github.com/yurineko73/Godot-MCP-Native) | 427 | `sources/yurineko73_godot-mcp-native` | `52d978d4c27b` | Godot 内原生 MCP/Debugger Bridge |
| [tomyud1/godot-mcp](https://github.com/tomyud1/godot-mcp) | 395 | `sources/tomyud1_godot-mcp` | `0d3f473922ea` | Asset Library MCP + Visualizer |
| [FlamxGames/godot-ai-assistant-hub](https://github.com/FlamxGames/godot-ai-assistant-hub) | 293 | `sources/flamxgames_ai-assistant-hub` | `5f3bb865bacb` | 编辑器内多模型 Assistant |
| [Erodenn/godot-mcp-runtime](https://github.com/Erodenn/godot-mcp-runtime) | 47 | `sources/erodenn_godot-mcp-runtime` | `92be97396533` | 零驻留 Runtime Bridge |
| [Godot4-Addons/ai_assistant_for_godot](https://github.com/Godot4-Addons/ai_assistant_for_godot) | 35 | `sources/godot4-addons_ai-assistant` | `e460f1f63a8e` | 编辑器内 Agent/权限/备份 |
| [sancheznot/Godot-AI-Assistant](https://github.com/sancheznot/Godot-AI-Assistant) | 7 | `sources/sancheznot_godot-ai-assistant` | `37b539165bf8` | Golem-AI 多模型 Agent |
| [ULT7RA/AIEssentialsToolkit](https://github.com/ULT7RA/AIEssentialsToolkit) | 0 | `sources/ult7ra_ai-essentials-toolkit` | `ad18111ce222` | 最小 HTTP 编辑器/游戏控制桥 |

以上仓库当前快照均包含 MIT License。引用或移植具体实现时仍应保留原作者版权和许可证文本，并在实际合入前重新检查上游许可证变化。

## 3. 总体结论

社区已经较好解决：

```text
项目/场景/节点读取
→ 编辑器结构化修改
→ 脚本写入与语法检查
→ 运行项目
→ 读取日志/错误
→ 截图和输入模拟
→ Agent 再次修改
```

社区尚未形成成熟实现：

```text
Agent Patch Attribution
→ Object/Resource 生命周期追踪
→ Method/Signal/Deferred/RPC 因果边
→ Failure-window Trace
→ Semantic State Diff
→ Top-k Root Cause Ranking
→ Targeted Instrumentation
→ Deterministic Replay
→ Behavior + Performance Transaction
```

因此，社区项目不会使本研究方向重复。它们主要覆盖 **Agent Control Plane** 和 **Editor Command Layer**，而本研究应重点建设 **Runtime Evidence Layer、Causal Analysis Layer 和 Transactional Verification Layer**。

## 4. 逐项目分析

### 4.1 Coding-Solo/godot-mcp

这是调研时 GitHub 热度最高的 Godot MCP 项目。它采用外部 TypeScript MCP Server，通过 Godot 命令行和打包的 `godot_operations.gd` 完成项目运行、调试输出采集、场景创建、节点添加和资源保存。

优点：

- 安装和使用路径简单；
- 不要求长期驻留 EditorPlugin；
- 将复杂 Godot 操作集中到一个 GDScript 操作脚本；
- 能形成“写入—运行—读取 Debug Output”的基础闭环；
- 适合作为普通日志 Baseline。

不足：

- 不具备持续的 Live Editor/Runtime 会话；
- 没有对象生命周期、Signal 或状态变化历史；
- 没有 UndoRedo、事务 Patch 或隔离验证；
- 调试证据仍以进程输出为主；
- 不支持失败窗口和因果查询。

可借鉴：把 Headless/CLI 操作保留为无编辑器依赖的降级通道和 Benchmark Baseline。

### 4.2 hi-godot/godot-ai

这是本次调研中工程完整度最高、测试覆盖最强、与控制面需求最接近的项目。架构为：

```text
MCP Client
→ Python FastMCP Server
→ WebSocket
→ Persistent Godot EditorPlugin
→ Game Helper
```

它使用约 43 个 MCP Tool 暴露 120 多个底层操作，并通过 `<domain>_manage(op, params)` 把大量低频工具折叠成领域工具，避免工具数量过多导致模型能力下降。

值得借鉴的实现：

- 持久 Editor 会话和多 Editor Session 路由；
- Typed Protocol Envelope 和结构化错误；
- `batch_execute` 原子批处理，失败时通过 UndoRedo 回退；
- Node、Scene、Resource、Signal 等写操作统一接入 UndoRedo；
- Editor/Game 日志分流、`run_id` 和增量 Cursor；
- Ring Buffer 溢出检测和 `truncated` 语义；
- 游戏 Helper 存活状态，而不是只相信编辑器“正在运行”按钮；
- Parse/Load Break 状态识别；
- 测试运行、性能 Snapshot、截图和输入；
- Tool Domain 裁剪；
- 鉴权 Token、Origin Guard、输入边界和 Circuit Breaker；
- 大量单元、集成和 Godot 项目内测试。

不足：

- 日志 Cursor 是可靠的事件消费机制，但事件之间没有游戏语义因果边；
- 属性 Snapshot 主要表示当前状态，不表示“为什么变成该状态”；
- Batch Undo 解决编辑器操作原子性，但不等于文件、运行时状态和外部副作用的统一事务；
- 没有 Patch ID 到运行时事件、测试结果和性能变化的完整关联；
- 没有确定性重放、失败窗口、根因排序和多根因模型。

建议：控制面协议、Session Registry、结构化错误、Ring Cursor、批处理事务和测试体系应重点参考该项目；不要重新发明一套质量更低的 MCP 编辑器桥。

### 4.3 ee0pdt/Godot-MCP

这是较早期的 EditorPlugin + WebSocket + Node MCP Server 实现。其命令处理按 Node、Scene、Script、Project 和 Editor 分类。

优点：

- 架构简单，便于理解最小 EditorPlugin 桥；
- Command Processor 分层清晰；
- 提供 `godot://scene/current` 等 Resource 思路。

不足：

- 当前活跃度和工具深度明显落后于新项目；
- 缺少 Runtime Helper、系统化测试和成熟事务；
- 不适合作为新架构的主基线。

可借鉴：只适合作为早期协议演进参考。

### 4.4 youichi-uda/godot-mcp-pro

该项目强调 175 个工具，覆盖编辑、运行时、输入、动画、资源、物理、导航、音频、性能和 Testing/QA。

值得借鉴：

- Editor UndoRedo 广泛覆盖节点和属性修改；
- Editor/Game Screenshot；
- Runtime SceneTree、节点属性、输入和多帧捕获；
- FPS、内存、Draw Call 等性能检查；
- Test Scenario、Assertion、Stress Test 和 Screenshot Compare；
- Lite/Minimal Tool Mode，承认工具数量会影响 Agent 表现；
- JSON-RPC 错误代码与后续建议。

局限：

- 175 个平铺工具会明显增加 Tool Schema 和选择成本；
- 所谓 Record/Replay 主要是工具层输入或运行控制，不能等同于引擎确定性重放；
- QA 工具仍以结果检查为主，没有中间状态因果链；
- UndoRedo 不覆盖脚本文件、异步运行和外部副作用的整体事务。

可借鉴：Testing/QA 工具的产品形态、性能预算接口和 Tool Profile；接口设计上应优先采用领域聚合，而不是复制 175 个平铺工具。

### 4.5 yurineko73/Godot-MCP-Native

这是与本研究技术接入点最接近的项目。它将 MCP HTTP Server、Bearer Token 鉴权、工具注册和 Editor Debugger Bridge 放在 Godot 插件侧，避免依赖 Node/Python Sidecar。

值得借鉴：

- 原生 HTTP MCP Server 和 Bearer Token；
- EditorDebuggerPlugin 会话、Break/Resume/Stop 状态；
- Breakpoint、Stack Frame、Local/Member/Global Variable；
- DAP 风格 Thread、Scope 和 Variable Reference；
- 自定义 EngineDebugger Message Prefix 捕获；
- Runtime Probe 的安装、卸载、SceneTree、属性和方法调用；
- Runtime Assertion、Condition Wait、Performance Snapshot 和 Memory Trend；
- Input、Animation、Shader、TileMap 和 Audio 的运行时操作；
- Project Test Runner 和像素级 Render Regression；
- 批量 Scene/Node 操作进入同一个 UndoRedo Action；
- 大量 GDScript 单元测试和 Python 集成测试。

不足：

- Debugger State Event 是控制状态历史，不是 Gameplay 因果图；
- Stack/Variable 接口仍然接近传统逐行调试器，Agent 调用成本较高；
- Runtime Probe 查询当前状态，但没有对象完整生命周期或方法调用生命周期；
- 自定义消息虽然可扩展，但没有统一 Trace Schema；
- 没有 Patch Attribution、Failure Window、Root Cause Ranker 和确定性重放。

建议：优先研究并复用其 EditorDebuggerPlugin Bridge、Runtime Probe、鉴权、Test Runner 和 Render Regression 设计。它可以显著缩短 Phase 1 的外围调试桥建设，但核心 Trace Schema 应独立设计。

### 4.6 tomyud1/godot-mcp

这是 Godot Asset Library 中较活跃的 MCP 插件，采用：

```text
MCP Client
→ Node.js MCP Server
→ WebSocket :6505
→ Godot EditorPlugin
```

特色包括 42 个工具、脚本关系图、Scene Graph Explorer、代码/场景可视化编辑、运行/停止、错误读取和 Runtime Helper。

值得借鉴：

- 项目脚本关系图和 Find Usages；
- 浏览器内交互式 Scene/Script 可视化；
- Script Rename/Move 时更新引用；
- Runtime Helper 与编辑器工具统一路由。

明确局限：项目 README 说明当前修改直接保存，**没有 Undo**，需要依赖版本控制。

可借鉴：因果图可视化不必从零开始设计 UI，可以参考其项目图和场景图交互；写操作实现不应直接采用。

### 4.7 FlamxGames/godot-ai-assistant-hub

这是编辑器内多模型 Assistant，支持 Ollama、Gemini、OpenRouter 等 Provider，并允许为不同 Assistant 配置不同 Tool Access Profile。

最值得借鉴的是权限系统：

- `Allow`：直接允许；
- `Ask`：执行前请求用户确认；
- `Hide`：不向模型暴露工具，同时减少 Tool Context；
- 每个 Assistant 使用独立权限 Profile；
- Tool Option 可以限制文件、节点属性或禁止生成特定关键字；
- 只读工具默认 Allow，写工具默认 Ask；
- Tool 级 Undo，使用最多 20 条的 LIFO Undo Queue。

局限：

- Undo Queue 依赖 Tool 自己实现 `undo()`，不是统一事务；
- Execution ID 主要来自毫秒时间，不适合作为跨进程全局事件标识；
- 不覆盖运行时状态、测试证据和性能回归；
- 缺少自动化测试目录。

建议：把 Allow/Ask/Hide、按 Agent 配置、Tool Option 和隐藏工具降低 Token 的思路迁移到 Agent Capability Manifest；底层改成正式 Patch ID、权限范围和事务日志。

### 4.8 Erodenn/godot-mcp-runtime

这个项目的定位类似“Godot 版 Playwright”。它不要求项目永久安装 Addon，而是在运行前临时注入 Autoload Bridge，结束后清理，从而提供：

- Headless Scene/Node/Script 编辑；
- Runtime SceneTree 和 UI Element 发现；
- 截图；
- 输入模拟；
- Live GDScript 执行；
- Background 和 Manual Attach 模式；
- 结构化输出和较完整测试。

安全方面，它在启动前扫描项目和被挂接脚本，限制高风险 GDScript，并实现路径验证和 Runtime Artifact 生命周期管理。

值得借鉴：

- 临时 Runtime Bridge，不污染用户项目；
- Bridge 注入/移除的生命周期；
- Runtime Tool 的结构化输出；
- UI Discovery 和输入模拟；
- 高风险脚本扫描；
- 将 Agent Runtime Testing 类比为 Playwright，而不是人工 Playtest 替代品。

局限：

- Live GDScript 执行仍然具有较大权限和观测扰动风险；
- 没有 Engine 内对象/调用级 Trace；
- Runtime Bridge 被移除后不会留下统一因果证据；
- 没有 Patch 事务或根因定位。

建议：有限重放和隔离验证阶段可以借鉴其临时 Bridge；生产方案应以类型安全 Probe 取代任意 Live Script。

### 4.9 Godot4-Addons/ai_assistant_for_godot

这是编辑器内 Agent 原型，包含 Tool Registry、Agent Loop、Memory、Permission Manager、Syntax Checker、Damage Repair 和 Rollback Manager。

值得借鉴：

- Tool 调用和结果进入 Working Memory；
- Agent Loop Guard 防止重复工具循环；
- Tool/Path 权限；
- 关键路径请求确认；
- 文件修改前备份；
- 写入后语法检查；
- 常见 Patch 失败、文件不存在和悬挂引用的自动修复分类。

源码审查发现这些能力仍偏原型：

- `patch_file` 默认是 `AUTO_ALLOW`，与破坏性风险不完全匹配；
- 文件备份使用文件名构造 Backup Path，不同目录同名文件可能冲突；
- 空文件因为内容为空而不会创建备份；
- 备份回滚不是多文件原子事务；
- Syntax Checker 大量使用文本启发式，不能替代引擎 Parser、Import 和运行测试；
- 没有把 Hypothesis、Evidence、Patch 和 Validation 做成统一状态机对象。

建议：Agent Loop Guard、Working Memory 和 Permission Manager 可作为交互参考，但修复事务和验证系统需要重新设计。

### 4.10 sancheznot/Godot-AI-Assistant（Golem-AI）

Golem-AI 提供多模型聊天、Skills、项目索引、文档搜索、Web Search、资源下载、插件互操作和多步 Tool Agent。

值得借鉴：

- Core/Full Harness：默认只暴露脚本和场景能力；
- 最小 Bootstrap，项目上下文按需查询；
- Hybrid Project Index 与 Godot Docs Search；
- Skills 作为 Markdown 能力包；
- Anti-hallucination Guard：没有成功 Tool Call 时阻止声称“已经修改”；
- 结束前强制执行 `get_script_errors`；
- 与其他插件的工具发现和调用。

局限：

- “执行过 Tool”和“修改正确”仍是两件事；
- 强制检查脚本错误不能覆盖行为和性能回归；
- 没有失败运行时的中间状态和因果证据；
- 没有正式 Patch Transaction。

建议：最小上下文、Harness Profile、Skill 和 Anti-hallucination Guard 应进入外部 Agent Orchestrator；验证门槛应升级为行为、资源完整性和性能联合验证。

### 4.11 ULT7RA/AIEssentialsToolkit

这是非常小的双 HTTP Server 原型：Editor Server 监听 `127.0.0.1:7777`，Game Autoload Server 监听 `127.0.0.1:7778`。

接口包括：

- Ping；
- Editor/Game Screenshot；
- SceneTree Dump；
- Node Property Read/Write；
- Run/Stop；
- 全项目 GDScript Parse Check。

优点：代码极少，协议直观，适合作为最小演示和对照基线。

明显问题：

- 没有鉴权或每请求 Token；
- `scene/set` 直接调用 `node.set()`，不进入 UndoRedo；
- Property 序列化主要使用字符串；
- Parse Check 只能判断 ResourceLoader 是否加载成功；
- 没有请求审计、事务、权限和 Runtime 状态查询；
- Game Server 只能截图。

建议：可用作“最小 Editor HTTP Bridge”教学基线，不应直接作为研究系统基础。

## 5. 能力矩阵

符号：`✓` 表示已有较完整支持，`△` 表示部分或原型支持，`—` 表示未发现。

| 项目 | Editor CRUD | Runtime | Screenshot/Input | Undo/权限 | 测试/验证 | 结构化调试 | 因果 Trace |
|---|---:|---:|---:|---:|---:|---:|---:|
| Coding-Solo | ✓ | △ | — | — | △ | △ | — |
| hi-godot | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| ee0pdt | ✓ | — | — | △ | — | △ | — |
| MCP Pro | ✓ | ✓ | ✓ | ✓ | ✓ | △ | — |
| MCP Native | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |
| tomyud1 | ✓ | ✓ | ✓ | — | △ | △ | — |
| AI Assistant Hub | ✓ | — | — | ✓ | — | △ | — |
| MCP Runtime | ✓ | ✓ | ✓ | △ | ✓ | △ | — |
| AI Coding Assistant | ✓ | △ | — | △ | △ | △ | — |
| Golem-AI | ✓ | △ | △ | △ | △ | △ | — |
| AI Essentials | ✓ | △ | ✓ | — | △ | — | — |

这里的“结构化调试”表示结构化 Error、Stack、Variable、Log Cursor 或 Runtime Inspector，不代表因果调试。

## 6. 建议直接借鉴的机制

### 6.1 控制面和协议

优先参考 `hi-godot/godot-ai`：

- Server 负责编排，EditorPlugin 负责直接编辑器操作；
- Typed Envelope；
- Session ID 和多编辑器路由；
- Tool Domain Rollup；
- 结构化错误、错误代码和修复建议；
- 请求超时、Circuit Breaker 和 Server 生命周期。

### 6.2 Debugger Bridge

优先参考 `Godot-MCP-Native`：

- EditorDebuggerPlugin Session；
- EngineDebugger Capture Prefix；
- Stack、Scope、Variable 和 Break State；
- Runtime Probe；
- Test Runner 与 Render Regression。

现有能力应被包装为高层语义查询，而不是原样把逐行调试暴露给 Agent。

### 6.3 修改事务和权限

组合参考：

- `hi-godot` 的 `batch_execute + UndoRedo rollback`；
- `AI Assistant Hub` 的 Allow/Ask/Hide 和按 Agent Capability Profile；
- `Godot MCP Pro/Native` 的批量 UndoRedo；
- `AI Coding Assistant` 的路径保护和备份概念。

本研究需要在此基础上增加文件、资源、运行测试和性能证据的跨层事务。

### 6.4 Runtime 隔离桥

参考 `godot-mcp-runtime` 的临时 Autoload 注入和清理机制。它适合 Phase 1/3 的验证运行，但正式观测探针应类型安全、不可修改业务逻辑，并具备 Trace Budget。

### 6.5 Agent 上下文和工具治理

参考：

- `hi-godot` 的 Tool Rollup；
- `AI Assistant Hub` 的 Hide Tool；
- `Golem-AI` 的 Core/Full Harness、Minimal Bootstrap 和按需 Project Index；
- `Golem-AI` 的 Anti-hallucination Guard；
- `Coding Assistant` 的 Loop Guard。

目标是减少工具描述和无关项目上下文，而不是把所有能力一次性塞给模型。

### 6.6 可视化

参考 `tomyud1/godot-mcp` 的 Project Map、Script Relationship 和 Scene Graph Explorer。未来 Causal Graph UI 可以复用其交互范式，但图中必须区分显式因果、推断因果和单纯时间相邻。

## 7. 必须自行补足的核心能力

以下部分没有发现可直接采用的成熟社区实现：

1. 统一、版本化的 Godot Agent Debug Trace Schema；
2. Patch、Hypothesis、Evidence、Validation 的全链路归因；
3. Object/Node/Resource 的创建、引用、加载、卸载和销毁历史；
4. Method Invocation Lifetime Trace；
5. Signal Emit/Receive、Deferred Schedule/Execute、RPC 和 Thread Task 因果边；
6. Failure-window Ring Buffer 冻结和导出；
7. Semantic Snapshot 与 State Diff；
8. Semantic Runtime Assertion；
9. 从 Failure 反向查询的 Top-k Root Cause Ranker；
10. 按假设开启的 Targeted Instrumentation；
11. 输入、随机数、时间步、异步完成顺序和资源版本的确定性记录；
12. 有限 Replay Budget；
13. 文件、资源、行为、性能和外部副作用的统一 Patch Transaction；
14. 面向 Agent Debug 的 Godot Benchmark。

这些能力正是引擎层研究的主要贡献，不应被 MCP Tool 数量或聊天 UI 掩盖。

## 8. 对整体改造方案的修正

社区调研后，建议调整原方案的建设优先级：

### 不再优先自行建设

- 通用 MCP Server 框架；
- 基础 Scene/Node/Resource CRUD；
- 基础 Run/Stop/Screenshot/Input；
- 传统 Stack/Variable Debugger；
- 普通编辑器聊天 UI；
- Provider 适配层。

这些能力已经有多个成熟参考，可选择复用、适配或建立兼容层。

### 仍然必须优先建设

```text
Trace Schema
→ Runtime Semantic Hooks
→ Failure Window
→ Causal Graph
→ Root Cause Query
→ Semantic Assertion
→ Patch Attribution
→ Isolated Validation
```

### 推荐的新集成策略

```text
现有 MCP/Editor Plugin
        │
        │ 兼容 Adapter
        ▼
Agent Semantic Debug Protocol
        │
        ▼
Godot Agent Debug Module
        │
        ├─ Trace / Snapshot / Assertion
        ├─ Causal Query
        └─ Replay / Validation
```

这样既能直接利用社区的编辑器控制能力，又不会把研究绑定到某个插件。

## 9. 第一批建议验证的社区组件

按研究价值排序：

1. **Godot-MCP-Native Debugger Bridge**：验证 EditorDebuggerPlugin、EngineDebugger Message 和 Runtime Probe 是否足以承载 Phase 1；
2. **hi-godot Atomic Batch + Log Cursor**：验证批量编辑回滚和失败窗口事件消费；
3. **godot-mcp-runtime Transient Bridge**：验证隔离运行、截图、输入和最小 Runtime Query；
4. **AI Assistant Hub Permission Profile**：设计 Agent Capability Manifest；
5. **hi-godot Test Runner / Native Render Regression**：建立行为和视觉验证接口；
6. **tomyud1 Visualizer**：设计 Causal Graph Explorer；
7. **Golem-AI Harness/Guard**：设计 Agent 调试状态机和完成条件。

验证时先写 Adapter 和最小实验，不把这些插件整体合并到 Godot Fork。

## 10. 风险提示

- 部分项目允许执行任意 GDScript 或写入任意属性，不能直接用于无人监管模式；
- Localhost 不等于安全，仍应使用 Token、Origin/Host 限制和请求审计；
- UndoRedo 只覆盖编辑器对象历史，不自动覆盖文件系统、运行时副作用或外部服务；
- Screenshot 相似不等于行为正确；
- “项目正在运行”不等于游戏成功进入可交互状态；
- Tool 数量越多不一定越强，可能增加 Token、Schema 解析和错误工具选择；
- 社区仓库变化较快，后续实现应固定 Commit 并记录许可证；
- 下载的源码仅用于研究，未经审计不得在真实项目中启用。

## 11. 最终判断

当前 Godot 社区已经进入“Agent 可以较完整地操作编辑器和运行项目”的阶段，但还没有进入“引擎能够向 Agent 解释失败为什么发生”的阶段。

因此最合理的研究定位是：

> 不再重复建设另一个大而全的 Godot MCP，而是在现有 MCP/Editor Control 生态之下，建设可跨插件复用的 Agent-oriented Runtime Observability、Causal Debugging 和 Transactional Verification 基础层。
