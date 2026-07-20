# Causal-Consistent Reversible Debugging：因果一致可逆调试

## 基本信息

- 原文：[arXiv:2406.05719](https://arxiv.org/abs/2406.05719)
- PDF：[Causal_Consistent_Reversible_Debugging.pdf](Causal_Consistent_Reversible_Debugging.pdf)
- 研究类型：并发程序可逆语义和重放调试
- 与 Godot 研究关联：P2，底层基础设施方向

## 方法分析

论文研究 Erlang 并发程序中的因果一致回退。一个动作只有在所有依赖它的动作都撤销后才能撤销，因此回退不要求简单地按全局时间逆序执行。论文还统一了用户驱动调试和 Replay Debugging，并改进了 CauDEr 调试器。

## 对 Godot 的意义

Godot 的 Signal、协程、异步资源加载、RPC 和 WorkerThreadPool 都存在先后依赖。“恢复某个属性”并不等于恢复运行时状态。需要记录对象创建/销毁、消息发送/接收、异步完成顺序、随机种子、输入、外部资源版本和不可逆副作用。

第一版建议限制为单线程、固定随机种子、固定输入和有限 Signal 模型，先验证快照、事件回退和确定性重放，再扩展到网络与多线程。

## 局限

论文面向 Erlang 语义，不是 Godot 或游戏引擎实现；它没有解决渲染、物理、GPU、外部文件和网络服务等不可逆外部副作用。

## 研究结论

它为 Godot 的 State Snapshot、事件日志和因果回退提供理论基础。重点不是实现“回到上一帧”，而是实现“在依赖允许的情况下回到某个可解释状态”。
