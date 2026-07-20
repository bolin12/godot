# 社区源码快照与复用登记

调研源码固定在 `sources/`。截至本次实现，`modules/agent_debugger` 为洁净的独立实现，尚未逐行复制下列仓库代码；以后实际移植时必须继续追加“上游文件 → 本地文件”的映射和修改说明。

## 已固定的上游版本

| 本地目录 | 上游 commit | 许可证 | 候选复用方向 |
|---|---|---|---|
| `coding-solo_godot-mcp` | `1209744fad78f3998f98c7394fd0f6ef50da5281` | MIT | MCP 工具面、场景操作 |
| `ee0pdt_godot-mcp` | `b7d08eb43ac938bf64ebd27e474619b8cc020ecc` | MIT | 编辑器桥接参考 |
| `erodenn_godot-mcp-runtime` | `92be973965331511acd73630529c40385bf7cdf3` | MIT | 临时运行时桥 |
| `flamxgames_ai-assistant-hub` | `5f3bb865bacb341a00988f64da8ef90b634f419b` | MIT | 权限配置、多模型策略 |
| `godot4-addons_ai-assistant` | `e460f1f63a8e52d667630bb28bd0a480f28e7785` | MIT | 编辑器 Assistant 交互 |
| `hi-godot_godot-ai` | `f94116ce0f75e64350ed03277d48c27d4535fa20` | MIT | 会话封装、游标、原子批处理 |
| `sancheznot_godot-ai-assistant` | `37b539165bf84579c2ff4ba37b0ae6e3ba6c13aa` | MIT | 编辑器对话工作流 |
| `tomyud1_godot-mcp` | `0d3f473922ea270c38df5c17cf061aa9da8470a2` | MIT | MCP 服务与工具定义 |
| `ult7ra_ai-essentials-toolkit` | `ad18111ce22213e568c75ac4fffd405a2f5e2e93` | MIT | 通用 AI 工具集参考 |
| `youichi-uda_godot-mcp-pro` | `c17a182d92f23ae22045598f6105a06a6737707b` | MIT | 编辑器自动化工具面 |
| `yurineko73_godot-mcp-native` | `52d978d4c27b6d04640e40a5741c252076aa5685` | MIT | 原生调试桥、运行时探针 |

## 移植规则

1. 使用表中固定 commit，不从浮动分支直接复制。
2. 复制代码时保留目标文件需要的版权声明，并把完整 MIT 文本随分发物携带。
3. 在本文件追加来源文件、本地文件、修改摘要和复用日期。
4. 即使上表当前均为 MIT，仍需逐文件检查额外声明、生成物和依赖许可证。
5. 外部服务、遥测、任意命令执行和项目写操作默认关闭，经过权限门禁后才能启用。

## 已并入记录

当前无。`modules/agent_debugger` 的 API 与实现由本项目独立编写，仅吸收了调研中总结出的抽象设计思想。
