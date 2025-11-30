# I-See Video Generator Client

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows%20%7C%20Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.x-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

**A powerful Qt-based desktop client for AI video generation with comprehensive task management**

[Features](#-features) • [Installation](#-installation) • [Usage](#-usage) • [Architecture](#-architecture) • [Contributing](#-contributing)

</div>

---

## 📖 Overview

I-See Video Generator Client is a modern, cross-platform desktop application built with Qt6 that provides a seamless interface for AI-powered video generation. It offers real-time task monitoring, automatic video downloading, and a comprehensive history management system with persistent storage.

### 🎯 Key Highlights

- 🎬 **AI Video Generation**: Submit text-to-video generation tasks with customizable parameters
- 📊 **Real-time Monitoring**: Automatic polling and progress tracking for video generation tasks
- 💾 **Smart Storage**: SQLite-based persistent storage with automatic video downloading
- 🔍 **Task History**: Query and manage all historical tasks with detailed information
- 🎥 **Integrated Playback**: Built-in video player and system default player support
- 🌐 **Cross-platform**: Native support for macOS, Windows, and Linux

## ✨ Features

### Core Features

#### 🎨 Video Generation
- Submit text-to-video generation tasks with custom prompts
- Configurable parameters (resolution, aspect ratio, duration, etc.)
- Real-time status updates with progress indicators
- Automatic retry mechanism for failed requests

#### 📋 Task Management
- **Persistent Storage**: All tasks saved to SQLite database
- **Task Query**: Search tasks by Task ID with API Key input
- **Status Tracking**: Real-time status updates (Queued → Processing → Completed/Failed)
- **Manual Operations**: Refresh and delete individual tasks

#### 💿 Automatic Download
- Auto-download completed videos to local storage
- Progress bar showing download status
- Smart duplicate detection to avoid re-downloading
- Organized file naming: `{taskId}_{timestamp}.mp4`

#### 🎥 Video Playback
- **Built-in Player**: Integrated video player in main window
- **System Player**: Double-click to open with default system player
- **History Access**: Play any historical video instantly

### Advanced Features

#### 📊 Task History Window
- Comprehensive task list with sortable columns
- Detailed task information panel
- Auto-refresh every 30 seconds for pending tasks
- Color-coded status indicators
- Task query by Task ID
- Individual task deletion

#### 🔄 Intelligent Polling
- Automatic status polling for incomplete tasks
- Configurable polling intervals (default: 3 seconds for active, 30 seconds for background)
- Smart caching to reduce API calls
- Graceful error handling and retry logic

#### 🗄️ Data Management
- **Database Schema**: Optimized SQLite with indexed queries
- **File Organization**: Videos stored in `~/Movies/VideoGenClient/`
- **Data Integrity**: Persistent storage with automatic task tracking

## 🚀 Installation

### Prerequisites

- **Qt 6.x** (6.2 or higher recommended)
- **CMake 3.16+**
- **C++23 compatible compiler**
  - macOS: Xcode 14+ with Apple Clang
  - Windows: MSVC 2022 or MinGW with GCC 12+
  - Linux: GCC 12+ or Clang 15+

### Build from Source

#### macOS

```bash
# Install Qt6 via Homebrew
brew install qt@6

# Clone the repository
git clone https://github.com/yourusername/I-See-Qt.git
cd I-See-Qt

# Create build directory
mkdir build && cd build

# Configure and build
cmake -G Ninja -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
ninja

# Run the application
./VideoGenClient.app/Contents/MacOS/VideoGenClient
```

#### Windows

```bash
# Install Qt6 from official installer: https://www.qt.io/download

# Clone the repository
git clone https://github.com/yourusername/I-See-Qt.git
cd I-See-Qt

# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt path)
cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64" ..

# Build
cmake --build . --config Release

# Run
Release\VideoGenClient.exe
```

#### Linux

```bash
# Install Qt6 (Ubuntu/Debian)
sudo apt-get install qt6-base-dev qt6-multimedia-dev libqt6sql6-sqlite

# Clone the repository
git clone https://github.com/yourusername/I-See-Qt.git
cd I-See-Qt

# Create build directory
mkdir build && cd build

# Configure and build
cmake -G Ninja ..
ninja

# Run
./VideoGenClient
```

## 📚 Usage

### Quick Start

1. **Launch the Application**
   ```bash
   ./VideoGenClient
   ```

2. **Configure API Key**
   - Enter your API Key in the settings panel
   - The key is securely stored for future sessions

3. **Generate a Video**
   - Enter your prompt in the text box
   - Click "Generate Video" button
   - Watch real-time progress updates

4. **View Task History**
   - Click "View Task History" button
   - Browse all your generation tasks
   - Double-click any task to play the video

### API Configuration

The application uses the following API endpoints:

```cpp
// Video Generation
POST https://api.ppinfra.com/v3/async/seedance-v1-pro-t2v

// Task Query
GET https://api.ppinfra.com/v3/async/task-result?task_id={taskId}
```

### Database Location

- **macOS**: `~/Library/Application Support/VideoGenClient/tasks.db`
- **Windows**: `%APPDATA%/VideoGenClient/tasks.db`
- **Linux**: `~/.local/share/VideoGenClient/tasks.db`

### Video Storage Location

- **macOS**: `~/Movies/VideoGenClient/`
- **Windows**: `%USERPROFILE%\Videos\VideoGenClient\`
- **Linux**: `~/Videos/VideoGenClient/`

## 🏗️ Architecture

### Project Structure

```
I-See-Qt/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── const/
│   │   ├── AppConfig.h            # Configuration constants
│   │   └── QtHeaders.h            # Centralized Qt headers
│   ├── models/
│   │   └── TaskItem.h             # Task data model
│   ├── services/
│   │   ├── ApiService.*           # API communication layer
│   │   ├── HistoryService.*       # Local history management
│   │   └── TaskDatabaseService.*  # SQLite database operations
│   ├── viewmodel/
│   │   └── MainViewModel.*        # MVVM pattern view model
│   ├── ui/
│   │   ├── MainWindow.*           # Main application window
│   │   ├── TaskHistoryWindow.*    # Task history browser
│   │   └── SetupDialog.*          # Configuration dialog
│   └── utils/                     # Utility functions
├── resources/
│   └── ui.qrc                     # Qt resource file
├── CMakeLists.txt                 # CMake build configuration
└── README.md                      # This file
```

### Technology Stack

- **GUI Framework**: Qt 6.x (Widgets)
- **Networking**: Qt Network (QNetworkAccessManager)
- **Database**: SQLite via Qt SQL module
- **Multimedia**: Qt Multimedia (Video playback)
- **Build System**: CMake 3.16+
- **Language**: C++23

### Design Patterns

- **MVVM** (Model-View-ViewModel): Clean separation of UI and business logic
- **Observer Pattern**: Signal-slot mechanism for event handling
- **Service Layer**: Abstracted API and database operations
- **Singleton Pattern**: Configuration and service management

## 📊 Features Comparison

| Feature | I-See Client | Web Interface | CLI Tools |
|---------|--------------|---------------|-----------|
| GUI Interface | ✅ Native Desktop | ✅ Browser-based | ❌ |
| Offline Access | ✅ | ❌ | ✅ |
| Task History | ✅ SQLite Database | ⚠️ Session-based | ❌ |
| Auto Download | ✅ | ⚠️ Manual | ✅ |
| Video Player | ✅ Built-in + System | ⚠️ Browser-based | ❌ |
| Task Query by ID | ✅ | ⚠️ Limited | ✅ |
| Cross-platform | ✅ macOS/Win/Linux | ✅ | ⚠️ Platform-specific |

## 🤝 Contributing

We welcome contributions from the community! Here's how you can help:

### Development Setup

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Run tests and ensure code quality
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

### Coding Standards

- Follow [Qt Coding Style](https://wiki.qt.io/Qt_Coding_Style)
- Use modern C++23 features where appropriate
- Write clear commit messages
- Add documentation for new features
- Ensure cross-platform compatibility

### Bug Reports

If you find a bug, please open an issue with:
- Clear description of the problem
- Steps to reproduce
- Expected vs actual behavior
- System information (OS, Qt version, etc.)
- Screenshots if applicable

## 📝 Documentation

Additional documentation is available in the project root:

- [API Update Guide](API_UPDATE_GUIDE.md) - API integration details
- [Task History Guide](TASK_HISTORY_GUIDE.md) - Task management features
- [Auto Download Feature](AUTO_DOWNLOAD_FEATURE.md) - Video download mechanism
- [Task ID Query Feature](TASK_ID_QUERY_FEATURE.md) - Query by task ID
- [Double Click Play Feature](DOUBLE_CLICK_PLAY_FEATURE.md) - Video playback

## 🐛 Known Issues

- [ ] Large video files (>500MB) may cause memory issues on 32-bit systems
- [ ] Network interruption during download requires manual retry
- [ ] Task history window may lag with 1000+ tasks (optimization in progress)

## 🗺️ Roadmap

### Version 1.1 (Planned)
- [ ] Batch video generation
- [ ] Advanced filtering and search in task history
- [ ] Export task history to CSV/JSON
- [ ] Dark mode support
- [ ] Video thumbnail preview

### Version 1.2 (Planned)
- [ ] Advanced video editing capabilities
- [ ] Cloud sync for task history
- [ ] Multi-language support (Chinese, English, Japanese)
- [ ] Batch task operations

### Version 2.0 (Planned)
- [ ] Plugin system for custom video processors
- [ ] REST API for automation
- [ ] Collaborative features
- [ ] Performance optimizations for large-scale usage

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with [Qt Framework](https://www.qt.io/)
- Video generation powered by [Seedance AI API](https://api.ppinfra.com/)
- Icons from [Qt Resources](https://doc.qt.io/qt-6/resources.html)
- Inspired by modern desktop application design principles

## 📧 Contact

- **Author**: Zhou Yujie (EpsilonZYJ)
- **Email**: your.email@example.com
- **GitHub**: [@yourusername](https://github.com/yourusername)
- **Issues**: [GitHub Issues](https://github.com/yourusername/I-See-Qt/issues)

## ⭐ Star History

[![Star History Chart](https://api.star-history.com/svg?repos=yourusername/I-See-Qt&type=Date)](https://star-history.com/#yourusername/I-See-Qt&Date)

---

<div align="center">

**Made with ❤️ using Qt6 and C++23**

[⬆ Back to Top](#i-see-video-generator-client)

</div>

