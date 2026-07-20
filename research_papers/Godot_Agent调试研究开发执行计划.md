# Godot Agent 调试研究开发执行计划

## 一、结论

应当拉取实例工程并进行 Agent 驱动操作，但不能从“让 Agent 随机开发一个游戏”开始。正确顺序是：

```text
人工构造且根因已知的微型故障
→ 官方小型 Demo 上的受控任务
→ 中型真实项目上的自然缺陷
→ 大型工程与性能、并发、资源问题
```

每个实验必须能复现、能自动判定成功、能回滚，并保存完整 Episode。否则只能得到演示，不能得到可比较的研究结果。

## 二、当前能力边界

当前 `modules/agent_debugger` 已具备：

- Trace session 和结构化事件。
- 固定容量环形缓冲区与失败窗口。
- 显式因果边。
- 从失败事件反向查询 Top-K 根因候选。
- `AgentTrace` GDScript 单例。

尚未具备：

- 对 Godot 生命周期、信号、资源、deferred call 的自动埋点。
- MCP/CLI Agent 控制适配器。
- 确定性输入录制与 Replay Capsule。
- 隔离 worktree、事务补丁、自动提交与回滚。
- 面向实例工程的统一任务运行器和评价器。

因此，第一批项目实验允许使用少量显式 `AgentTrace.record_event()` 埋点；随后再逐步把高价值事件下沉为引擎自动观测。

## 三、实例工程组合

### A. 自建微型基准工程

创建一个很小的 `agent_debug_benchmarks` 工程，每类故障一个独立场景：

1. 节点在 `_ready` 前被读取。
2. 信号重复连接或接收者错误。
3. 资源重载后使用陈旧引用。
4. 场景切换后 deferred 操作访问已释放对象。
5. 随机种子或输入缺失导致偶发失败。
6. Agent 补丁引入逐帧分配或高频遍历。

这些样例拥有人工记录的真实根因，用于验证 Trace Schema、因果边和排名算法。

### B. Godot 官方 Demo

固定 `godotengine/godot-demo-projects` 的一个 commit，并从以下类型各选一个小项目：

- 2D：输入、角色移动、碰撞和节点生命周期。
- GUI：信号连接、状态同步和控件树。
- Loading：资源加载、失败路径和异步状态。
- Navigation 或 3D：跨帧状态、物理帧和性能预算。

不要一次导入全部 Demo 作为实验集。先选 3 至 4 个，给每个项目建立基线启动命令、固定输入和验收断言。

### C. 中型社区项目

当官方 Demo 的自动修复成功率稳定后，再选一个持续维护、Godot 4、MIT 许可的中型项目。中型项目主要用于检验：

- Agent 在陌生代码库中的定位能力。
- 因果候选是否仍能降低搜索空间。
- Trace 开销和上下文规模是否可控。
- 补丁是否产生跨系统回归。

大型 TPS、联网游戏和复杂插件不应作为第一阶段样例。

## 四、标准 Agent Episode

每次实验都执行同一状态机：

```text
Prepare
→ Baseline
→ Inject Task/Bug
→ Reproduce
→ Observe
→ Hypothesize
→ Patch in Isolated Worktree
→ Validate
→ Commit or Rollback
→ Archive Episode
```

### Prepare

- 固定引擎 commit、项目 commit、模块配置和 Agent 模型版本。
- 创建隔离分支或 worktree。
- 清除 `.godot` 缓存差异，记录必要的导入环境。

### Baseline

- 运行项目启动检查和已有测试。
- 保存基线输出、性能值和关键状态哈希。
- 基线失败的项目不得进入 Agent 实验。

### Inject Task/Bug

- 每个故障保存独立 patch。
- `task.json` 只描述用户可见症状，不向 Agent泄露真实根因。
- `oracle.json` 保存真实注入文件、行、事件和期望行为，只供评价器使用。

### Reproduce and Observe

- 使用固定场景、输入序列、随机种子和最大帧数。
- 失败时保存 Failure Window。
- Agent 先获得低成本信息，再按假设请求更高粒度观测。

### Patch and Validate

- Agent 只能修改任务允许的文件范围。
- 先做解析和脚本加载检查，再运行目标场景、断言和性能门禁。
- 失败补丁自动回滚；成功补丁也必须保留 diff、理由和验证证据。

## 五、实验组与消融对比

同一批故障至少运行三组：

| 组别 | Agent 可用证据 | 研究目的 |
|---|---|---|
| L | 错误文本和普通日志 | 基线 |
| T | L + 结构化 Trace/Failure Window | 测量语义事件价值 |
| C | T + 因果图/Top-K 根因 | 测量因果查询价值 |

条件允许时再加入 R 组：C + Replay Capsule，用于测量确定性重放的增益。

## 六、核心指标

- `reproduction_rate`：固定条件下的复现比例。
- `task_success_rate`：最终行为验收通过比例。
- `root_cause_recall@1/3/5`：真实根因是否进入 Top-K。
- `attempt_count`：成功前生成的补丁数量。
- `time_to_first_useful_cause_ms`：获得首个有效候选的时间。
- `human_intervention_count`：人工介入次数。
- `changed_lines` 与 `scope_violation_count`：补丁大小与越界修改。
- `trace_overhead_cpu_pct`、`trace_bytes_per_second`：观测成本。
- `regression_rate` 与 `rollback_success_rate`：安全性。

## 七、近期开发迭代

### 迭代 1：可运行研究流水线

- 建立微型基准工程和 `task.json`/`oracle.json`。
- 实现 CLI Runner：启动项目、设置超时、收集退出码与 Trace。
- 建立 Episode 目录格式。
- 先由人工或简单脚本调用 `AgentTrace`，跑通一条完整故障。

### 迭代 2：Agent 控制面

- 从已下载的 MIT 社区源码中按需移植 MCP Session、游标和 Debugger Bridge。
- 工具分为只读、运行、项目写入和外部命令四个权限级别。
- 写操作使用隔离 worktree，并记录 patch_id。

### 迭代 3：自动语义观测

- 优先接入错误/断言、对象生命周期、信号 emit/deliver、deferred schedule/execute。
- 为每种边编写确定性构造规则测试。
- 对热点路径做采样和预算限制。

### 迭代 4：事务验证与重放

- 固定输入、随机种子、场景和运行帧数。
- 实现补丁验证门禁和自动回滚。
- 形成 L/T/C/R 消融实验数据。

### 迭代 5：扩展到官方与中型工程

- 在 3 至 4 个官方 Demo 上执行受控功能任务与故障修复。
- 达到预设成功率和开销阈值后，再引入一个中型项目。
- 从失败 Episode 中反推新的事件类型和观测 Hook，而不是凭感觉扩展引擎。

## 八、进入下一阶段的门槛

微型基准进入官方 Demo 前，建议至少满足：

- 六类故障均可自动复现。
- `root_cause_recall@3 >= 80%`。
- 自动回滚成功率为 100%。
- 默认追踪 CPU 开销低于预先设定的研究预算。
- 每个 Episode 能由命令行独立重跑并生成相同判定。

官方 Demo 进入中型项目前，建议至少满足：

- 受控任务总数不少于 20。
- C 组相对 L 组在成功率或修复尝试次数上有稳定提升。
- 没有依赖人工查看编辑器 UI 才能判定的关键步骤。
