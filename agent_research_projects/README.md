# Godot Agent 研究项目池

## 目录约定

所有外部工程都放在 Godot 引擎仓库的以下相对目录：

```text
<ENGINE_ROOT>/agent_research_projects/upstream/<directory>
```

`<ENGINE_ROOT>` 可以位于任意绝对路径。验证脚本和文档只约定相对目录，不要求必须使用 `D:\godot`。

当前首个验证工程的规范路径：

```text
<ENGINE_ROOT>/agent_research_projects/upstream/godot-demo-projects/2d/dodge_the_creeps
```

## 上游锁定清单

所有上游项目均以浅克隆固定当前 commit。`upstream/` 只作为只读基线；Agent 实验应从这些基线创建独立 worktree 或副本，不能直接修改上游目录。

| 目录 | URL | 固定 commit | Godot 工程入口 | 许可证摘要 |
|---|---|---|---|---|
| `godot-demo-projects` | `https://github.com/godotengine/godot-demo-projects.git` | `9ed97d10cac6750ec2f84441d2a7c64b3f527a08` | `2d/dodge_the_creeps/project.godot`；仓库内共 138 个 Demo | MIT |
| `gamecraft-bench` | `https://github.com/tongxuluo/gamecraft-bench.git` | `7ca06072e1160a0092a2bf0c2b3bcf4204982f5d` | `tasks/<task-id>/solution/files/project.godot` | Apache-2.0；任务资源另行核验 |
| `gdquest-getting-started` | `https://github.com/gdquest-demos/getting-started-with-godot-4.git` | `940f23e54e0207cc1135c2f6dc7fdd1ce5b52127` | `2d-project-completed/project.godot`、`3d-project-completed/project.godot`、两个 starter project | 代码 MIT；美术 CC-BY-NC-SA-4.0 |
| `godot-game-template` | `https://github.com/Maaack/Godot-Game-Template.git` | `93e66a02b979872b870ccb5ac1452f8d6a31c737` | `project.godot` | MIT |
| `flick` | `https://github.com/codernunk/flick.git` | `1de309ba12e98f2a06f95b9681f763329264fafd` | `project.godot` | 代码 MIT；资产 CC-BY-4.0 |
| `roguelike-example` | `https://github.com/statico/godot-roguelike-example.git` | `5fa12fd3206df9ce42284cf7aab6b41b69b7fca6` | `project.godot` | 代码 MIT；部分资产有独立许可 |

## 一次性拉取全部可用上游

在 Godot 引擎仓库根目录运行以下 PowerShell。命令要求目标目录尚不存在，以免覆盖已有实验或本地修改。

```powershell
$engineRoot = (Get-Location).Path
$upstreamRoot = Join-Path $engineRoot "agent_research_projects\upstream"

$repositories = @(
    @{
        Directory = "godot-demo-projects"
        Url = "https://github.com/godotengine/godot-demo-projects.git"
        Commit = "9ed97d10cac6750ec2f84441d2a7c64b3f527a08"
    },
    @{
        Directory = "gamecraft-bench"
        Url = "https://github.com/tongxuluo/gamecraft-bench.git"
        Commit = "7ca06072e1160a0092a2bf0c2b3bcf4204982f5d"
    },
    @{
        Directory = "gdquest-getting-started"
        Url = "https://github.com/gdquest-demos/getting-started-with-godot-4.git"
        Commit = "940f23e54e0207cc1135c2f6dc7fdd1ce5b52127"
    },
    @{
        Directory = "godot-game-template"
        Url = "https://github.com/Maaack/Godot-Game-Template.git"
        Commit = "93e66a02b979872b870ccb5ac1452f8d6a31c737"
    },
    @{
        Directory = "flick"
        Url = "https://github.com/codernunk/flick.git"
        Commit = "1de309ba12e98f2a06f95b9681f763329264fafd"
    },
    @{
        Directory = "roguelike-example"
        Url = "https://github.com/statico/godot-roguelike-example.git"
        Commit = "5fa12fd3206df9ce42284cf7aab6b41b69b7fca6"
    }
)

New-Item -ItemType Directory -Force -Path $upstreamRoot | Out-Null

foreach ($repository in $repositories) {
    $destination = Join-Path $upstreamRoot $repository.Directory
    if (Test-Path -LiteralPath $destination) {
        throw "Refusing to overwrite existing upstream directory: $destination"
    }

    git clone --filter=blob:none --no-checkout $repository.Url $destination
    if ($LASTEXITCODE -ne 0) {
        throw "Clone failed: $($repository.Url)"
    }

    git -C $destination fetch --depth 1 origin $repository.Commit
    if ($LASTEXITCODE -ne 0) {
        throw "Fetch failed: $($repository.Commit)"
    }

    git -C $destination checkout --detach $repository.Commit
    if ($LASTEXITCODE -ne 0) {
        throw "Checkout failed: $($repository.Commit)"
    }

    $actualCommit = git -C $destination rev-parse HEAD
    if ($actualCommit -ne $repository.Commit) {
        throw "Commit mismatch in $destination"
    }
}
```

拉取完成后，首个验证项目路径为：

```powershell
$project = Join-Path $engineRoot "agent_research_projects\upstream\godot-demo-projects\2d\dodge_the_creeps"
```

## 给 Agent 的复刻指令

可以直接下达：

> 读取 `agent_research_projects/README.md` 的“上游锁定清单”，把所有可用上游仓库拉取到声明的相对目录，逐一 checkout 到固定 commit，并校验实际 HEAD。不得覆盖已存在目录，不得提交 `upstream/`，不得修改只读基线。完成后使用 `godot-demo-projects/2d/dodge_the_creeps` 作为第一个验证项目。

## 暂不纳入自动拉取的项目

COGITO URL 为 `https://codeberg.org/Phazorknight/Cogito.git`。当前最后尝试的 commit 是 `a9ecf308532173386da0466fe6903d2727ac2e08`，但两次浅克隆均因上游连接中断而未得到可信的完整工作树，因此不进入 `$repositories`，也不得作为实验基线。

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
