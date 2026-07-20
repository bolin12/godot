# 自然语言驱动 Agent 开发与 Bug 收割流程

## 1. 核心思路

没有现成 Bug 不会阻断研究。自然语言驱动开发会自然产生解析错误、错误 API、节点路径失效、信号错误、状态机遗漏、资源问题和行为回归。第一次出现时它们是“未知自然缺陷”；完成修复和复盘后，可以转化成拥有标准答案的可重复 Bug。

研究同时维护两条轨道：

```text
Discovery Track
自然语言需求 → Agent 实现 → 自动门禁发现未知 Bug → 调试并修复

Benchmark Track
冻结失败版本 → 最小化复现 → 固化根因与修复 → 以后重复评测
```

Discovery Track 评价真实开发能力；Benchmark Track 评价不同 Agent、调试接口和因果算法的相对能力。

## 2. 自然语言任务的写法

需求应描述可观察行为，不指定代码文件和实现方式。例如：

```text
在暂停菜单增加“重新开始当前关卡”按钮。
点击后需要关闭暂停状态、清理当前关卡临时对象、重新加载当前场景。
玩家生命值和关卡内收集物应恢复到进入关卡时的状态。
连续操作三次不能出现重复信号、悬空节点或明显内存增长。
```

一个有效任务至少包含：

- 用户可观察的目标。
- 操作步骤或输入序列。
- 必须保持不变的行为。
- 明确的失败条件。
- 性能或资源预算（适用时）。

纯粹的“做得更好”“优化一下”不能作为自动研究任务，除非补充评价 Rubric。

## 3. Episode 记录

每次自然语言任务建立唯一 `episode_id`，记录：

```text
episodes/<episode_id>/
  request.md                原始自然语言需求
  environment.json          引擎、项目、Agent、平台和种子
  baseline.json             修改前启动、测试和性能结果
  timeline.jsonl            Agent 假设、工具调用和状态转换
  patches/
    attempt_001.diff
    attempt_002.diff
  traces/
    session_001.jsonl
    failure_window_001.json
    causes_001.json
  validation/
    syntax.json
    runtime.json
    behavior.json
    performance.json
  outcome.json              成功、失败、回滚及人工介入
  postmortem.md             修复后根因复盘
```

自然语言对话本身不是充分证据。必须同时记录实际 diff、运行命令、退出码、Trace event、验证输出和最终状态。

## 4. 自动开启 Incident

以下任一门禁失败时，从当前 Episode 派生一个 `incident_id`：

- 工程或脚本无法解析。
- 项目不能启动或出现引擎错误。
- 固定输入无法达到需求行为。
- 原有行为回归。
- 状态哈希、节点断言或截图检查失败。
- 帧时间、内存或对象数量超过预算。
- Agent 修改了允许范围之外的文件。

Incident 创建时立即冻结：

- 最后一个通过验证的 commit。
- 第一个失败 patch。
- 复现命令和输入序列。
- 失败前后 Trace 窗口。
- 当时的 Agent 假设和工具输出。

## 5. 从未知 Bug 生成标准基准

修复成功后执行 Bug Harvest：

1. 验证失败版本在固定环境中至少重复失败三次。
2. 验证修复版本至少重复通过三次。
3. 缩小失败 patch，排除与缺陷无关的修改。
4. 标注首次异常事件、根因事件、关键因果边和修复位置。
5. 生成不泄露根因的 `task.md`。
6. 生成只供评价器使用的 `oracle.json`。
7. 保存 `broken.patch`、`fix.patch`、输入和自动断言。
8. 分别运行只看日志、Trace、Trace+因果图三个实验组。

由此得到：

```text
benchmarks/<bug_id>/
  base_commit.txt
  broken.patch
  fix.patch
  task.md
  reproduce.json
  assertions.json
  oracle.json
```

## 6. 第一批自然语言任务建议

### 官方 Dodge the Creeps

- 增加暂停、继续和重新开始流程。
- 增加连续受击后的短暂无敌时间。
- 增加最高分保存与异常存档恢复。

### Godot Game Template

- 增加按场景保存的图形/音频设置。
- 增加异步场景加载进度与取消操作。
- 增加手柄重映射并保持键鼠配置不变。

### Flick

- 增加检查点和死亡后复位。
- 增加新的可收集物及关卡统计。
- 增加移动平台，并保证 Flick 动作与平台速度叠加正确。

### Roguelike Example

- 增加一种具有冷却时间的敌人行为。
- 增加库存堆叠、丢弃和拾回。
- 增加保存/恢复地牢种子与玩家状态。

这些任务很容易自然触发生命周期、信号、状态恢复、资源和跨帧 Bug，适合作为 `AgentTrace` 的第一批真实输入。

## 7. 评价原则

- 未知自然 Bug 在修复完成前不计算 root-cause oracle 指标。
- 修复后由可执行证据和最小 diff 确认根因，不能只采信 Agent 的文字解释。
- 无法稳定复现的 Episode 可以保留为案例，但不能进入确定性 Benchmark。
- 为避免只统计成功案例，所有失败、超时和人工终止 Episode 都必须保存。
