# Godot Agent 原生验证流水线实施步骤 v0.2

## 1. 目标

把 Godot 项目的开发与调试过程拆成一组可查询、可失效、可重复执行的验证阶段。系统必须能回答：

1. 当前结论由哪些输入和验证器产生；
2. 某个文件变化后，哪些结论必须重新验证；
3. 第一个失败阶段在哪里；
4. 哪些结论是严格证明、测试结果、运行观测或推断。

本方案不承诺脱离契约范围的“绝对正确”。正确性定义为：

> 在已声明契约及验证器覆盖范围内结果成立，并且增量验证结果与完整重算一致。

## 2. 精简后的阶段

### Gate A：Native Parse

直接调用 Godot 原生解析和资源加载链，不实现第二套语法分析器。

执行该 Gate 前必须先完成 `godot --headless --import`。导入产物缺失属于 Import 前置条件失败，不能归类为源码语法错误。

覆盖：

- GDScript 解析与编译；
- `.tscn`、`.tres` 等资源格式解析；
- 资源是否存在且能被 `ResourceLoader` 加载；
- 被扫描依赖闭包内的资源。

不覆盖：

- 未进入扫描根依赖闭包的脚本；
- 运行时动态拼接路径；
- NodePath 和 Signal 接收方法的业务有效性；
- 只有真实渲染后端才能完成的 Shader/GPU 验证。

结论：语法检查是必要能力，但不需要自行实现；它只是原生工具链的一个验证回执。

### Gate B：Artifact Resolve

建立 Artifact Dependency Graph，并验证：

- Resource UID 与 fallback path；
- PackedScene、Script、Texture、Audio 等资源依赖；
- 主场景和被实例化场景；
- 依赖闭包是否完整；
- 内容 Hash 和验证器版本是否仍然匹配已有回执。

### Gate C：Semantic Contract

验证合法数据是否满足项目意图：

- 必需 Node、类型和分组；
- NodePath 指向；
- Signal 来源、接收者与方法；
- InputMap action；
- 项目声明的不变量和属性范围。

### Gate D：Runtime State

验证实例化之后的运行语义：

- Object/Node 生命周期；
- 方法调用和状态 Diff；
- Signal emit/deliver；
- Deferred schedule/execute；
- 输入、随机种子、physics frame；
- 行为断言和性能预算。

### Gate E：Render Submission

记录进入渲染系统之前的高层 Manifest：

- Camera、Viewport 和可见对象；
- Transform、Z index、layer；
- Texture、Material、Shader UID；
- Shader 参数 Hash；
- Render target 和 frame ID。

### Gate F：Image

在真实渲染后端运行并验证：

- Shader 编译和驱动错误；
- GPU/后端差异；
- 截图或视频；
- 结构化视觉断言与感知差异。

## 3. 公共证据等级

所有 Gate 共享以下等级：

| 等级 | 含义 |
|---|---|
| `proven` | 由确定性规则在明确前提下推出 |
| `tested` | 由可重复自动测试得到 |
| `observed` | 由一次运行观测得到 |
| `inferred` | 由不完整证据推断 |
| `unverified` | 尚未验证 |

低等级证据不得被 Agent 自动描述为高等级保证。

## 4. 数据结构

### ArtifactNode

```text
artifact_id
path
uid
resource_type
content_hash
exists
```

优先使用 Resource UID 作为稳定 ID；无 UID 时退化为规范化路径。

### DependencyEdge

```text
from_artifact_id
to_artifact_id
edge_type
evidence
construction_rule
```

第一版只实现 `uses`，方向为“消费者 → 依赖”。受影响范围查询沿反向边传播。

### ValidationReceipt

```text
receipt_id
gate
subject_id
validator_id
validator_hash
input_hash
result
proof_level
error_code
timestamp_usec
assertions[]
failed_assertions[]
```

验证器版本或任意传递输入 Hash 变化时，对应回执失效。

## 5. 实施顺序

### Step 0：建立可测试基线

1. 使用 `tests=yes module_agent_debugger_enabled=yes` 构建编辑器；
2. 运行所有 `[AgentDebugger]` 测试；
3. 固定 Godot commit、构建参数和测试输出；
4. 基线不通过时停止后续功能开发。

退出条件：模块测试可由命令行独立执行。

### Step 0.5：完成项目导入

1. 对目标项目运行 `godot --headless --path <project> --import`；
2. 等待所有源资产产生导入产物；
3. 导入失败单独返回 `import` 阶段错误；
4. 只有 Import 成功后才执行 Native Parse 和依赖闭包加载。

退出条件：重复执行 Import 不产生新的导入失败，且依赖资源能够加载。

### Step 1：Artifact Graph MVP

1. 接收一个或多个资源根；
2. 通过 `ResourceLoader::get_dependencies()` 递归发现依赖；
3. 使用 Resource UID 生成稳定 Artifact ID；
4. 记录资源类型、存在状态和 SHA-256；
5. 去重节点和边；
6. 支持反向查询受影响资源。

退出条件：`main.tscn` 的传递资源依赖可输出为稳定 JSON 数据。

### Step 2：Native Parse Receipt

1. 对依赖闭包中的每个资源调用原生加载器；
2. 分别记录 `resource_exists` 与 `resource_loads` assertion；
3. 保存引擎版本和输入 Hash；
4. 缺失或加载失败返回失败回执；
5. 不用自然语言日志代替结构化结果。

退出条件：删除资源或制造 GDScript/Scene 解析错误时，CLI 非零退出。

### Step 3：From-scratch Consistency

1. 保存完整扫描输出；
2. 修改单一 Artifact；
3. 只重算反向依赖闭包；
4. 同时执行一次完整重算；
5. 比较节点、边、回执和最终判定。

退出条件：增量结果与完整重算一致率为 100%。

### Step 4：Dodge the Creeps Contract

首批契约：

- `application/run/main_scene` 可解析；
- `Main.mob_scene` 是 `PackedScene`；
- `Player/CollisionShape2D` 存在且类型正确；
- `Player.hit → Main.game_over`；
- `HUD.start_game → Main.new_game`；
- `start_game` InputMap action 存在；
- Mob 属于 `mobs` 分组。

退出条件：四类人工缺陷均在预期 Gate 首次失败。

### Step 5：Runtime Trace 对接

1. Artifact ID 与现有 `AgentTrace.resource_uid` 对齐；
2. Validation Receipt 可以引用 Runtime Event ID；
3. 增加调用、Signal、Deferred 和生命周期 correlation ID；
4. Failure Window 返回关联 Artifact 子图；
5. Top-K 查询优先使用直接因果边。

退出条件：运行时失败可回溯到 Artifact、Patch 和验证回执。

### Step 6：Render Boundary

1. 先实现高层 Render Manifest，不 Hook GPU 命令；
2. 固定输入、随机种子和采样帧；
3. 对比 Runtime Snapshot 与 Render Manifest；
4. 只有 Manifest 一致但图像不一致时，才把范围收缩到 GPU/Shader/合成；
5. 最后再评估是否需要底层 RenderingServer Hook。

退出条件：至少能区分 Runtime State、Render Submission 和 Final Image 三类故障。

## 6. 当前实现状态

已完成：

- `ArtifactNode`、`DependencyEdge`、`ValidationReceipt` v0.2；
- ResourceLoader 递归依赖扫描；
- UID 优先的 Artifact ID；
- SHA-256 输入 Hash；
- Native Parse 回执；
- Import 前置阶段已经在真实 Demo 上确认必要；
- 反向受影响范围查询；
- `AgentArtifactGraph` 引擎单例；
- 基础单元测试；
- CLI 扫描脚本。

待完成：

- Artifact Resolve 的项目级契约；
- 增量缓存和 from-scratch consistency 测试；
- Dodge the Creeps Contract；
- Runtime Trace 对接；
- Render Manifest。

## 7. 非目标

第一版明确不做：

- 自制 GDScript Parser；
- 全量逐属性或逐语句 Trace；
- 任意 Live GDScript 执行；
- 网络和多线程确定性重放；
- GPU 指令级因果图；
- 仅凭截图声明行为正确。
