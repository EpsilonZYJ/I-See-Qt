# I-See 视频生成客户端

<div align="center">

![Version](https://img.shields.io/badge/版本-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/平台-macOS%20%7C%20Windows%20%7C%20Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-orange.svg)
![License](https://img.shields.io/badge/许可证-MIT-blue.svg)

**基于 Qt6 的强大 AI 视频生成桌面客户端，支持全面的任务管理**

[English](README.md) | 简体中文

[功能特性](#-功能特性) • [安装说明](#-安装说明) • [使用指南](#-使用指南) • [架构设计](#-架构设计) • [参与贡献](#-参与贡献)

</div>

---

## 📖 项目简介

I-See 视频生成客户端是一款使用 Qt6 构建的现代化跨平台桌面应用程序，为 AI 驱动的视频生成提供无缝的用户界面。它提供实时任务监控、自动视频下载以及基于持久化存储的完善历史管理系统。

### 🎯 核心亮点

- 🎬 **AI 视频生成**：提交文本到视频生成任务，支持自定义参数
- 📊 **实时监控**：自动轮询和进度跟踪视频生成任务
- 💾 **智能存储**：基于 SQLite 的持久化存储，支持自动视频下载
- 🔍 **任务历史**：查询和管理所有历史任务及详细信息
- 🎥 **集成播放**：内置视频播放器和系统默认播放器支持
- 🌐 **跨平台**：原生支持 macOS、Windows 和 Linux

## ✨ 功能特性

### 核心功能

#### 🎨 视频生成
- 使用自定义提示词提交文本到视频生成任务
- 可配置参数（分辨率、宽高比、时长等）
- 实时状态更新和进度指示器
- 失败请求的自动重试机制

#### 📋 任务管理
- **持久化存储**：所有任务保存到 SQLite 数据库
- **任务查询**：通过 Task ID 搜索任务
- **状态跟踪**：实时状态更新（排队中 → 处理中 → 已完成/失败）
- **手动操作**：刷新和删除单个任务

#### 💿 自动下载
- 自动下载已完成的视频到本地存储
- 进度条显示下载状态
- 智能重复检测避免重复下载
- 有序的文件命名：`{taskId}_{timestamp}.mp4`

#### 🎥 视频播放
- **内置播放器**：主窗口集成的视频播放器
- **系统播放器**：双击使用默认系统播放器打开
- **历史访问**：即时播放任何历史视频

### 高级功能

#### 📊 任务历史窗口
- 包含可排序列的综合任务列表
- 详细的任务信息面板
- 对待处理任务每 30 秒自动刷新
- 彩色状态指示器
- 通过 Task ID 查询任务
- 删除单个任务

#### 🔄 智能轮询
- 对未完成任务自动状态轮询
- 可配置的轮询间隔（默认：活动任务 3 秒，后台任务 30 秒）
- 智能缓存减少 API 调用
- 优雅的错误处理和重试逻辑

#### 🗄️ 数据管理
- **数据库架构**：优化的 SQLite，带索引查询
- **文件组织**：视频存储在 `~/Movies/I See/`
- **数据完整性**：持久化存储，自动任务跟踪

## 🚀 安装说明

### 系统要求

- **Qt 6.x**（推荐 6.2 或更高版本）
- **CMake 3.16+**
- **C++23 兼容编译器**
  - macOS: Xcode 14+ 配合 Apple Clang
  - Windows: MSVC 2022 或 MinGW GCC 12+
  - Linux: GCC 12+ 或 Clang 15+

### 从源码构建

#### macOS

```bash
# 通过 Homebrew 安装 Qt6
brew install qt@6

# 克隆仓库
git clone https://github.com/yourusername/I-See-Qt.git
cd I-See-Qt

# 创建构建目录
mkdir build && cd build

# 配置并构建
cmake -G Ninja -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
ninja

# 运行应用程序
./I-See.app/Contents/MacOS/I-See
```

#### Windows

```bash
# 从官方网站安装 Qt6: https://www.qt.io/download

# 克隆仓库
git clone https://github.com/yourusername/I-See-Qt.git
cd I-See-Qt

# 创建构建目录
mkdir build
cd build

# 使用 CMake 配置（调整 Qt 路径）
cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64" ..

# 构建
cmake --build . --config Release

# 运行
Release\I-See.exe
```

#### Linux

```bash
# 安装 Qt6（Ubuntu/Debian）
sudo apt-get install qt6-base-dev qt6-multimedia-dev libqt6sql6-sqlite

# 克隆仓库
git clone https://github.com/yourusername/I-See-Qt.git
cd I-See-Qt

# 创建构建目录
mkdir build && cd build

# 配置并构建
cmake -G Ninja ..
ninja

# 运行
./I-See
```

## 📚 使用指南

### 快速开始

1. **启动应用程序**
   ```bash
   ./I-See
   ```

2. **配置 API Key**
   - 在设置面板输入您的 API Key
   - 密钥会安全保存供将来使用

3. **生成视频**
   - 在文本框中输入提示词
   - 点击"生成视频"按钮
   - 观看实时进度更新

4. **查看任务历史**
   - 点击"查看任务历史"按钮
   - 浏览所有生成任务
   - 双击任何任务播放视频

### 主要功能

#### 提交视频生成任务

1. 在主窗口的提示词输入框输入描述
2. 系统会自动使用以下参数：
   - 分辨率：1080p
   - 宽高比：16:9
   - 时长：5 秒
   - 尺寸：1280x720
3. 点击"生成视频"按钮
4. 实时查看状态和进度

#### 通过 Task ID 查询任务

1. 打开"任务历史"窗口
2. 在顶部输入框输入 Task ID
3. 输入 API Key（会自动填充已保存的）
4. 点击"查询"按钮
5. 系统会自动：
   - 查询任务状态
   - 下载完成的视频
   - 保存到数据库

#### 管理历史任务

1. 在任务历史窗口查看所有任务
2. 点击任务查看详细信息
3. 双击任务使用系统默认播放器播放
4. 使用"刷新"按钮手动更新状态
5. 使用"删除"按钮删除不需要的任务

### API 配置

应用程序使用以下 API 端点：

```cpp
// 视频生成
POST https://api.ppinfra.com/v3/async/seedance-v1-pro-t2v

// 任务查询
GET https://api.ppinfra.com/v3/async/task-result?task_id={taskId}
```

### 数据存储位置

#### 数据库位置
- **macOS**: `~/Library/Application Support/ISeeOrg/I See/tasks.db`
- **Windows**: `%APPDATA%/ISeeOrg/I See/tasks.db`
- **Linux**: `~/.local/share/ISeeOrg/I See/tasks.db`

#### 视频存储位置
- **macOS**: `~/Movies/I See/`
- **Windows**: `%USERPROFILE%\Videos\I See\`
- **Linux**: `~/Videos/I See/`

## 📊 功能对比

| 功能 | I-See 客户端 | Web 界面 | CLI 工具 |
|------|-------------|----------|---------|
| GUI 界面 | ✅ 原生桌面 | ✅ 基于浏览器 | ❌ |
| 离线访问 | ✅ | ❌ | ✅ |
| 任务历史 | ✅ SQLite 数据库 | ⚠️ 基于会话 | ❌ |
| 自动下载 | ✅ | ⚠️ 手动 | ✅ |
| 视频播放器 | ✅ 内置 + 系统 | ⚠️ 基于浏览器 | ❌ |
| Task ID 查询 | ✅ | ⚠️ 有限 | ✅ |
| 跨平台 | ✅ macOS/Win/Linux | ✅ | ⚠️ 平台特定 |

## 🎯 使用场景

### 场景 1：内容创作者
- 快速生成视频素材
- 管理多个创作任务
- 本地存储便于编辑

### 场景 2：开发者
- 集成 AI 视频生成到工作流
- 批量生成测试视频
- API 测试和调试

### 场景 3：研究人员
- 实验不同的提示词
- 跟踪生成历史
- 数据分析和可视化

## 🤝 参与贡献

我们欢迎社区贡献！以下是您可以帮助的方式：

### 开发设置

1. Fork 仓库
2. 创建特性分支（`git checkout -b feature/amazing-feature`）
3. 进行更改
4. 运行测试确保代码质量
5. 提交更改（`git commit -m 'Add amazing feature'`）
6. 推送到分支（`git push origin feature/amazing-feature`）
7. 开启 Pull Request

详细的贡献指南请参阅 [CONTRIBUTING.md](docs/CONTRIBUTING.md)

### 编码规范

- 遵循 [Qt 编码风格](https://wiki.qt.io/Qt_Coding_Style)
- 适当使用现代 C++23 特性
- 编写清晰的提交消息
- 为新功能添加文档
- 确保跨平台兼容性


## 💡 技术特点

### 性能优化
- 智能轮询减少 API 调用
- SQLite 索引优化查询
- 异步下载避免 UI 阻塞
- 内存管理使用 Qt 的父子对象树

### 安全性
- API Key 安全存储在系统配置中
- HTTPS 通信
- 输入验证和清理
- 错误处理和日志记录

### 用户体验
- 直观的界面设计
- 实时反馈和进度显示
- 友好的错误消息
- 快捷键支持

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🙏 致谢

- 使用 [Qt Framework](https://www.qt.io/) 构建

## ⭐ Star 历史

[![Star History Chart](https://api.star-history.com/svg?repos=yourusername/I-See-Qt&type=Date)](https://star-history.com/#yourusername/I-See-Qt&Date)

---

<div align="center">

**使用 ❤️ 和 Qt6、C++23 构建**

[⬆ 回到顶部](#i-see-视频生成客户端)

</div>

