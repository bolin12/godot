# Godot Agent 原生引擎整体改造方案

## 1. 研究目标

本方案的目标不是为 Godot 增加一个能够点击编辑器或生成脚本的聊天界面，而是研究游戏引擎如何为 Autonomous Agent 提供一套可观测、可查询、可验证、可恢复且运行时开销可控的开发环境。

建议将最小完整闭环定义为：

```text
Agent Patch
→ 运行时语义追踪
→ 失败窗口冻结
→ 因果图反向查询
→ Top-k 根因候选
→ 定向补充观测
→ 隔离修复
→ 行为与性能验证
→ Commit 或 Rollback
```

该方向主要吸收以下研究机制：

- AgentTrace：从失败节点反向查询因果图，并使用可解释特征进行根因排序；
- Agent-centric Debugging Interface：以调用和对象生命周期为调试单元，按需提高观测粒度；
- TraceCoder：将插桩、分析、修复与回滚组成正式闭环；
- DebugRepair：使用运行时中间状态和最小失败上下文，而不是只依赖错误文本；
- Knowledge-Based Zero-Replay Debugging：将重放视为有限预算资源；
- RepairAgent：使用显式假设状态机、工具选择和失败 Patch 自动撤销；
- From Agent Traces to Trust：记录需求、假设、证据、Patch 和验证结果之间的血缘；
- Causal-Consistent Reversible Debugging：异步和并发状态必须按因果依赖回退。

## 2. 总体设计原则

1. **模块优先，核心 Hook 后置。** 先在独立 Module、EditorPlugin 和外部工具中验证研究假设，证明收益后再修改 Godot 热路径。
2. **引擎提供证据，Agent 负责决策。** 引擎负责不可伪造、类型安全、结构化的运行时证据；Agent 负责提出假设、选择观测目标和生成修复。
3. **根因初筛不依赖 LLM。** 使用确定性图查询和轻量排序生成 Top-k 候选，再交给 Agent 解释和修复。
4. **按需观测，不做全量日志。** 默认只记录轻量事件，在失败或假设收缩后提高特定对象、方法或系统的追踪粒度。
5. **开发语义与运行表示分离。** Agent 使用高语义接口，发布版本可以裁剪追踪和验证数据，不要求 LLM 参与每帧运行。
6. **不把时间先后等同于因果。** 每条因果边必须记录构造证据、规则和置信度。
7. **所有修改必须可审计、可验证、可回滚。** Agent 不直接在主工作区进行不可控试错。

## 3. 分层架构

```text
┌──────────────── Agent Control Plane ────────────────┐
│ Hypothesis / Tool Selection / Patch / Validation    │
│ MCP、CLI 和其他 Agent 协议只作为外部适配器            │
└──────────────────────┬──────────────────────────────┘
                       │ Semantic Debug API
┌──────────────────────▼──────────────────────────────┐
│ Godot Agent Debug Module                            │
│ Trace Session        Failure Window                 │
│ Semantic Snapshot    Runtime Assertions             │
│ Causal Graph         Root Cause Ranker              │
│ Replay Scheduler     Patch Provenance               │
└──────────────────────┬──────────────────────────────┘
                       │ Thin Instrumentation Hooks
┌──────────────────────▼──────────────────────────────┐
│ Godot Runtime                                       │
│ Object/Node · Signal · GDScript · Resource          │
│ Deferred Call · Input · Thread · RPC · Performance  │
└─────────────────────────────────────────────────────┘
```

Godot 内部不绑定某个大模型或 Agent 框架。MCP Server、命令行工具和未来的 IDE 集成都通过同一套 Semantic Debug API 访问能力。

## 4. 建议的源码结构

第一阶段新增独立引擎 Module：

```text
modules/agent_debugger/
├── config.py
├── register_types.cpp
├── register_types.h
├── trace/
│   ├── agent_trace_event.*
│   ├── agent_trace_session.*
│   ├── trace_ring_buffer.*
│   ├── trace_serializer.*
│   └── failure_window.*
├── causal/
│   ├── causal_graph.*
│   ├── causal_edge_builder.*
│   ├── backward_query.*
│   └── root_cause_ranker.*
├── state/
│   ├── semantic_snapshot.*
│   ├── state_diff.*
│   └── runtime_assertion.*
├── replay/
│   ├── replay_manifest.*
│   ├── checkpoint_store.*
│   └── replay_scheduler.*
├── protocol/
│   ├── agent_debug_command.*
│   └── agent_debug_transport.*
└── editor/
    ├── agent_debugger_plugin.*
    └── failure_inspector.*
```

外部控制面建议与引擎模块隔离：

```text
tools/agent_debug/
├── orchestrator/
├── patch_transaction/
├── headless_runner/
├── benchmark/
└── mcp_adapter/
```

## 5. 统一 Trace Schema

所有事件至少共享以下字段：

```text
schema_version
session_id
event_id
parent_event_ids[]
patch_id
event_type
frame / physics_frame
monotonic_timestamp
thread_id

object_id
object_generation
node_path
scene_uid
resource_uid

script
method
source_location

payload
before_hash
after_hash
evidence_level
```

首版事件类型：

- `patch_begin / patch_end`
- `object_create / object_destroy`
- `node_enter_tree / node_exit_tree`
- `method_enter / method_exit`
- `property_write`
- `signal_emit / signal_receive`
- `deferred_schedule / deferred_execute`
- `resource_load_begin / complete / fail`
- `input_event`
- `random_seed`
- `assertion_fail`
- `engine_error`
- `performance_sample`
- `test_begin / test_result`

因果边类型：

- `CALL`
- `EMIT_DELIVER`
- `SCHEDULE_EXECUTE`
- `WRITE_READ`
- `CREATE_USE`
- `LOAD_USE`
- `PATCH_AFFECTS`
- `VIOLATES`
- `HAPPENS_BEFORE`

每条边都记录 `evidence`、`confidence` 和 `construction_rule`。其中 `evidence` 至少区分 `direct`、`inferred` 和 `temporal_only`。

## 6. Godot 接入策略

### 6.1 第一阶段：外围原型

不修改核心热路径，优先复用：

- `EngineDebugger` 的 profiler 和 message capture；
- `SceneTree` 的 Node 增删、重命名和场景变化通知；
- SceneDebugger 的远程场景树和对象检查能力；
- 错误处理器；
- 显式类型安全 Probe；
- Headless 场景与测试运行。

该阶段需要完成 Trace Session、失败窗口、场景快照、状态 Diff、基础因果图和 Top-k 根因查询。

### 6.2 第二阶段：少量核心 Hook

原型证明有效后，进入以下边界：

| 语义 | 接入位置 |
|---|---|
| Signal 发送 | `core/object/object.cpp` 的 `Object::emit_signalp()` |
| 对象创建与销毁 | `ObjectDB::add_instance()` / `remove_instance()` |
| Node 生命周期 | `scene/main/node.cpp` |
| GDScript 调用生命周期 | `modules/gdscript/gdscript_vm.cpp` 的 `GDScriptFunction::call()` |
| Resource 加载 | `core/io/resource_loader.cpp` |
| 帧边界 | `main/main.cpp` 的 EngineDebugger iteration |
| Deferred 调用 | `core/object/message_queue.cpp` |
| RPC 与属性同步 | `modules/multiplayer/` |
| 线程任务 | `core/object/worker_thread_pool.*` |

所有核心 Hook 必须具有极低成本的关闭路径，并受到构建开关和运行时 Trace Level 双重控制。关闭某类事件时不得构造字符串、Variant 或进行堆分配。

## 7. Agent 高层调试接口

首批接口：

```text
start_trace_session(config)
stop_trace_session()

query_failure_window(failure_id)
query_object_history(object_id)
query_invocation(invocation_id)
query_signal_chain(event_id)
query_resource_history(resource_uid)
query_state_diff(snapshot_a, snapshot_b)
query_patch_impact(patch_id)
rank_root_causes(failure_id, top_k)

request_instrumentation(hypothesis, targets, budget)
capture_snapshot(scope)
replay_from_checkpoint(checkpoint_id, overrides)

declare_hypothesis(...)
reject_hypothesis(...)
submit_patch_manifest(...)
run_validation(...)
commit_patch(...)
rollback_patch(...)
```

Agent 应查询“某对象为什么变成当前状态”，而不是逐行读取调试器或遍历每帧全部属性。

## 8. 根因定位

第一版使用确定性反向图查询：

1. 从断言、测试失败、Crash 或性能超预算生成 `FailureNode`；
2. 在失败窗口内沿显式因果边反向遍历；
3. 删除不能影响失败对象或状态的分支；
4. 对候选事件进行可解释排序；
5. 返回一个或多个共同根因候选。

候选排序参考：

- 显式依赖距离；
- Patch 关联度；
- 对象和资源一致性；
- 生命周期边界；
- 异常状态 Diff；
- Runtime Assertion 违反情况；
- 跨线程或异步传播次数；
- 时间邻近度；
- 观测完整性。

时间位置只能作为弱特征，不能直接照搬通用多 Agent Trace 中“越早越可能是根因”的高权重结论。

## 9. 失败窗口与自适应观测

```text
Level 0：完全关闭或编译裁剪
Level 1：错误、帧统计、对象数量
Level 2：生命周期、调用摘要、采样事件
Level 3：指定 Node/Resource/方法的详细追踪
Level 4：失败前后目标范围的完整追踪
```

每个线程使用预分配 Ring Buffer，在帧边界合并。发生断言、脚本错误、测试失败、性能超预算或 Agent 主动标记失败时，冻结失败前后的目标窗口。

自适应观测流程：

```text
轻量 Trace
→ Top-k 候选
→ Agent 提出可证伪假设
→ 对相关对象和事件开启细粒度追踪
→ 有限重放
→ 更新候选置信度
```

## 10. 快照和有限确定性重放

首版限定为：

- 单线程 Gameplay；
- 固定 physics tick；
- 固定随机种子；
- 录制输入序列；
- 同步加载或固定异步完成顺序；
- 无外部网络服务；
- 只恢复白名单 Node 和 Resource 属性。

快照包含 SceneTree 子图、Node 类型和生命周期、白名单属性、Resource UID 与内容 Hash、随机数状态、输入游标、时间步和待执行 Deferred Call 摘要。

GPU、音频设备、操作系统窗口和外部服务首版定义为不可逆副作用。网络和多线程阶段采用因果一致回退，而不是简单回退到上一帧。

## 11. 事务式修复

每次修改生成 `PatchManifest`：

```text
patch_id
intent
hypothesis_ids
evidence_ids
allowed_paths
changed_files
changed_resources
expected_invariants
behavior_tests
performance_budgets
timeout
rollback_point
```

执行流程：

```text
创建隔离工作区
→ 应用 Patch
→ 编译或导入
→ 运行最小场景测试
→ 检查 Runtime Assertion
→ 运行性能预算测试
→ 检查非目标修改
→ Commit 或 Rollback
```

失败 Patch 不保留在主工作区。失败假设、观测证据和验证结果进入结构化历史记录。

## 12. 研究实施阶段

### Phase 0：Schema、协议和 Benchmark

- Trace Schema v0.1；
- Causal Edge 构造规则；
- Semantic Debug Command 协议；
- 首批六类可复现故障；
- Baseline 测量脚本。

首批故障类型：Node 销毁后仍收到回调、Signal 参数或连接错误、场景切换后的旧对象引用、Resource 路径或版本污染、输入/随机数导致的不可复现、Patch 引入帧时间回归。

### Phase 1：外围 MVP

- 独立 Module；
- Trace Session 和 Ring Buffer；
- SceneTree 生命周期；
- 错误和性能采集；
- 场景快照与 Diff；
- Failure Window；
- 基础因果图和 Top-k 查询；
- Headless 测试运行器。

### Phase 2：语义级核心 Hook

- Object 生命周期；
- Signal 发送和接收；
- GDScript Invocation Trace；
- Resource 加载；
- Deferred Call；
- 定向属性写入追踪。

### Phase 3：有限确定性重放

- 输入、随机种子和时间步录制；
- 检查点；
- 支持范围内的重放；
- Top-k 重放预算；
- 重放前后 State Diff。

### Phase 4：Agent 修复闭环

- 假设状态机；
- 定向插桩规划；
- Patch Manifest；
- 隔离验证；
- 自动回滚；
- MCP 和 CLI 适配器。

### Phase 5：网络、多线程和完整研究评估

- RPC、同步属性和 WorkerThreadPool 因果边；
- 多根因定位；
- 因果一致回退；
- Benchmark 扩展；
- 与普通日志 Agent 的对照实验。

## 13. 评价指标

- 根因 Top-1、Top-3、Top-5 命中率；
- 自动修复成功率；
- 回归率和非目标修改数；
- 平均修复轮数；
- Token、工具调用和重放次数；
- 首次失败到定位的时间；
- Trace CPU、内存和输出体积；
- 支持范围内的确定性重放成功率；
- 人工介入次数。

建议将以下性能要求作为工程目标，而不是预先宣称的实验结果：

- Level 0：Release 可以完全裁剪；
- Level 1：CPU 增量目标低于 2%；
- Level 2：CPU 增量目标低于 5%；
- Level 3/4：只能对目标范围短期开启；
- 关闭某事件类型时，其热路径不得发生堆分配。

## 14. 第一版范围

第一版只处理：

> GDScript + 单机 + 单线程 Gameplay + Node/Signal/Resource + 行为和性能测试。

暂时不同时处理 C#、原生 C++ Gameplay、GPU 状态、完整物理恢复、网络和多线程重放。该收缩可以保证研究首先回答“引擎语义追踪和因果查询是否能显著降低 Agent 调试成本”，而不是过早陷入维护大型引擎 Fork。

## 15. 近期交付顺序

1. 完成架构 RFC 和 Trace Schema v0.1；
2. 建立 `modules/agent_debugger` 可编译骨架；
3. 建立六类最小故障 Benchmark；
4. 完成外围 Trace、失败窗口和 Top-k 查询；
5. 对 Baseline 与 Proposed 进行第一轮测量；
6. 只有在指标证明有效后，才进入 Signal、ObjectDB 和 GDScript VM 等核心热路径。

## 16. 社区插件调研

社区源码调研已经完成，独立文档见：

- [Godot 社区 AI 与 Agent 插件调研](godot_ai_plugins/Godot社区AI与Agent插件调研.md)
- [社区源码复用与集成计划](godot_ai_plugins/社区源码复用与集成计划.md)

本次下载并静态分析了 11 个代表性项目，包括 GitHub 热度最高的 MCP、Godot Asset Library 插件、Runtime Bridge、原生 Debugger Bridge、编辑器内 Agent、权限系统和 Undo 实现。

调研后的核心结论是：社区已经较好覆盖 Agent Control Plane 和 Editor Command Layer，但没有形成 Patch、运行时状态和失败之间的因果链。现有项目普遍能够：

- 读取和修改 Scene/Node/Resource；
- 运行项目并读取错误；
- 截图和模拟输入；
- 提供传统 Stack/Variable Debugger；
- 使用 UndoRedo 或文件备份进行有限回退。

尚未形成成熟实现：

- 统一 Trace Schema；
- Patch Attribution；
- Object/Resource 生命周期历史；
- Invocation Lifetime Trace；
- Signal/Deferred/RPC 因果边；
- Failure-window Trace；
- Semantic State Diff；
- Top-k Root Cause Ranking；
- Targeted Instrumentation；
- Deterministic Replay；
- 行为和性能联合事务。

因此原方案应避免重新建设通用 MCP CRUD，优先兼容或复用社区控制面，把主要工程投入放在 Runtime Evidence、Causal Analysis 和 Transactional Verification。

本项目采用“能够复用就直接复用”的策略。社区源码以固定 Commit 和 MIT License 进入 `thirdparty/agent_tools/`，通过 Adapter 与本项目核心隔离；`sources/` 只保留原始研究快照。复用代码必须记录上游仓库、Commit、许可证、导入路径和本地修改，避免后续无法区分上游实现与原创研究贡献。

重点参考项目：

- `hi-godot/godot-ai`：Session、Typed Envelope、Atomic Batch、Log Cursor、测试体系；
- `yurineko73/Godot-MCP-Native`：EditorDebuggerPlugin Bridge、Runtime Probe、鉴权和 Render Regression；
- `Erodenn/godot-mcp-runtime`：临时 Runtime Bridge 和隔离运行；
- `FlamxGames/godot-ai-assistant-hub`：Allow/Ask/Hide 和按 Agent 权限 Profile；
- `tomyud1/godot-mcp`：Project/Scene Graph Visualizer；
- `sancheznot/Godot-AI-Assistant`：Minimal Bootstrap、Harness Mode 和 Anti-hallucination Guard。

后续比较和集成仍重点关注：

- 编辑器控制协议；
- Scene/Node/Resource 的结构化操作；
- 代码和资产生成；
- 运行、截图、日志及测试闭环；
- 事务、权限和回滚；
- Runtime Trace、状态 Diff 与因果定位；
- 可复用组件及与本方案之间的能力缺口。
