# TextDiff

**TextDiff** is a Windows desktop application for comparing text and identifying differences between two text inputs.

Built with **C++20**, **Qt 6**, and **CMake**, TextDiff provides a desktop interface with dedicated text panes, a comparison view, and a custom visual theme.

---

## Table of Contents

- [Features](#features)
- [Screenshots](#screenshots)
- [Download and Installation](#download-and-installation)
- [Using TextDiff](#using-textdiff)
- [Development Requirements](#development-requirements)
- [Building from Source](#building-from-source)
- [Development Guide](#development-guide)
- [Project Architecture](#project-architecture)
- [Project Structure](#project-structure)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- Text comparison and difference visualization.
- File loading for text comparison.
- Dedicated text input panes.
- Code-editor-style text components.
- Custom desktop user interface.
- Custom visual theme and UI components.
- Native Windows application support.

> Additional features and capabilities will be documented as they are implemented and verified.

---

## Screenshots

Application screenshots will be added here.

<!-- Add screenshots using:
![TextDiff Main Window](screenshots/main-window.png)
-->

---

## Download and Installation

### Windows Portable Version

Pre-built portable versions are intended to be distributed through the project's GitHub Releases.

**Download:**

[TextDiff Releases](https://github.com/DRwander/TextDiff/releases)

Installation steps:

1. Open the Releases page.
2. Download the latest Windows portable package.
3. Extract the downloaded archive.
4. Keep all included DLL files and Qt plugin folders together with the executable.
5. Run `TextDiff.exe`.

No separate Qt installation should be necessary when using a correctly prepared portable package.

**Important:** Do not move `TextDiff.exe` away from its accompanying DLL files and plugin directories.

---

## Using TextDiff

TextDiff is designed to make comparing two text inputs straightforward.

### Basic Comparison Workflow

1. Launch TextDiff.
2. Load or enter the first text in the first text pane.
3. Load or enter the second text in the second text pane.
4. Start the comparison using the appropriate application control.
5. Review the differences displayed in the comparison view.

### Working with Files

TextDiff includes file-loading functionality for bringing text into the application.

The exact supported file formats and additional file-handling options depend on the current application implementation and will be documented in future updates.

---

## Development Requirements

If you want to build TextDiff from source or contribute to its development, you will need the following tools.

| Requirement | Version |
|---|---|
| C++ Standard | C++20 |
| Qt | 6.12 or newer |
| CMake | 3.21 or newer |
| Compiler | C++20-compatible compiler |
| Git | Recent version |
| IDE | Qt Creator or another CMake-compatible IDE |
| Platform | Windows 10 / Windows 11 |

### Required Qt Components

The project currently requires:

- Qt Widgets
- Qt Concurrent

### Recommended Windows Development Environment

For Windows development, the following configuration is recommended:

- Visual Studio C++ Build Tools
- MSVC x64 compiler
- Qt 6.12 MSVC 64-bit
- CMake
- Qt Creator

**Important:** Your Qt installation and compiler must use compatible architectures and toolchains.

For example, an MSVC 64-bit Qt installation should be used with a compatible MSVC 64-bit compiler.

---

## Building from Source

### 1. Clone the Repository

Open a terminal and execute:

```bash
git clone https://github.com/DRwander/TextDiff.git
```

Navigate to the project directory:

```bash
cd TextDiff
```

### 2. Configure the Project

Create the CMake build configuration:

```bash
cmake -S . -B build
```

If CMake cannot automatically locate your Qt installation, specify the Qt installation directory using `CMAKE_PREFIX_PATH`.

Example:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/msvc2022_64"
```

Replace the example path with the actual location of your Qt installation.

The specified directory should be the Qt kit installation prefix containing directories such as `bin` and `lib`.

### 3. Build the Application

For a Release build:

```bash
cmake --build build --config Release
```

For a Debug build:

```bash
cmake --build build --config Debug
```

### 4. Run the Application

After a successful build, locate the generated `TextDiff.exe` in the build output directory.

Run the executable to launch the application.

---

## Development Guide

This section is intended for developers who want to modify, improve, or extend TextDiff.

### Opening the Project

The recommended way to open the project in Qt Creator is:

1. Launch Qt Creator.
2. Select **File → Open File or Project**.
3. Navigate to the TextDiff project directory.
4. Select `CMakeLists.txt`.
5. Select a compatible Qt Kit.
6. Configure the project.

### Making Code Changes

When modifying the project:

1. Identify the source file responsible for the functionality you want to change.
2. Review the existing implementation and its dependencies.
3. Make focused changes without unnecessarily modifying unrelated components.
4. Rebuild the application.
5. Test the modified functionality.
6. Review the final changes before committing them.

### Adding New Source Files

When adding new `.cpp` or `.h` files:

1. Place the files in the appropriate source directory.
2. Update `CMakeLists.txt` when necessary.
3. Reconfigure the CMake project.
4. Build the application.
5. Verify that the new functionality works correctly.

### Modifying Application Resources

Application resources are managed through Qt resource files.

The main resource collection is:

```text
resources/resources.qrc
```

Windows application resources are defined in:

```text
resources/app.rc
```

When adding or replacing icons and other resources, make sure the corresponding resource references are updated correctly.

---

## Project Architecture

TextDiff uses a modular source structure to separate its main application components.

The following table describes the intended responsibilities based on the current source file organization.

| Component | File | Responsibility |
|---|---|---|
| Application Entry | `main.cpp` | Application startup |
| Main Window | `MainWindow.cpp / .h` | Main application window |
| Diff Algorithm | `DiffAlgo.h` | Difference algorithm |
| Diff Engine | `DiffEngine.cpp / .h` | Difference processing |
| Diff View | `DiffView.cpp / .h` | Difference visualization |
| Code Editor | `CodeEditor.cpp / .h` | Editor-style text component |
| File Pane | `FilePane.cpp / .h` | Text and file pane component |
| Text Loader | `TextLoader.cpp / .h` | Text loading functionality |
| Theme | `Theme.h` | Theme definitions |
| Glass Panel | `GlassPanel.cpp / .h` | Custom glass-style UI component |
| Window Chrome | `WindowChrome.cpp / .h` | Window appearance and controls |

### Main Technologies

**C++20**

Used for the application's core implementation.

**Qt 6 Widgets**

Used for the graphical user interface and desktop components.

**Qt Concurrent**

Provides Qt's concurrent programming functionality.

**CMake**

Used for project configuration, dependency management, and building the application.

---

## Project Structure

```text
TextDiff/
│
├── src/
│   ├── main.cpp
│   ├── MainWindow.cpp
│   ├── MainWindow.h
│   ├── DiffAlgo.h
│   ├── DiffEngine.cpp
│   ├── DiffEngine.h
│   ├── DiffView.cpp
│   ├── DiffView.h
│   ├── CodeEditor.cpp
│   ├── CodeEditor.h
│   ├── FilePane.cpp
│   ├── FilePane.h
│   ├── GlassPanel.cpp
│   ├── GlassPanel.h
│   ├── TextLoader.cpp
│   ├── TextLoader.h
│   ├── Theme.h
│   ├── WindowChrome.cpp
│   └── WindowChrome.h
│
├── resources/
│   ├── icons/
│   ├── app.rc
│   └── resources.qrc
│
├── CMakeLists.txt
├── .gitignore
└── README.md
```

Build directories, generated files, and local IDE configuration files should not be committed to the source repository.

Portable application packages should be distributed separately through GitHub Releases.

---

## Troubleshooting

### CMake Cannot Find Qt

If CMake cannot locate Qt:

- Verify that Qt 6.12 or newer is installed.
- Confirm that the Qt Widgets and Concurrent components are installed.
- Check the configured Qt installation path.
- Set `CMAKE_PREFIX_PATH` to the correct Qt installation directory.
- Make sure your compiler and Qt Kit are compatible.

### Application Does Not Start Outside Qt Creator

Building an application does not automatically include all required Qt runtime dependencies.

The executable may require:

- Qt runtime DLL files.
- Qt platform plugins.
- Additional Qt plugin directories.
- Other required runtime dependencies.

On Windows, use the compatible `windeployqt` utility to deploy the required Qt dependencies.

After deployment, test the portable directory independently of Qt Creator.

### Build Errors After Changing the Compiler or Qt Kit

CMake stores compiler and dependency information in its build directory.

If you change the Qt Kit or compiler, create a fresh build directory and configure the project again.

---

## Contributing

Contributions, suggestions, and bug reports are welcome.

If you would like to contribute:

1. Fork the repository.
2. Create a separate branch for your changes.
3. Implement your changes.
4. Build and test the application.
5. Commit your changes with a descriptive commit message.
6. Submit a Pull Request.

### Contribution Guidelines

- Keep changes focused and maintainable.
- Follow the existing project structure and coding conventions.
- Avoid unnecessary modifications to unrelated components.
- Test changes before submitting a Pull Request.
- Do not commit build outputs, temporary files, or local IDE configuration files.
- Clearly describe the purpose and impact of your changes.

For significant changes, consider opening an issue first to discuss the proposed implementation.

---

## License

A license has not yet been specified for this repository.

Until an explicit open-source license is added, no permission to use, modify, or redistribute this project's source code should be assumed.

---

## Repository

**GitHub:** [https://github.com/DRwander/TextDiff](https://github.com/DRwander/TextDiff)

**Releases:** [https://github.com/DRwander/TextDiff/releases](https://github.com/DRwander/TextDiff/releases)

---

**TextDiff** — A desktop tool for text comparison, built with C++20 and Qt 6.