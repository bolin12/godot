# Agent 验证流水线首个纵向切片实施记录

日期：2026-07-20

## 1. 实施范围

本轮只实现验证流水线的最小基础，不修改 Godot 核心热路径：

```text
Resource Roots
→ ResourceLoader Dependency Scan
→ Artifact Graph
→ Native Parse Validation Receipt
→ JSON Report / Exit Code
```

未实现 Semantic Contract、Runtime State、Render Manifest 和 Image Gate。

## 2. 语法验证决策

没有实现自制 GDScript Parser。GDScript、Scene 和 Resource 格式检查直接委托给 Godot 原生 `ResourceLoader` 和对应格式加载器。

编译器/解析器可以排除其覆盖闭包内的语法和格式错误，但不能排除：

- 未被扫描或加载的脚本；
- 动态路径和反射调用；
- NodePath、Signal 和项目意图错误；
- Runtime 状态错误；
- Shader/GPU 最终执行错误。

## 3. 新增实现

- `AgentArtifactNode`：稳定 ID、路径、UID、类型、内容 Hash 和存在状态；
- `AgentArtifactEdge`：消费者到依赖的直接 `uses` 边；
- `AgentValidationReceipt`：Gate、验证器、输入 Hash、结果、证据等级、错误码及失败 assertion；
- `AgentArtifactGraph`：依赖扫描、回执生成和反向受影响范围查询；
- `AgentArtifactGraph` Godot 单例；
- CLI 扫描脚本；
- 模块单元测试。

## 4. 构建与测试

构建命令：

```powershell
py -m SCons platform=windows target=editor tests=yes module_agent_debugger_enabled=yes
```

结果：成功。

模块测试：

```powershell
bin\godot.windows.editor.x86_64.console.exe `
  --test `
  --test-case="*[AgentDebugger]*" `
  --no-colors
```

结果：

```text
test cases: 5 passed, 0 failed
assertions: 31 passed, 0 failed
```

## 5. 真实项目验证

项目：`agent_research_projects/upstream/godot-demo-projects/2d/dodge_the_creeps`

首次扫描暴露 `.godot/imported` 尚未生成。图片、音频和字体源文件存在，但加载器无法找到导入产物，因此产生 13 个失败回执。

这证明 Import 是 Native Parse 的独立前置阶段，不能把“尚未导入”误报为源码语法错误。

执行：

```powershell
bin\godot.windows.editor.x86_64.console.exe `
  --headless `
  --path <dodge_the_creeps> `
  --import
```

随后执行：

```powershell
bin\godot.windows.editor.x86_64.console.exe `
  --headless `
  --path <dodge_the_creeps> `
  --script <scan_artifact_graph.gd> `
  -- `
  --summary-only
```

最终结果：

```json
{
  "artifact_count": 21,
  "dependency_count": 20,
  "failed_receipt_count": 0,
  "receipt_count": 21,
  "schema_version": "0.2"
}
```

资源导入后，Resource UID Cache 生效，Artifact ID 从路径回退值升级为稳定的 `uid://...`。

## 6. 编译 Gate 实际发现的问题

首次测试构建发现：

- 序列化实现缺少完整 `Variant/Array` 类型包含；
- 一个局部变量遮蔽结构体成员；
- doctest 不能直接比较 `Variant` 与 `StringName`。

以上问题均在编译阶段阻断并已修复。这说明编译 Gate 应保留，但只需调用原生工具链，不需要重复实现语法规则。

## 7. 当前保证边界

当前可以保证：

- 指定根的静态 ResourceLoader 依赖闭包可构造；
- 闭包中资源存在并能被原生加载器加载；
- 每个输入拥有 SHA-256 和验证回执；
- 依赖边具有确定的构造来源；
- 模块 API 和基本失败语义通过单元测试。

当前不能保证：

- 项目所有孤立脚本均已检查；
- Signal、NodePath 和业务契约正确；
- 运行时状态正确；
- 渲染输入或最终图像正确；
- 增量重算与全量重算已经满足 from-scratch consistency。

## 8. 下一步

下一轮优先实现：

1. `project.godot` 主场景和 InputMap Artifact；
2. Scene Node、Script Attach 和 Signal Connection Artifact；
3. Dodge the Creeps 首批 Semantic Contract；
4. 人工注入四类缺陷并记录第一个失败 Gate；
5. 增量缓存及 from-scratch consistency 对照测试。
