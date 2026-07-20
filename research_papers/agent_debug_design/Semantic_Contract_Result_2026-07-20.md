# Semantic Contract 阶段实施记录

日期：2026-07-20

## 1. 本阶段目标

在不实例化游戏、不推进帧、不依赖真实 GPU 输出的条件下，验证 Godot 已保存工程数据是否满足显式项目契约。

本阶段位于验证流水线的 Gate C：

```text
Import
→ Native Parse
→ Artifact Resolve
→ Semantic Contract（本阶段）
→ Runtime State
→ Render Submission
→ Final Image
```

## 2. 编译器与本阶段的职责边界

Godot 原生解析器、GDScript 编译器和 ResourceLoader 负责排除：

- 被加载依赖闭包中的 GDScript 语法与静态分析错误；
- `.tscn`、`.tres` 等资源格式错误；
- 缺失资源和无法完成的资源加载；
- 本模块 C++ 的类型、声明、链接等构建错误。

因此不需要实现第二套 GDScript parser。语法 Gate 是必要步骤，但不是完整正确性的证明。

Semantic Contract 在此基础上继续排除：

- 必需场景或节点缺失；
- 节点类型不满足声明；
- 节点未加入必需 group；
- 已保存的 Signal connection 缺失；
- 必需 InputMap action 缺失。

它仍不能排除：

- 动态生成的 NodePath、Callable 和反射调用错误；
- Signal 接收方法在具体运行状态中的行为错误；
- 生命周期、时序、物理、随机数和异步错误；
- Render Submission 与最终图像错误；
- Shader 在特定驱动和 GPU 上的编译或执行错误。

## 3. 新增数据与 API

新增 `AgentContractValidator` 引擎单例，输入结构化 Dictionary 契约，输出 `schema_version = 0.2` 的报告与 Validation Receipt。

当前契约字段：

```text
contract_id
scenes[]
  path
  required_nodes[]
    path
    type
    groups[]
  required_connections[]
    source
    signal
    target
    method
required_input_actions[]
```

每张回执记录：

- subject、validator ID 与 validator hash；
- Gate、result、proof level 与 timestamp；
- 全部 assertions；
- failed assertions；
- 失败详情和原生 error code。

场景检查直接读取 `PackedScene/SceneState`，不会实例化游戏场景。节点查找使用 SceneState 的只读线性接口，避免依赖仅在构建态存在的内部 node cache。

## 4. Dodge the Creeps 契约

项目级脚本验证四个场景：

1. `main.tscn`：Main、Player、三个 Timer、HUD，以及五条主流程 Signal connection；
2. `player.tscn`：Area2D 根、动画、碰撞、Trail，以及 `body_entered` connection；
3. `mob.tscn`：RigidBody2D 根、`mobs` group、动画、碰撞、可见性通知节点与 connection；
4. `hud.tscn`：CanvasLayer 根、分数、消息、按钮、Timer 与两条 connection；
5. `project.godot`：移动和开始游戏所需的五个 InputMap action。

执行命令：

```powershell
bin\godot.windows.editor.x86_64.agent_contract.console.exe `
  --headless `
  --path agent_research_projects\upstream\godot-demo-projects\2d\dodge_the_creeps `
  --script D:\godot\agent_research_projects\tools\validate_dodge_contract.gd
```

结果：

```json
{
  "contract_id": "dodge_the_creeps/0.1",
  "failed_receipt_count": 0,
  "receipt_count": 5,
  "result": "pass",
  "schema_version": "0.2"
}
```

## 5. 构建与模块测试

为避免覆盖正在运行的 Godot 编辑器，验证版本使用独立后缀：

```powershell
py -m SCons `
  platform=windows `
  target=editor `
  tests=yes `
  module_agent_debugger_enabled=yes `
  extra_suffix=agent_contract
```

测试结果：

```text
test cases: 7 passed, 0 failed
assertions: 39 passed, 0 failed
```

测试覆盖有效契约和缺失节点的负路径。首次执行还暴露了 SceneState node cache 前置条件；改用公开只读遍历后，模块测试与真实工程检查均通过。

## 6. 当前 OpenGL 后端结论

该 Demo 在 `project.godot` 中显式设置：

```ini
renderer/rendering_method="gl_compatibility"
renderer/rendering_method.mobile="gl_compatibility"
```

所以当前日志显示 OpenGL Compatibility 是项目配置结果，不是 RTX 4060 不支持 Vulkan 或 D3D12。当前 Godot Windows 构建同时列出了 `vulkan`、`d3d12`、`opengl3` 和 `dummy` display driver。

这个纯 2D 官方 Demo 使用 Compatibility 后端是合理基线：启动成本低、覆盖范围广，也没有依赖 Forward+ 特性的必要。对于后续 Agent Render Gate，建议建立后端矩阵：

```text
Primary on Windows: D3D12
Cross-platform reference: Vulkan
Compatibility comparison: OpenGL
Headless semantic gates: dummy/headless
```

本阶段没有修改项目渲染器。切换后端应作为 Render Gate 的独立实验变量，并分别记录 driver、GPU、Godot commit、shader compile result 和图像回执。

## 7. 当前可以声明的保证

在本次项目内容、Godot 构建和 `dodge_the_creeps/0.1` 契约下，可以声明：

- 依赖闭包资源可由原生加载链解析；
- 契约声明的节点、类型、group、已保存连接和 InputMap action 存在；
- 相同输入可通过命令行重复获得结构化回执；
- 这些结论的证据等级为 `tested`，不是对运行时或最终图像的 `proven`。

## 8. 后续步骤

1. 为四类契约缺陷做 fault injection，确认第一次失败 Gate 与错误详情准确；
2. 实现增量验证与 from-scratch 全量验证的一致性对照；
3. 将 Semantic Receipt 与现有 Runtime Trace 的 event/correlation ID 对接；
4. 定义 Runtime State contract，覆盖启动、移动、碰撞和 game-over；
5. 定义高层 Render Manifest；
6. 最后在 D3D12、Vulkan、OpenGL 后端分别运行真实渲染与图像 Gate。
