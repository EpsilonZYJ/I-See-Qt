# Contributing to I-See Video Generator Client

First off, thank you for considering contributing to I-See Video Generator Client! It's people like you that make this tool better for everyone.

## 📋 Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Guidelines](#coding-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Community](#community)

## Code of Conduct

This project and everyone participating in it is governed by our Code of Conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

### Our Pledge

We pledge to make participation in our project a harassment-free experience for everyone, regardless of age, body size, disability, ethnicity, gender identity and expression, level of experience, nationality, personal appearance, race, religion, or sexual identity and orientation.

## Getting Started

### Prerequisites

Before you begin, ensure you have:
- Qt 6.2 or higher installed
- CMake 3.16 or higher
- A C++23 compatible compiler
- Git for version control
- Basic understanding of Qt Widgets and MVVM pattern

### First Contribution

Unsure where to begin? You can start by looking through these issues:
- `good-first-issue` - Issues that should only require a few lines of code
- `help-wanted` - Issues that are a bit more involved than beginner issues

## Development Setup

1. **Fork the repository**
   ```bash
   # Click the 'Fork' button on GitHub
   ```

2. **Clone your fork**
   ```bash
   git clone https://github.com/YOUR_USERNAME/I-See-Qt.git
   cd I-See-Qt
   ```

3. **Add upstream remote**
   ```bash
   git remote add upstream https://github.com/ORIGINAL_OWNER/I-See-Qt.git
   ```

4. **Create a development branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

5. **Build the project**
   ```bash
   mkdir build && cd build
   cmake -G Ninja -DCMAKE_PREFIX_PATH=/path/to/qt6 ..
   ninja
   ```

## How to Contribute

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates. When you create a bug report, include as many details as possible:

**Template for Bug Reports:**
```markdown
**Describe the bug**
A clear and concise description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:
1. Go to '...'
2. Click on '....'
3. Scroll down to '....'
4. See error

**Expected behavior**
A clear description of what you expected to happen.

**Screenshots**
If applicable, add screenshots to help explain your problem.

**Environment:**
 - OS: [e.g. macOS 14.0, Windows 11, Ubuntu 22.04]
 - Qt Version: [e.g. 6.5.3]
 - Compiler: [e.g. Clang 15, MSVC 2022]
 - Version: [e.g. 1.0.0]

**Additional context**
Add any other context about the problem here.
```

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

- **Use a clear and descriptive title**
- **Provide a detailed description** of the suggested enhancement
- **Explain why this enhancement would be useful**
- **List some examples** of how it would be used
- **Include mockups or diagrams** if applicable

### Pull Requests

1. **Update your fork**
   ```bash
   git fetch upstream
   git checkout main
   git merge upstream/main
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/amazing-feature
   ```

3. **Make your changes**
   - Write clear, readable code
   - Add or update tests as needed
   - Update documentation as needed

4. **Test your changes**
   ```bash
   cd build
   ninja
   ./I-See  # Manual testing
   ```

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "feat: add amazing feature"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/amazing-feature
   ```

7. **Create a Pull Request**
   - Go to the original repository
   - Click "New Pull Request"
   - Select your fork and branch
   - Fill in the PR template

## Coding Guidelines

### C++ Style Guide

We follow the [Qt Coding Conventions](https://wiki.qt.io/Qt_Coding_Style) with some modifications:

#### Naming Conventions

```cpp
// Classes: PascalCase
class VideoGenerator { };

// Functions/Methods: camelCase
void generateVideo();

// Variables: camelCase
int taskCount;

// Constants: UPPER_SNAKE_CASE or PascalCase for const objects
const int MAX_RETRIES = 3;
const QString DEFAULT_PATH = "...";

// Private members: camelCase with no prefix/suffix
class MyClass {
private:
    int myVariable;  // Not m_myVariable or myVariable_
};
```

#### Code Formatting

```cpp
// Braces: Same line for functions, new line for classes
void myFunction() {
    if (condition) {
        // code
    }
}

class MyClass
{
public:
    MyClass();
};

// Indentation: 4 spaces, no tabs
void example() {
    if (condition) {
        doSomething();
    }
}

// Line length: Maximum 120 characters
// Break long lines at logical points
QString longString = "This is a very long string that should be "
                     "broken across multiple lines for readability";
```

#### Modern C++ Features

Use C++23 features where appropriate:

```cpp
// Use auto for obvious types
auto reply = manager->get(request);

// Use range-based for loops
for (const auto &task : tasks) {
    processTask(task);
}

// Use nullptr instead of NULL
MyClass *ptr = nullptr;

// Use smart pointers when appropriate
std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();

// Use structured bindings
auto [success, result] = parseResponse();
```

### Qt-Specific Guidelines

```cpp
// Use Qt's parent-child ownership
auto *button = new QPushButton("Click", this);  // 'this' is the parent

// Use Qt's signal-slot mechanism
connect(button, &QPushButton::clicked, this, &MyClass::onButtonClicked);

// Use Qt's foreach only for Qt containers (prefer range-based for)
foreach (const Task &task, tasks) {  // OK for Qt containers
    // ...
}

// Prefer Qt types for Qt APIs
QString text = lineEdit->text();  // Not std::string
QList<int> numbers;                // Not std::vector<int>
```

### File Organization

```cpp
// Header file (.h)
#ifndef MYCLASS_H
#define MYCLASS_H

// Qt includes first
#include <QObject>
#include <QString>

// Project includes
#include "models/TaskItem.h"

class MyClass : public QObject
{
    Q_OBJECT  // Always first in class declaration

public:
    explicit MyClass(QObject *parent = nullptr);
    ~MyClass();

signals:
    void taskCompleted(const QString &id);

public slots:
    void processTask(const Task &task);

private:
    void helperFunction();
    
    // Member variables
    QString taskId;
    QList<Task> tasks;
};

#endif // MYCLASS_H
```

## Commit Messages

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, semicolons, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

### Examples

```bash
# Feature
git commit -m "feat(history): add task filtering by date range"

# Bug fix
git commit -m "fix(download): prevent duplicate downloads"

# Documentation
git commit -m "docs(readme): update installation instructions"

# Breaking change
git commit -m "feat(api)!: migrate to v3 API endpoints

BREAKING CHANGE: API v2 is no longer supported"
```

## Pull Request Process

### Before Submitting

1. ✅ Code compiles without errors or warnings
2. ✅ All existing tests pass
3. ✅ New tests added for new features
4. ✅ Documentation updated
5. ✅ Code follows style guidelines
6. ✅ Commits follow commit message conventions

### PR Template

```markdown
## Description
Brief description of what this PR does.

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## How Has This Been Tested?
Describe the tests you ran and how to reproduce them.

## Checklist
- [ ] My code follows the style guidelines
- [ ] I have performed a self-review
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix is effective or that my feature works
- [ ] New and existing unit tests pass locally with my changes

## Screenshots (if applicable)
Add screenshots to help explain your changes.

## Related Issues
Closes #issue_number
```

### Review Process

1. **Automated Checks**: GitHub Actions will run automated checks
2. **Code Review**: At least one maintainer will review your code
3. **Feedback**: Address any requested changes
4. **Approval**: Once approved, your PR will be merged

### After Merge

- Your contribution will be included in the next release
- You'll be added to the contributors list
- Consider helping with code reviews for other PRs

## Testing

### Manual Testing

Test your changes thoroughly:

1. **Functional Testing**: Does the feature work as expected?
2. **Edge Cases**: Test with unusual inputs or conditions
3. **Cross-platform**: Test on different OS if possible
4. **Performance**: Check for memory leaks or performance issues

### Test Checklist

- [ ] Feature works with valid input
- [ ] Feature handles invalid input gracefully
- [ ] Error messages are clear and helpful
- [ ] UI is responsive and intuitive
- [ ] No memory leaks (use Valgrind or similar)
- [ ] Database operations are correct
- [ ] Network errors are handled properly

## Documentation

### Code Documentation

```cpp
/**
 * @brief Downloads a video from the given URL
 * 
 * This function initiates an asynchronous download of the video
 * and emits signals to track progress.
 * 
 * @param taskId The unique identifier of the task
 * @param videoUrl The URL of the video to download
 * 
 * @note The download is performed asynchronously
 * @warning Ensure the URL is valid before calling
 * 
 * @see onVideoDownloaded()
 */
void downloadVideo(const QString &taskId, const QString &videoUrl);
```

### User Documentation

When adding new features:
1. Update the README.md
2. Add examples to documentation files
3. Update the usage section
4. Add screenshots if applicable

## Community

### Getting Help

- **GitHub Discussions**: For questions and discussions
- **GitHub Issues**: For bug reports and feature requests
- **Email**: For private inquiries

### Staying Updated

- Watch the repository for notifications
- Check the roadmap for upcoming features
- Review closed PRs to learn from others

### Recognition

Contributors are recognized in:
- README.md contributors section
- Release notes
- Project website (if applicable)

## Questions?

Don't hesitate to ask questions! We're here to help:
- Open a GitHub Discussion
- Comment on relevant issues
- Contact maintainers directly

---

**Thank you for contributing to I-See Video Generator Client!** 🎉

Your contributions help make this project better for everyone.

