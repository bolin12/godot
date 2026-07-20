# Godot Agent 研究项目池

## 已下载项目

所有上游项目均以浅克隆固定当前 commit。`upstream/` 只作为只读基线；Agent 实验应从这些基线创建独立 worktree 或副本，不能直接修改上游目录。

| 项目 | 固定 commit | 项目数 | 许可证摘要 | 推荐用途 |
|---|---|---:|---|---|
| `godot-demo-projects` | `9ed97d10cac6750ec2f84441d2a7c64b3f527a08` | 138 | MIT | 小型、单机制任务；生命周期、信号、资源、物理、GUI |
| `gamecraft-bench` | `7ca06072e1160a0092a2bf0c2b3bcf4204982f5d` | 38 个参考 Godot 工程、140 个自然语言任务 | Apache-2.0；任务资源另行核验 | 自然语言生成、交互重放、Rubric 评价 |
| `gdquest-getting-started` | `940f23e54e0207cc1135c2f6dc7fdd1ce5b52127` | 4 | 代码 MIT；美术 CC-BY-NC-SA-4.0 | 小型完整 2D/3D 功能任务；研究用途 |
| `godot-game-template` | `93e66a02b979872b870ccb5ac1452f8d6a31c737` | 1 | MIT | 菜单、选项、暂停、场景加载与存档类需求 |
| `flick` | `1de309ba12e98f2a06f95b9681f763329264fafd` | 1 | 代码 MIT；资产 CC-BY-4.0 | 中小型 2D 平台游戏的自然功能迭代 |
| `roguelike-example` | `5fa12fd3206df9ce42284cf7aab6b41b69b7fca6` | 1 | 代码 MIT；部分资产有独立许可 | 地牢生成、视野、库存、行为树、回合状态 |

COGITO 已迁移到 Codeberg，但两次浅克隆均因上游连接中断而无法完成 checkout；不完整下载保存在 `_incomplete_cogito_download`，不得作为实验基线。

## 推荐启用顺序

1. `godot-demo-projects/2d/dodge_the_creeps`
2. `godot-demo-projects/gui/control_gallery` 或 `gui/drag_and_drop`
3. `godot-demo-projects/loading` 下的资源加载样例
4. `godot-game-template`
5. `flick` 或 `roguelike-example`
6. `gamecraft-bench` 的低复杂度任务

`gamecraft-bench` 的原始运行环境面向 Ubuntu 22.04、Godot 4.6.2、Xvfb 和 Harbor。在 Windows 上先复用它的 `instruction.md`、`rubric.json`、参考工程与 demo trace 结构，不直接照搬 Linux Runner。

## 工作区纪律

- `upstream/`：只读上游快照。
- `worktrees/<episode_id>/`：Agent 实际修改的隔离工程。
- `episodes/<episode_id>/`：需求、操作、Trace、补丁和评价结果。
- `benchmarks/<bug_id>/`：从已修复 Episode 提炼的可重复 Bug。
- 每次实验固定 engine commit、project commit、Agent 配置和随机种子。
