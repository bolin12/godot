# 增量失效规划与 Runtime 纵向切片实施记录

日期：2026-07-20

## 1. 本阶段结论

Gate A–C 已具备可重复的错误注入和确定性比较基线，并完成了第一个真实 Runtime Contract 纵向切片。

```text
Artifact Graph
→ Invalidated Artifact Plan
→ Semantic Fault Matrix
→ Real Scene Instantiation
→ Method Call / State Snapshot
→ Signal Emit / Observed Delivery
→ Runtime Assertions / Causal Edges
```

## 2. 增量失效规划

新增两个 `AgentArtifactGraph` API：

- `plan_invalidated_artifacts(changed_paths)`：返回变更 Artifact 本身及其反向依赖闭包；
- `get_graph_fingerprint()`：对 Artifact、依赖边和验证结果生成确定性 SHA-256，排除 receipt ID 和 timestamp。

以 `res://player.gd` 为变更输入，结果为三个 Artifact：

```text
player.gd
→ player.tscn
→ main.tscn
```

机器结果：

```json
{
  "invalidated_artifact_count": 3,
  "plan_matches_full_graph": true,
  "full_scan_fingerprint_stable": true,
  "validation_passed": true,
  "result": "pass"
}
```

当前结论严格限定为“增量失效规划正确”。系统尚未实现持久缓存和局部图记录重算，因此不能声称完整的 incremental recomputation 已完成。

## 3. Semantic Fault Matrix

新增五种不修改 Demo 文件的契约故障注入：

1. 缺失 Node；
2. 错误 Node 类型；
3. 缺失 Group；
4. 缺失 Signal connection；
5. 缺失 InputMap action。

每个 case 必须同时满足：

- Contract 总结果为 fail；
- `failed_assertions` 包含该 case 指定的精确 assertion。

结果：

```json
{
  "case_count": 5,
  "failed_case_count": 0,
  "result": "pass"
}
```

这里的 `failed_case_count = 0` 表示五个故障均被正确捕获，而不是故障输入意外通过。

## 4. Runtime Contract 纵向切片

Runtime Harness 在独立 headless 进程中：

1. 使用 ResourceLoader 加载真实 `main.tscn`；
2. 实例化并加入真实 SceneTree；
3. 等待 `_ready`；
4. 验证 Player 初始隐藏、Gameplay Timer 初始停止；
5. 调用真实 `Main.new_game()`；
6. 验证 score、Player 可见性、StartTimer 和碰撞状态；
7. 发出真实 `Player.hit` Signal；
8. 验证 `Main.game_over()` 产生的 Timer 和 HUD 状态；
9. 输出事件、因果边、Assertion，以及失败时的 Failure Window 和 Cause Candidates。

结果：

```json
{
  "assertion_count": 9,
  "failed_assertion_count": 0,
  "event_count": 15,
  "edge_count": 13,
  "result": "pass"
}
```

连续执行三次结果一致，退出时无 ObjectDB 或 ResourceCache 泄漏报告。

## 5. 证据等级

Runtime Harness 直接记录的资源加载、实例化、方法调用和状态 Snapshot 属于 `observed/tested` 证据。

当前 Signal delivery 并非由引擎 Signal 热路径自动 Hook。Harness 执行真实 `emit_signal()`，结合已保存 connection 和后续状态观测建立 `emit_deliver` 边，因此明确记录为：

```text
evidence = inferred
confidence = 0.9
construction_rule = persisted_connection_and_observed_state
```

只有未来引擎级 Hook 捕获 emit 入口与接收方法入口后，才能把该边升级为 `direct`。

## 6. 首次运行发现的假绿问题

首次 Runtime Harness 执行出现 GDScript typed-array 调用错误，但脚本仍继续到退出路径，产生了零断言 pass。

修复包括：

- 调整 parent event array 的类型边界；
- 添加 `expected_assertion_count = 9`；
- 只有断言数量完整且失败数为零时才允许 pass；
- SceneTree 和资源完全清理后再退出。

这证明 Runtime Gate 不能只检查进程退出码，还必须检查执行完整性、预期断言数量和结构化结果。

## 7. 构建与测试

模块测试结果：

```text
test cases: 8 passed, 0 failed
assertions: 50 passed, 0 failed
```

新增测试覆盖：

- dependency fixture 的反向闭包；
- 按路径生成失效计划；
- 两次全量扫描指纹一致；
- Runtime causal edge 的公开查询。

## 8. 下一阶段

1. 实现 Runtime assertion 的负路径，验证 Failure Window 与 backward cause ranking；
2. 为 Node 生命周期和 Signal delivery 增加可选的引擎级自动 Hook；
3. 把 Artifact ID、Patch ID、Runtime Event ID 和 Validation Receipt ID 对齐；
4. 实现持久 Receipt cache 与真正的局部重算；
5. 在此后进入 Render Manifest，而不是直接跳到截图比较。
