# Building with Visual Studio 2022

To build all the C++ design patterns with Visual Studio 2022:

## Option 1: Using Developer Command Prompt (Recommended)

1. Open **"x64 Native Tools Command Prompt for VS 2022"** from the Start Menu
   - Search for "x64 Native" in the Start Menu
   - Make sure it's the x64 version, not x86

2. Navigate to your project directory:
   ```
   cd designPatterns
   ```

3. Run the build script:
   ```
   build_all.bat
   ```

## Option 2: Using Visual Studio IDE

1. Open Visual Studio 2022
2. Select "Open a local folder"
3. Navigate to the `designPatterns` folder
4. The CMakeLists.txt should be detected automatically
5. Select x64-Debug or x64-Release configuration
6. Build → Build All

## Option 3: Using Regular Command Prompt

1. Open a regular Command Prompt (cmd.exe)
2. Initialize VS environment e.g.:
   ```
   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
   ```
3. Navigate to project:
   ```
   cd designPatterns
   ```
4. Run build:
   ```
   build_all.bat
   ```

## Troubleshooting

If you see "library machine type 'x86' conflicts with target machine type 'x64'":
- Make sure you're using the x64 Developer Command Prompt
- Or run vcvars64.bat (not vcvars32.bat or vcvarsamd64_x86.bat)

The build scripts have been updated to properly detect .cpp files in each pattern directory.