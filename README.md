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

- 🎬 **AI Video Generation**: Submit text-to-video and image-to-video generation tasks with customizable parameters
- 🖼️ **Image-to-Video**: Generate videos from static images with optional start and end frames
- 📊 **Real-time Monitoring**: Automatic polling and progress tracking for video generation tasks
- 💾 **Smart Storage**: SQLite-based persistent storage with automatic video downloading
- 🔍 **Task History**: Query and manage all historical tasks with detailed information
- 🎥 **Integrated Playback**: Built-in video player and system default player support
- 🌐 **Cross-platform**: Native support for macOS, Windows, and Linux

## ✨ Features

### Core Features

#### 🎨 Video Generation
- **Text-to-Video**: Generate videos from text prompts with custom parameters
- **Image-to-Video**: Transform static images into dynamic videos
  - Support for start frame (required) and end frame (optional)
  - Multiple image format support: JPEG, PNG, WebP, BMP, TIFF, GIF
  - Automatic Base64 encoding for API compatibility
  - Image preview before submission
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
- **File Organization**: Videos stored in `~/Movies/I See/`
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
git clone https://github.com/EpsilonZYJ/I-See-Qt.git
cd I-See-Qt

# Create build directory
mkdir build && cd build

# Configure and build
cmake -G Ninja -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6 ..
ninja

# Run the application
./I-See.app/Contents/MacOS/I-See
```

#### Windows

```bash
# Install Qt6 from official installer: https://www.qt.io/download

# Clone the repository
git clone https://github.com/EpsilonZYJ/I-See-Qt.git
cd I-See-Qt

# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt path)
cmake -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64" ..

# Build
cmake --build . --config Release

# Run
Release\I-See.exe
```

#### Linux

```bash
# Install Qt6 (Ubuntu/Debian)
sudo apt-get install qt6-base-dev qt6-multimedia-dev libqt6sql6-sqlite

# Clone the repository
git clone https://github.com/EpsilonZYJ/I-See-Qt.git
cd I-See-Qt

# Create build directory
mkdir build && cd build

# Configure and build
cmake -G Ninja ..
ninja

# Run
./I-See
```

## 📚 Usage

### Quick Start

1. **Launch the Application**
   ```bash
   ./I-See
   ```

2. **Configure API Key**
   - Enter your API Key in the settings panel
   - The key is securely stored for future sessions

3. **Generate a Video**
   
   **Text-to-Video:**
   - Select "Text-to-Video" mode
   - Enter your prompt in the text box
   - Click "Generate Video" button
   - Watch real-time progress updates
   
   **Image-to-Video:**
   - Select "Image-to-Video" mode
   - Click "Select First Frame" to choose your starting image
   - (Optional) Click "Select Last Frame" for end frame
   - Enter an optional prompt for better results
   - Click "Generate Video" button
   - Watch real-time progress updates

4. **View Task History**
   - Click "View Task History" button
   - Browse all your generation tasks
   - Double-click any task to play the video

### API Configuration

The application uses the following API endpoints:

```cpp
// Text-to-Video Generation
POST https://api.ppinfra.com/v3/async/seedance-v1-pro-t2v

// Image-to-Video Generation
POST https://api.ppinfra.com/v3/async/seedance-v1-pro-i2v

// Task Query
GET https://api.ppinfra.com/v3/async/task-result?task_id={taskId}
```

#### Image-to-Video Requirements

- **Supported Formats**: JPEG, PNG, WebP, BMP, TIFF, GIF
- **File Size**: Maximum 30MB per image
- **Dimensions**: 
  - Short edge: minimum 300 pixels
  - Long edge: maximum 6000 pixels
  - Aspect ratio: 0.4 to 2.5
- **Input Methods**: URL or Base64 encoding (auto-handled by the client)

### Database Location

- **macOS**: `~/Library/Application Support/ISeeOrg/I See/tasks.db`
- **Windows**: `%APPDATA%/ISeeOrg/I See/tasks.db`
- **Linux**: `~/.local/share/ISeeOrg/I See/tasks.db`

### Video Storage Location

- **macOS**: `~/Movies/I See/`
- **Windows**: `%USERPROFILE%\Videos\I See\`
- **Linux**: `~/Videos/I See/`

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

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with [Qt Framework](https://www.qt.io/)

## ⭐ Star History

[![Star History Chart](https://api.star-history.com/svg?repos=EpsilonZYJ/I-See-Qt&type=Date)](https://star-history.com/#EpsilonZYJ/I-See-Qt&Date)

---

<div align="center">

**Made with ❤️ using Qt6 and C++23**

[⬆ Back to Top](#i-see-video-generator-client)

</div>

