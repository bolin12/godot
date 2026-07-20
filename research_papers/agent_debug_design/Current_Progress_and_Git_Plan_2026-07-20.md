# Godot Agent Debugger 当前进展与 Git 追踪方案

日期：2026-07-20

## 1. 当前结论

当前实验分支已经完成 Gate A–D 的第一个可运行纵向切片：

```text
Import
→ Native Parse
→ Artifact Dependency Graph
→ Semantic Contract
→ Runtime Contract / AgentTrace
```

它已经足以支持 Dodge the Creeps 的资源、场景结构和基础运行状态修改，并能从命令行输出机器可读的验证结果。

尚未完成：

- 持久 Validation Receipt cache；
- 真正的局部图重算；
- 引擎 Signal/Callable 热路径自动 Hook；
- Runtime 失败注入与完整 backward cause ranking；
- Render Manifest；
- D3D12、Vulkan、OpenGL 图像 Gate。

## 2. 已完成的引擎能力

### AgentTrace 与因果图

- 有界 Ring Buffer；
- Session、Event ID、Frame、Physics Frame 和 Thread ID；
- 显式 parent event；
- typed causal edge；
- Failure Window；
- backward cause query；
- causal edge 结构化导出。

### Artifact Graph

- ResourceLoader 原生递归依赖发现；
- Resource UID 优先的 Artifact ID；
- SHA-256 内容 Hash；
- Native Parse Validation Receipt；
- 反向受影响范围查询；
- 按变更路径生成 invalidation plan；
- 排除时间戳和 Receipt ID 的确定性 graph fingerprint。

### Semantic Contract

- PackedScene 加载；
- 必需 Node 与兼容类型；
- Group；
- 已保存 Signal connection；
- InputMap action；
- assertion 级失败详情。

### Runtime Contract

- 在独立 headless SceneTree 中加载并实例化真实主场景；
- 等待 `_ready`；
- 调用真实 `new_game()`；
- 采集 State Snapshot；
- 发出真实 `Player.hit`；
- 验证 Timer、Player、Collision 与 HUD 状态；
- 输出 Event、Causal Edge、Failure Window 和 Cause Candidate 数据。

当前 Signal delivery 是“已保存 connection + 状态观测”的推断证据，尚不是引擎方法入口自动 Hook，因此记录为 `inferred / confidence 0.9`。

## 3. 最新验证结果

### 引擎模块

```text
test cases: 8 passed, 0 failed
assertions: 50 passed, 0 failed
```

### Artifact Graph

```json
{
  "artifact_count": 21,
  "dependency_count": 20,
  "receipt_count": 21,
  "failed_receipt_count": 0,
  "graph_fingerprint": "d7d2cc8e7866c1b7dfa885ad4e67d0d1e5636047ff3be0f05fd034a6fc87852b"
}
```

### Semantic Contract

```json
{
  "receipt_count": 5,
  "failed_receipt_count": 0,
  "result": "pass"
}
```

### Semantic Fault Matrix

五种故障均在精确 assertion 上被捕获：缺失 Node、错误类型、缺失 Group、缺失 connection、缺失 InputMap action。

```json
{
  "case_count": 5,
  "failed_case_count": 0,
  "result": "pass"
}
```

### Incremental Invalidation Plan

`player.gd` 的变更闭包：

```text
player.gd → player.tscn → main.tscn
```

规划结果与全量依赖图一致，两次全量扫描 fingerprint 一致。当前只证明 invalidation planning，尚未声称真正的缓存局部重算。

### Runtime Contract

```json
{
  "assertion_count": 9,
  "failed_assertion_count": 0,
  "event_count": 15,
  "edge_count": 13,
  "result": "pass"
}
```

连续执行三次结果一致。

## 4. 已知问题

`--headless --import` 完成且退出码为 0，但在当前 Godot `4.8.dev` 和编辑器同时运行的现场中，退出清理阶段曾报告：

- `EditorNode singleton null`；
- Thread 未完成 join；
- 少量 ObjectDB leak。

导入后的 Artifact、Semantic 和 Runtime Gate 全部通过，因此暂未归类为项目内容错误。关闭编辑器后应单独复测 Import Gate，以区分并发占用与 dev 构建退出问题。

另外，打开/导入 Godot 4.7 Demo 后，上游仓库出现自动升级变更：

- `project.godot` feature tag 从 `4.7` 更新为 `4.8`；
- 11 个 `.import` 文件新增 `compress/high_quality_mode=0`。

这些变更不属于 Agent Debugger 实现，不应混入主仓库提交。

## 5. Git 追踪分类

### 应追踪并提交

| 路径 | 理由 |
|---|---|
| `.gitignore` | 防止大型上游仓库、PDF 和临时产物误提交 |
| `modules/agent_debugger/**` | 引擎模块源码、注册、构建配置、fixtures 与单元测试 |
| `agent_research_projects/README.md` | 实验项目来源、固定 commit 和工作区纪律 |
| `agent_research_projects/tools/*.gd` | 可重复执行的 Artifact、Semantic、Fault、Incremental、Runtime CLI |
| `research_papers/agent_debug_design/*.md` | 协议、Schema、实施步骤和验证记录 |
| `research_papers/**/*.md` | 自有研究分析、总体方案和社区调研 |
| `research_papers/godot_ai_plugins/THIRD_PARTY_NOTICES.md` | 第三方来源与许可证记录 |

### 不应追踪

| 路径 | 理由 |
|---|---|
| `bin/`、`bin/obj/` | 可重建二进制和目标文件，Godot 已默认忽略 |
| `agent_research_projects/upstream/` | 约 366 MB 的独立 Git 仓库，应由固定 URL/commit 重建 |
| `research_papers/godot_ai_plugins/sources/` | 社区项目的独立 Git 快照，不应嵌入主仓库 |
| `research_papers/agent_debugging/**/*.pdf` | 原始论文体积与再分发风险；提交分析和引用即可 |
| `tmp/` | PDF 文本提取等临时产物 |
| Demo `.godot/` | Godot 导入缓存，已由上游 `.gitignore` 忽略 |
| `*.agent_contract.exe` | 独立验证构建产物，已由根仓库 `bin/` 规则忽略 |

### 暂不提交、需要单独处理

`agent_research_projects/upstream/godot-demo-projects` 是独立 Git 仓库。当前 12 个变更来自 Godot 4.8 自动升级，应在确认无需保留后于该嵌套仓库单独恢复，不能与 Agent Debugger 代码一起 stage。

不要在根仓库直接执行：

```text
git add agent_research_projects/upstream
```

否则可能把嵌套仓库误记录为 embedded repository/gitlink。

## 6. 推荐提交拆分

建议至少拆成四个可审查提交：

1. `Add AgentTrace event store and causal graph`
   - Trace types、Ring Buffer、Causal Graph、AgentTrace API。
2. `Add artifact and semantic validation gates`
   - Artifact Graph、Validation Receipt、Contract Validator、fixtures 和 tests。
3. `Add Dodge CLI validation harnesses`
   - Artifact、Semantic、Fault Matrix、Incremental Plan、Runtime scripts。
4. `Document agent-native debugging architecture and results`
   - 研究分析、协议、实施记录、第三方 notices 和 `.gitignore`。

提交前再次执行：

```powershell
bin\godot.windows.editor.x86_64.agent_contract.console.exe `
  --test --test-case="*[AgentDebugger]*" --no-colors
```

并分别运行 Semantic、Fault Matrix、Incremental Plan 和 Runtime Contract。

## 7. 下一阶段建议

先关闭当前编辑器，使用 `agent_contract` 编辑器重新打开并复测 Import Gate。确认进程清理问题后，再实现 Runtime 负路径和 Artifact/Receipt/Event ID 对齐；随后才进入引擎级自动 Hook 与 Render Manifest。
