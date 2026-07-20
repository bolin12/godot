# Agent 调试基准集 v0.1

## 目标

以固定工程、固定输入、固定失败条件评估 Agent 调试闭环，不用“能否生成代码”代替调试能力。每个样例都必须输出事件窗口、因果候选、补丁关联和验证结果。

## 首批六类样例

| ID | 场景 | 注入缺陷 | 必须采集的关键事件 | 成功条件 |
|---|---|---|---|---|
| B01 | 节点生命周期 | 在 `_ready` 前读取尚未初始化的子节点 | node_create、tree_enter、method_enter、property_read、error | 根因候选包含过早读取，且不是只返回最后一条报错 |
| B02 | 信号链 | 重复连接或连接到错误接收者 | signal_connect、signal_emit、signal_deliver、method_enter | 能还原 emit→deliver，并定位错误连接操作 |
| B03 | 资源状态 | 重载后继续使用陈旧资源引用 | resource_load、resource_reload、object_use、error | 能区分当前资源与 stale object，并给出 create/load→use 边 |
| B04 | 场景切换 | deferred 操作在场景卸载后执行 | scene_change、node_exit、schedule、execute、object_use | 能还原 schedule→execute 和对象生命周期冲突 |
| B05 | 确定性复现 | 输入或随机种子缺失导致偶发失败 | input、random_seed、physics_step、assertion_fail | 同一 capsule 重放至少三次得到相同失败帧与状态哈希 |
| B06 | 性能回归 | Agent 补丁引入每帧分配或高频遍历 | patch_begin/end、frame_metric、allocation、method_sample | 自动关联 patch_id，并判定预算超限而回滚或拒绝提交 |

## 统一评价指标

- `reproduction_rate`：相同 capsule 的稳定复现比例。
- `root_cause_recall@k`：真实注入点是否出现在前 k 个候选。
- `causal_precision`：高置信边中能由直接观测验证的比例。
- `time_to_first_useful_cause_ms`：从失败发生到获得第一个有效候选的时间。
- `trace_overhead_cpu_pct` 与 `trace_bytes_per_second`：追踪成本。
- `patch_validation_pass_rate`：补丁通过语法、运行、断言和性能门禁的比例。

## 数据纪律

基准夹具与期望答案后续放入 `modules/agent_debugger/tests/fixtures`。缺少直接证据时只能标记 `inferred` 或 `temporal_only`，不得把时间邻近描述成确定因果。
