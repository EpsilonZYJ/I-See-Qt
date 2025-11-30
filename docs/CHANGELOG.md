# Changelog

All notable changes to I-See Video Generator Client will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned
- Batch video generation
- Advanced task filtering
- Export functionality (CSV/JSON)
- Dark mode theme

## [1.0.0] - 2025-12-01

### Added
- Initial release of I-See Video Generator Client
- Text-to-video generation with AI API integration
- Real-time task monitoring and status updates
- SQLite-based persistent task history
- Automatic video downloading to local storage
- Built-in video player with Qt Multimedia
- System default player integration (double-click to play)
- Task query by Task ID
- Comprehensive task history window with:
  - Sortable task list
  - Detailed task information panel
  - Auto-refresh for pending tasks (30-second interval)
  - Color-coded status indicators
  - Batch operations (refresh, delete)
- Smart download management:
  - Automatic download on task completion
  - Progress bar for download status
  - Duplicate detection
  - Organized file naming
- Cross-platform support:
  - macOS (Apple Silicon and Intel)
  - Windows (MSVC 2022)
  - Linux (Ubuntu, Debian, Fedora)

### Technical Features
- MVVM architecture pattern
- Qt 6.x framework
- C++23 modern features
- CMake build system
- Indexed SQLite database for performance
- Asynchronous network operations
- Signal-slot event handling
- Parent-child memory management

### User Interface
- Clean and intuitive main window
- Responsive task history browser
- Real-time progress indicators
- Status bar notifications
- Error dialogs with detailed messages
- Tooltip helpers

### API Integration
- Seedance AI video generation API (v3)
- RESTful API communication
- JSON request/response handling
- Bearer token authentication
- Automatic retry on failure
- Configurable polling intervals

### Data Management
- SQLite database for task persistence
- Automatic schema creation and migration
- Indexed queries for performance
- Transaction support
- Data integrity checks
- Automatic backup on exit

### Documentation
- Comprehensive README with installation guide
- Contributing guidelines
- API update documentation
- Feature-specific guides:
  - Task History Management
  - Auto Download Feature
  - Task ID Query
  - Double-Click Play
- Chinese and English documentation

## Version History Notes

### Release Process
1. Update version in CMakeLists.txt
2. Update CHANGELOG.md with new features and fixes
3. Create git tag: `git tag -a v1.0.0 -m "Release version 1.0.0"`
4. Push tag: `git push origin v1.0.0`
5. Create GitHub release with binaries

### Compatibility
- **Qt**: 6.2+
- **CMake**: 3.16+
- **Compiler**: C++23 support required
- **Database**: SQLite 3.x

### Known Issues in 1.0.0
- Large video files (>500MB) may cause memory pressure on 32-bit systems
- Network interruption during download requires manual retry
- Task history window performance degrades with 1000+ tasks
- Some Unicode characters in filenames may cause issues on Windows

### Deprecated
- N/A (initial release)

### Security
- API keys stored in system preferences (plaintext - encryption planned for 1.1)
- HTTPS communication for all API calls
- Input validation for user data
- SQL injection prevention through parameterized queries

---

## Release Checklist

Before each release:
- [ ] Update version number in CMakeLists.txt
- [ ] Update CHANGELOG.md
- [ ] Run full test suite
- [ ] Test on all supported platforms
- [ ] Update documentation
- [ ] Create git tag
- [ ] Build release binaries
- [ ] Create GitHub release
- [ ] Update website/announcement

## Versioning Guidelines

- **Major version** (X.0.0): Breaking API changes, major features
- **Minor version** (1.X.0): New features, backwards compatible
- **Patch version** (1.0.X): Bug fixes, minor improvements

## Support

- **Version 1.0.x**: Active development and support
- **Future versions**: LTS support planned for major releases

---

[Unreleased]: https://github.com/yourusername/I-See-Qt/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/yourusername/I-See-Qt/releases/tag/v1.0.0

