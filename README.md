
# Windows API Hooking

## Overview

**HookWindowsAPI** is a sample project demonstrating API hooking by intercepting the CreateFileW function in Windows from the kernel32.dll library. It employs DLL injection to inject a custom DLL into any target process, allowing for real-time profiling of CreateFileW usage within that process. The project leverages a pipe to transfer the captured API call data from the injected DLL back to a main process, which then receives and displays the profiled information in a console application. After profiling is complete, the injected DLL is safely unloaded from the target process, ensuring minimal disruption to the target's normal behavior. This project showcases practical API hooking techniques, useful for performance monitoring, debugging, and behavior analysis of file operations in Windows.

**Disclaimer:** This code is developed solely for educational purposes. It is intended to demonstrate how Windows API Hooking works to enhance understanding and improve defensive measures. Do not use this code for malicious activities.

## Windows API Hooking

**Windows API Hooking** is a technique used to intercept and modify the behavior of Windows system calls, allowing developers or attackers to alter how applications or the operating system functions. It works by redirecting calls to system APIs, either by inserting custom code into an application's address space or modifying system libraries. Hooking is widely used for legitimate purposes such as debugging, monitoring, or enhancing software functionality, but it can also be employed maliciously for activities like malware development or bypassing security controls. API hooking can occur at different levels, including inline hooking, import address table (IAT) hooking, or using more sophisticated mechanisms like detours.

## Detours

**Detours** is a powerful library developed by Microsoft for intercepting and modifying functions in Windows applications, particularly for hooking Windows API calls. It allows developers to reroute calls to existing functions, replacing or extending their behavior without altering the original binary code. By injecting custom code into the execution flow, Detours enables tasks such as monitoring, logging, debugging, and dynamically modifying application behavior. It is widely used in both academic and commercial settings to instrument applications, create virtual environments, and facilitate seamless software upgrades. Due to its flexibility, Detours is employed in legitimate software development as well as potentially malicious activities.
Technically, Detours works by intercepting and redirecting function calls at the binary level. It modifies the in-memory instructions of target functions, such as Windows API calls, by overwriting the initial instructions with a jump to a detour function. This allows developers to inject custom logic, then optionally return control to the original function to ensure smooth integration. This technique is invaluable for logging, profiling, or extending application functionality without altering the original executable.

## Libraries and Tools Used 

For this challenge, I utilized Detours libraries and the modern features of C++:

- **Detours:** A Microsoft library for intercepting and modifying Windows API calls, enabling function hooking without altering the original code.
- **Modern C++ (C++17/20):** Advanced features of modern C++ were used to enhance code readability, maintainability, and performance, showcasing best practices in contemporary software development.

## Class Descriptions and Responsibilities

### 1. **NamedPipeClient.h in NtHookDLL**
   - The `NamedPipeClient` class is a Singleton designed for educational purposes to manage client connections to a named pipe in Windows, enabling simple inter-process communication without encryption. It allows sending wide-character string messages to a server through a pipe using the `SendMessage()` function. While this approach is sufficient for demonstration, in real-world applications, implementing encryption and considering more efficient solutions such as sockets is mandatory to ensure security and scalability.
   
### 2. **NamedPipeServer.h in HookWindowsAPI**
   - The `NamedPipeServer` class is responsible for creating and managing a named pipe server for inter-process communication. It handles the creation of the pipe, waits for a client to connect, and reads messages from the client via the pipe. The class utilizes a `notifier` (Notifier.h) to send status updates or errors to the main module. For educational purposes, this implementation uses a simple pipe communication without encryption. In real-world scenarios, encryption and more efficient mechanisms like sockets should be considered to enhance security and performance.
   
### 3. **Notifier.h in HookWindowsAPI**
   - The `Notifier` class is responsible for managing and triggering notifications and error messages to registered listeners. It provides mechanisms to register callbacks for both regular notifications and error events. These callbacks are stored in respective vectors, ensuring thread safety through the use of mutex locks. When a notification or error occurs, the class invokes all registered callbacks, allowing for real-time updates and handling of messages. This class is particularly useful for broadcasting messages or errors in a multithreaded environment.
   
### 4. **ProcessInjectionHelper.h in HookWindowsAPI**
   - This class is responsible for managing the injection and unloading of DLLs into target processes. It provides key functionality such as identifying the target process by its name or process ID, checking if the specified DLL file exists, and using Windows API calls to perform DLL injection or removal. The class also handles memory allocation within the target process and creates remote threads to execute functions like `LoadLibraryW` and `FreeLibrary` for loading and unloading the DLL, respectively. Additionally, it communicates any errors or statuses through the `Notifier` class, ensuring smooth operation and feedback during the injection process.
	
### 5. **SynchronizationPoint.h in HookWindowsAPI**
   - The `SynchronizationPoint` class is designed to manage thread synchronization. It uses a promise-future mechanism from the C++ standard library to enable threads to signal task completion and wait for that signal. The `Signal()` method is called by a thread to indicate that its task is complete, while `Wait()` allows other threads to block execution until the signal is received. This class ensures smooth communication and coordination between threads, making it useful for synchronizing complex, multi-threaded operations.

## Build Instructions

To build the "NtHookDLL" project in Visual Studio, follow these steps:

1. **Download and Build Detours Library**:  
   Download the Detours library from its official GitHub repository at [https://github.com/microsoft/Detours](https://github.com/microsoft/Detours). Once downloaded, follow the instructions in the repository to build the library on your system.

2. **Configure Project Settings**:  
   After building the Detours library, you need to configure Visual Studio to use it in your project:
   - **Include Directories**: Go to your project settings, under *C/C++ -> General -> Additional Include Directories*. Add the path to the **Detours** include folder (e.g., `C:\path\to\detours\include`).
   - **Library Directories**: Navigate to *Linker -> General -> Additional Library Directories* and add the path to the **Detours** library folder (e.g., `C:\path\to\detours\lib.X64` or `lib.X64` depending on your target platform).
   - **Link Libraries**: In *Linker -> Input -> Additional Dependencies*, add the **detours.lib** to link Detours with your project.

3. **Build the Project**:  
   Once the Detours library is correctly linked, you can build the "NtHookDLL" project using Visual Studio's Build options.

Following these steps will ensure that your project is correctly set up to use the Detours library for API hooking.

## How to Execute the Project

To execute the **HookWindowsAPI** project, follow these steps:

1. Open the solution in Visual Studio.
2. Build the project to generate the necessary binaries.
3. From the command line or within Visual Studio, execute **HookWindowsAPI** with a single parameter: the **process name** of the target process you want to hook. For example:

   ```bash
   HookWindowsAPI.exe target_process_name
   
   HookWindowsAPI.exe "notepad.exe"
   ```

   Replace `target_process_name` with the name of the process you want to profile. Make sure to include the full extension (e.g., `.exe`) when specifying the process name!

## Debugging Instructions

If you wish to debug the project, it is crucial to ensure that the **NtHookDLL.dll** file is referenced using its **full path**. This is because, during the DLL injection process, the `LoadLibraryW` API looks for the DLL in the directory of the **target process**, not in the directory where **HookWindowsAPI** is located. 

To avoid issues with DLL injection while debugging, make sure to provide the full path to **NtHookDLL.dll**. This can be done by setting the correct path in your code. Here’s an example of how to use the full path:

```cpp
LoadLibraryW(L"C:\Full\Path\To\NtHookDLL.dll");
```

By specifying the absolute path, you ensure that the **LoadLibraryW** function correctly finds the DLL, allowing successful injection and debugging.

## License

This project is licensed under the MIT License - see the [LICENSE] file for details.

## Libraries Used

This project uses the following libraries:

- [Detours](https://github.com/microsoft/Detours/blob/main/LICENSE.md) - Microsoft Research Detours License

## Conclusion

**HookWindowsAPI** demonstrates the practical use of API hooking through DLL injection, allowing for real-time profiling of the CreateFileW function in Windows. By leveraging the power of Microsoft's Detours library and modern C++, this project provides a clear example of how to intercept and monitor API calls in a non-intrusive manner. With the ability to safely inject and unload the DLL, this solution highlights the efficiency and flexibility of API hooking techniques, useful for debugging, monitoring, and enhancing Windows applications.

## Important Notice

This project is intended for educational purposes only. The techniques demonstrated here are commonly used by malware, and it is important to understand them to develop effective defenses. Do not use this code for malicious activities.